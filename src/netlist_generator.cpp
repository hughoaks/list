#include "netlist_generator.h"
#include <algorithm>
#include <iostream>
#include <cmath>

NetlistGenerator::NetlistGenerator(const GeneratorConfig& config)
    : config_(config)
    , rng_(config.seed)
    , signal_counter_(0)
    , op_counter_(0)
{
}

void NetlistGenerator::generate() {
    if (config_.verbose) {
        std::cout << "Generating netlist..." << std::endl;
    }

    generateInputs();
    generateOutputs();
    generateDatapath();

    if (config_.num_pipeline_stages > 0) {
        generatePipeline();
    }

    // Generate control flow structures for synthesis testing
    generateControlBlocks();

    connectOutputs();
    assignDepths();

    if (config_.verbose) {
        std::cout << "Generated " << operations_.size() << " operations" << std::endl;
        std::cout << "Generated " << control_blocks_.size() << " control blocks" << std::endl;
        std::cout << "Total signals: " << (inputs_.size() + outputs_.size() + wires_.size() + regs_.size()) << std::endl;
    }
}

void NetlistGenerator::generateInputs() {
    for (int i = 0; i < config_.num_inputs; ++i) {
        int width = generateRandomWidth();
        bool is_signed = config_.use_signed && randomBool(0.5);
        std::string name = "in_" + std::to_string(i);
        inputs_.push_back(std::make_shared<Signal>(name, width, SignalType::INPUT, is_signed));
    }
}

void NetlistGenerator::generateOutputs() {
    for (int i = 0; i < config_.num_outputs; ++i) {
        int width = randomInt(config_.output_width_min, config_.output_width_max);
        bool is_signed = config_.use_signed && randomBool(0.5);
        std::string name = "out_" + std::to_string(i);
        outputs_.push_back(std::make_shared<Signal>(name, width, SignalType::OUTPUT, is_signed));
    }
}

void NetlistGenerator::generateDatapath() {
    for (int i = 0; i < config_.num_operations; ++i) {
        OpType op_type = selectRandomOpType();
        std::vector<SignalPtr> available = getAvailableSignals();

        if (available.empty()) {
            available = inputs_;  // Fallback to inputs if no signals available
        }

        OperationPtr op = nullptr;

        // Generate operation based on type
        if (op_type >= OpType::ADD && op_type <= OpType::MOD) {
            op = generateArithmeticOp(available);
        } else if (op_type >= OpType::AND && op_type <= OpType::XNOR) {
            op = generateLogicalOp(available);
        } else if (op_type >= OpType::EQ && op_type <= OpType::GTE) {
            op = generateComparisonOp(available);
        } else if (op_type >= OpType::SLL && op_type <= OpType::SRA) {
            op = generateShiftOp(available);
        } else if (op_type == OpType::MUX2 || op_type == OpType::MUX4) {
            op = generateMuxOp(available);
        } else if (op_type == OpType::CONCAT) {
            op = generateConcatOp(available);
        } else if (op_type >= OpType::RED_AND && op_type <= OpType::RED_XNOR) {
            op = generateReductionOp(available);
        }

        if (op) {
            operations_.push_back(op);
        }
    }
}

void NetlistGenerator::generatePipeline() {
    // Add pipeline registers between stages
    // This is a simplified implementation
    for (auto& op : operations_) {
        int stage = op->getDepth() * config_.num_pipeline_stages / config_.max_depth;
        op->setStage(stage);
    }
}

void NetlistGenerator::connectOutputs() {
    // Connect available wires to outputs
    std::vector<SignalPtr> available = getAvailableSignals();

    for (auto& output : outputs_) {
        if (!available.empty()) {
            SignalPtr source = selectRandomSignal(available);

            // Create assignment operation
            OperationPtr op = std::make_shared<Operation>(OpType::AND, output);
            op->addInput(source);

            // If widths don't match, create concat/slice operation
            if (source->getWidth() != output->getWidth()) {
                // For simplicity, just use truncation or zero-extension
                // In a more sophisticated implementation, we'd handle this better
            }

            op->addInput(source);  // Simple pass-through or operation
        }
    }
}

void NetlistGenerator::assignDepths() {
    // Simple depth assignment based on dependencies
    // In a more sophisticated implementation, we'd do topological sort
    for (size_t i = 0; i < operations_.size(); ++i) {
        operations_[i]->setDepth(static_cast<int>(i) % config_.max_depth);
    }
}

SignalPtr NetlistGenerator::createWire(int width, bool is_signed) {
    std::string name = "wire_" + std::to_string(signal_counter_++);
    auto wire = std::make_shared<Signal>(name, width, SignalType::WIRE, is_signed);
    wires_.push_back(wire);
    return wire;
}

SignalPtr NetlistGenerator::createReg(int width, bool is_signed) {
    std::string name = "reg_" + std::to_string(signal_counter_++);
    auto reg = std::make_shared<Signal>(name, width, SignalType::REG, is_signed);
    regs_.push_back(reg);
    return reg;
}

SignalPtr NetlistGenerator::selectRandomSignal(const std::vector<SignalPtr>& candidates) {
    if (candidates.empty()) return nullptr;
    std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
    return candidates[dist(rng_)];
}

SignalPtr NetlistGenerator::selectRandomSignalWithWidth(const std::vector<SignalPtr>& candidates, int width) {
    std::vector<SignalPtr> matching;
    for (const auto& sig : candidates) {
        if (sig->getWidth() == width) {
            matching.push_back(sig);
        }
    }

    if (!matching.empty()) {
        return selectRandomSignal(matching);
    }

    // If no exact match, return any signal
    return selectRandomSignal(candidates);
}

std::vector<SignalPtr> NetlistGenerator::getAvailableSignals(int max_depth) const {
    std::vector<SignalPtr> available;

    // Always include inputs
    available.insert(available.end(), inputs_.begin(), inputs_.end());

    // Include wires (from previous operations)
    available.insert(available.end(), wires_.begin(), wires_.end());

    return available;
}

OpType NetlistGenerator::selectRandomOpType() {
    // Build weighted distribution
    std::vector<double> weights;
    std::vector<OpType> types;

    // Arithmetic
    if (config_.weight_arithmetic > 0) {
        weights.push_back(config_.weight_arithmetic);
        types.push_back(OpType::ADD);  // Will refine later
    }

    // Logical
    if (config_.weight_logical > 0) {
        weights.push_back(config_.weight_logical);
        types.push_back(OpType::AND);
    }

    // Comparison
    if (config_.weight_comparison > 0) {
        weights.push_back(config_.weight_comparison);
        types.push_back(OpType::EQ);
    }

    // Shift
    if (config_.weight_shift > 0) {
        weights.push_back(config_.weight_shift);
        types.push_back(OpType::SLL);
    }

    // Mux
    if (config_.weight_mux > 0) {
        weights.push_back(config_.weight_mux);
        types.push_back(OpType::MUX2);
    }

    // Concat
    if (config_.weight_concat > 0) {
        weights.push_back(config_.weight_concat);
        types.push_back(OpType::CONCAT);
    }

    // Reduction
    if (config_.weight_reduction > 0) {
        weights.push_back(config_.weight_reduction);
        types.push_back(OpType::RED_AND);
    }

    std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
    return types[dist(rng_)];
}

int NetlistGenerator::generateRandomWidth() {
    return randomInt(config_.input_width_min, config_.input_width_max);
}

bool NetlistGenerator::randomBool(double probability) {
    std::bernoulli_distribution dist(probability);
    return dist(rng_);
}

int NetlistGenerator::randomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng_);
}

OperationPtr NetlistGenerator::generateArithmeticOp(const std::vector<SignalPtr>& available) {
    OpType op_type = selectArithmeticOpType();

    SignalPtr a = selectRandomSignal(available);
    SignalPtr b = selectRandomSignal(available);

    if (!a || !b) return nullptr;

    // Output width calculation
    int out_width = std::max(a->getWidth(), b->getWidth());
    if (op_type == OpType::MULT) {
        out_width = a->getWidth() + b->getWidth();  // Multiplier produces wider result
    }

    bool is_signed = a->isSigned() || b->isSigned();
    SignalPtr output = createWire(out_width, is_signed);

    auto op = std::make_shared<Operation>(op_type, output);
    op->addInput(a);
    op->addInput(b);

    return op;
}

OperationPtr NetlistGenerator::generateLogicalOp(const std::vector<SignalPtr>& available) {
    // Select random logical operation
    std::vector<OpType> logical_ops = {OpType::AND, OpType::OR, OpType::XOR, OpType::NOT,
                                        OpType::NAND, OpType::NOR, OpType::XNOR};
    OpType op_type = logical_ops[randomInt(0, logical_ops.size() - 1)];

    if (op_type == OpType::NOT) {
        SignalPtr a = selectRandomSignal(available);
        if (!a) return nullptr;

        SignalPtr output = createWire(a->getWidth(), a->isSigned());
        auto op = std::make_shared<Operation>(op_type, output);
        op->addInput(a);
        return op;
    } else {
        SignalPtr a = selectRandomSignal(available);
        SignalPtr b = selectRandomSignal(available);
        if (!a || !b) return nullptr;

        int out_width = std::max(a->getWidth(), b->getWidth());
        SignalPtr output = createWire(out_width, false);

        auto op = std::make_shared<Operation>(op_type, output);
        op->addInput(a);
        op->addInput(b);
        return op;
    }
}

OperationPtr NetlistGenerator::generateComparisonOp(const std::vector<SignalPtr>& available) {
    std::vector<OpType> comp_ops = {OpType::EQ, OpType::NEQ, OpType::LT,
                                     OpType::GT, OpType::LTE, OpType::GTE};
    OpType op_type = comp_ops[randomInt(0, comp_ops.size() - 1)];

    SignalPtr a = selectRandomSignal(available);
    SignalPtr b = selectRandomSignal(available);
    if (!a || !b) return nullptr;

    SignalPtr output = createWire(1, false);  // Comparison produces 1-bit result

    auto op = std::make_shared<Operation>(op_type, output);
    op->addInput(a);
    op->addInput(b);

    return op;
}

OperationPtr NetlistGenerator::generateShiftOp(const std::vector<SignalPtr>& available) {
    OpType op_type = selectShiftOpType();

    SignalPtr a = selectRandomSignal(available);
    SignalPtr b = selectRandomSignal(available);
    if (!a || !b) return nullptr;

    SignalPtr output = createWire(a->getWidth(), a->isSigned());

    auto op = std::make_shared<Operation>(op_type, output);
    op->addInput(a);
    op->addInput(b);

    return op;
}

OperationPtr NetlistGenerator::generateMuxOp(const std::vector<SignalPtr>& available) {
    bool use_mux4 = randomBool(0.3);
    OpType op_type = use_mux4 ? OpType::MUX4 : OpType::MUX2;

    if (op_type == OpType::MUX2) {
        SignalPtr sel = selectRandomSignal(available);
        SignalPtr a = selectRandomSignal(available);
        SignalPtr b = selectRandomSignal(available);

        if (!sel || !a || !b) return nullptr;

        int out_width = std::max(a->getWidth(), b->getWidth());
        SignalPtr output = createWire(out_width, a->isSigned() || b->isSigned());

        auto op = std::make_shared<Operation>(op_type, output);
        op->addInput(sel);
        op->addInput(a);
        op->addInput(b);

        return op;
    } else {
        // MUX4 needs 1 select (2-bit) and 4 data inputs
        SignalPtr sel = selectRandomSignal(available);
        if (!sel || sel->getWidth() < 2) {
            // Need at least 2-bit select
            sel = createWire(2, false);
        }

        std::vector<SignalPtr> data_inputs;
        for (int i = 0; i < 4; ++i) {
            SignalPtr d = selectRandomSignal(available);
            if (d) data_inputs.push_back(d);
        }

        if (data_inputs.size() < 4) return nullptr;

        int out_width = data_inputs[0]->getWidth();
        SignalPtr output = createWire(out_width, false);

        auto op = std::make_shared<Operation>(op_type, output);
        op->addInput(sel);
        for (auto& d : data_inputs) {
            op->addInput(d);
        }

        return op;
    }
}

OperationPtr NetlistGenerator::generateConcatOp(const std::vector<SignalPtr>& available) {
    int num_inputs = randomInt(2, 4);
    std::vector<SignalPtr> inputs;
    int total_width = 0;

    for (int i = 0; i < num_inputs; ++i) {
        SignalPtr sig = selectRandomSignal(available);
        if (sig) {
            inputs.push_back(sig);
            total_width += sig->getWidth();
        }
    }

    if (inputs.size() < 2) return nullptr;

    SignalPtr output = createWire(total_width, false);
    auto op = std::make_shared<Operation>(OpType::CONCAT, output);

    for (auto& input : inputs) {
        op->addInput(input);
    }

    return op;
}

OperationPtr NetlistGenerator::generateReductionOp(const std::vector<SignalPtr>& available) {
    std::vector<OpType> red_ops = {OpType::RED_AND, OpType::RED_OR, OpType::RED_XOR,
                                    OpType::RED_NAND, OpType::RED_NOR, OpType::RED_XNOR};
    OpType op_type = red_ops[randomInt(0, red_ops.size() - 1)];

    SignalPtr a = selectRandomSignal(available);
    if (!a) return nullptr;

    SignalPtr output = createWire(1, false);  // Reduction produces 1-bit result

    auto op = std::make_shared<Operation>(op_type, output);
    op->addInput(a);

    return op;
}

OpType NetlistGenerator::selectArithmeticOpType() {
    std::vector<double> weights = {
        config_.weight_add,
        config_.weight_sub,
        config_.weight_mult,
        config_.weight_div,
        config_.weight_mod
    };

    std::vector<OpType> types = {
        OpType::ADD, OpType::SUB, OpType::MULT, OpType::DIV, OpType::MOD
    };

    std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
    return types[dist(rng_)];
}

OpType NetlistGenerator::selectShiftOpType() {
    std::vector<double> weights = {
        config_.weight_sll,
        config_.weight_srl,
        config_.weight_sra
    };

    std::vector<OpType> types = {
        OpType::SLL, OpType::SRL, OpType::SRA
    };

    std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
    return types[dist(rng_)];
}

void NetlistGenerator::generateControlBlocks() {
    if (config_.generate_case_statements || config_.num_case_statements > 0) {
        generateCaseStatements();
    }

    if (config_.generate_if_else_chains || config_.num_if_else_chains > 0) {
        generateIfElseChains();
    }

    if (config_.generate_sharing_opportunities) {
        generateSharingOpportunities();
    }
}

void NetlistGenerator::generateCaseStatements() {
    int num_cases = config_.num_case_statements > 0 ? config_.num_case_statements :
                    (config_.generate_case_statements ? 2 : 0);

    for (int i = 0; i < num_cases; ++i) {
        std::vector<SignalPtr> available = getAvailableSignals();
        if (available.empty()) continue;

        // Select a signal to use as the case selector
        SignalPtr selector = selectRandomSignal(available);
        if (!selector) continue;

        // Determine number of cases based on selector width (limit to reasonable number)
        int max_case_value = std::min(1 << std::min(selector->getWidth(), 4), config_.cases_per_statement);

        auto control_block = std::make_shared<ControlBlock>(ControlType::CASE_STATEMENT, selector);

        // Create output signal(s) that will be assigned in each case
        int num_outputs = randomInt(1, 3);
        std::vector<SignalPtr> case_outputs;
        for (int j = 0; j < num_outputs; ++j) {
            int width = generateRandomWidth();
            SignalPtr output = createReg(width, config_.use_signed && randomBool(0.5));
            case_outputs.push_back(output);
        }

        // Generate cases
        for (int case_val = 0; case_val < max_case_value; ++case_val) {
            control_block->addCase(case_val);

            // For each output, assign a value in this case
            for (auto& output : case_outputs) {
                if (config_.generate_sharing_opportunities && randomBool(0.7)) {
                    // Generate an arithmetic operation (creates sharing opportunity)
                    SignalPtr a = selectRandomSignal(available);
                    SignalPtr b = selectRandomSignal(available);
                    if (a && b) {
                        // The synthesis tool can potentially share this operation across cases
                        OpType op_type = selectArithmeticOpType();
                        SignalPtr temp_out = createWire(std::max(a->getWidth(), b->getWidth()),
                                                       a->isSigned() || b->isSigned());
                        auto op = std::make_shared<Operation>(op_type, temp_out);
                        op->addInput(a);
                        op->addInput(b);
                        control_block->addCaseOperation(case_val, op);
                        control_block->addCaseAssignment(case_val, output, temp_out);
                    }
                } else {
                    // Simple assignment
                    SignalPtr input = selectRandomSignal(available);
                    if (input) {
                        control_block->addCaseAssignment(case_val, output, input);
                    }
                }
            }
        }

        // Add default case
        std::vector<std::pair<SignalPtr, SignalPtr>> default_assigns;
        for (auto& output : case_outputs) {
            SignalPtr input = selectRandomSignal(available);
            if (input) {
                default_assigns.push_back({output, input});
            }
        }
        control_block->setDefaultCase(default_assigns);

        control_blocks_.push_back(control_block);

        if (config_.verbose) {
            std::cout << "Generated case statement with " << max_case_value << " cases" << std::endl;
        }
    }
}

void NetlistGenerator::generateIfElseChains() {
    int num_chains = config_.num_if_else_chains > 0 ? config_.num_if_else_chains :
                     (config_.generate_if_else_chains ? 2 : 0);

    for (int i = 0; i < num_chains; ++i) {
        std::vector<SignalPtr> available = getAvailableSignals();
        if (available.size() < 3) continue;

        auto control_block = std::make_shared<ControlBlock>(ControlType::IF_ELSE_CHAIN);

        // Create outputs that will be assigned in mutually exclusive branches
        int num_outputs = randomInt(1, 3);
        std::vector<SignalPtr> shared_outputs;
        for (int j = 0; j < num_outputs; ++j) {
            int width = generateRandomWidth();
            SignalPtr output = createReg(width, config_.use_signed && randomBool(0.5));
            shared_outputs.push_back(output);
        }

        // Generate 2-4 branches (if/else if/else if/else)
        int num_branches = randomInt(2, 4);

        for (int branch = 0; branch < num_branches; ++branch) {
            // All branches except the last have a condition
            if (branch < num_branches - 1) {
                // Create a condition signal
                SignalPtr cond = selectRandomSignal(available);
                if (!cond) continue;
                control_block->addBranch(cond);
            } else {
                // Last branch is "else"
                control_block->addElseBranch();
            }

            // In each branch, perform operations on shared outputs
            // These operations are MUTUALLY EXCLUSIVE and can potentially share resources
            for (auto& output : shared_outputs) {
                if (config_.generate_sharing_opportunities && randomBool(0.8)) {
                    // Generate an expensive operation (e.g., multiply)
                    // Synthesis tools should recognize these are mutually exclusive
                    SignalPtr a = selectRandomSignal(available);
                    SignalPtr b = selectRandomSignal(available);
                    if (a && b) {
                        OpType op_type = OpType::MULT;  // Expensive operation
                        SignalPtr temp_out = createWire(a->getWidth() + b->getWidth(),
                                                       a->isSigned() || b->isSigned());
                        auto op = std::make_shared<Operation>(op_type, temp_out);
                        op->addInput(a);
                        op->addInput(b);
                        control_block->addBranchOperation(branch, op);
                        control_block->addBranchAssignment(branch, output, temp_out);
                    }
                } else {
                    SignalPtr input = selectRandomSignal(available);
                    if (input) {
                        control_block->addBranchAssignment(branch, output, input);
                    }
                }
            }
        }

        control_blocks_.push_back(control_block);

        if (config_.verbose) {
            std::cout << "Generated if-else chain with " << num_branches << " branches (mutually exclusive)" << std::endl;
        }
    }
}

void NetlistGenerator::generateSharingOpportunities() {
    // Generate additional operations that could potentially share resources
    // This creates temporal sharing opportunities based on control signals
    std::vector<SignalPtr> available = getAvailableSignals();
    if (available.size() < 4) return;

    // Create a few sets of operations that are enabled by different control signals
    int num_groups = randomInt(1, 3);

    for (int g = 0; g < num_groups; ++g) {
        // Create enable signal
        SignalPtr enable = selectRandomSignal(available);
        if (!enable) continue;

        // Create 2-3 operations that all use the same enable
        // These can potentially share a single functional unit if only one is active at a time
        int ops_in_group = randomInt(2, 3);

        for (int i = 0; i < ops_in_group; ++i) {
            SignalPtr a = selectRandomSignal(available);
            SignalPtr b = selectRandomSignal(available);
            if (!a || !b) continue;

            // Use expensive operations (multipliers, dividers) for sharing
            OpType op_type = randomBool(0.7) ? OpType::MULT : OpType::ADD;

            int out_width = (op_type == OpType::MULT) ?
                           (a->getWidth() + b->getWidth()) :
                           std::max(a->getWidth(), b->getWidth());

            SignalPtr result = createWire(out_width, a->isSigned() || b->isSigned());

            auto op = std::make_shared<Operation>(op_type, result);
            op->addInput(a);
            op->addInput(b);
            operations_.push_back(op);
        }

        if (config_.verbose) {
            std::cout << "Generated sharing opportunity group with " << ops_in_group << " operations" << std::endl;
        }
    }
}
