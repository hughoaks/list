#include "control_block.h"
#include <sstream>
#include <algorithm>

ControlBlock::ControlBlock(ControlType type, SignalPtr select)
    : type_(type)
    , select_(select)
{
}

void ControlBlock::addCase(int value) {
    cases_.emplace_back(value);
}

void ControlBlock::addCaseOperation(int case_value, OperationPtr op) {
    for (auto& case_item : cases_) {
        if (case_item.value == case_value) {
            case_item.operations.push_back(op);
            return;
        }
    }
}

void ControlBlock::addCaseAssignment(int case_value, SignalPtr output, SignalPtr input) {
    for (auto& case_item : cases_) {
        if (case_item.value == case_value) {
            case_item.assignments.push_back({output, input});
            return;
        }
    }
}

void ControlBlock::setDefaultCase(const std::vector<std::pair<SignalPtr, SignalPtr>>& assignments) {
    default_assignments_ = assignments;
}

void ControlBlock::addBranch(SignalPtr condition) {
    branches_.emplace_back(condition);
}

void ControlBlock::addElseBranch() {
    branches_.emplace_back(nullptr);  // nullptr condition = else
}

void ControlBlock::addBranchOperation(size_t branch_idx, OperationPtr op) {
    if (branch_idx < branches_.size()) {
        branches_[branch_idx].operations.push_back(op);
    }
}

void ControlBlock::addBranchAssignment(size_t branch_idx, SignalPtr output, SignalPtr input) {
    if (branch_idx < branches_.size()) {
        branches_[branch_idx].assignments.push_back({output, input});
    }
}

std::vector<SignalPtr> ControlBlock::getWrittenSignals() const {
    std::vector<SignalPtr> signals;

    if (type_ == ControlType::CASE_STATEMENT) {
        for (const auto& case_item : cases_) {
            for (const auto& [output, input] : case_item.assignments) {
                if (std::find(signals.begin(), signals.end(), output) == signals.end()) {
                    signals.push_back(output);
                }
            }
        }
        for (const auto& [output, input] : default_assignments_) {
            if (std::find(signals.begin(), signals.end(), output) == signals.end()) {
                signals.push_back(output);
            }
        }
    } else if (type_ == ControlType::IF_ELSE_CHAIN) {
        for (const auto& branch : branches_) {
            for (const auto& [output, input] : branch.assignments) {
                if (std::find(signals.begin(), signals.end(), output) == signals.end()) {
                    signals.push_back(output);
                }
            }
        }
    }

    return signals;
}

std::string ControlBlock::generateVerilog(int indent) const {
    if (type_ == ControlType::CASE_STATEMENT) {
        return generateCaseStatement(indent);
    } else if (type_ == ControlType::IF_ELSE_CHAIN) {
        return generateIfElseChain(indent);
    }
    return "";
}

std::string ControlBlock::getIndent(int level) const {
    return std::string(level * 4, ' ');
}

std::string ControlBlock::generateCaseStatement(int indent) const {
    std::ostringstream oss;
    std::string ind = getIndent(indent);
    std::string ind1 = getIndent(indent + 1);
    std::string ind2 = getIndent(indent + 2);

    oss << ind << "always @(*) begin\n";
    oss << ind1 << "case (" << select_->getUsage() << ")\n";

    // Generate each case
    for (const auto& case_item : cases_) {
        oss << ind2 << case_item.value << ": begin\n";

        // Operations in this case
        for (const auto& op : case_item.operations) {
            oss << ind2 << "    " << op->generateAssignment() << "\n";
        }

        // Assignments in this case
        for (const auto& [output, input] : case_item.assignments) {
            oss << ind2 << "    " << output->getUsage() << " = " << input->getUsage() << ";\n";
        }

        oss << ind2 << "end\n";
    }

    // Default case
    if (!default_assignments_.empty()) {
        oss << ind2 << "default: begin\n";
        for (const auto& [output, input] : default_assignments_) {
            oss << ind2 << "    " << output->getUsage() << " = " << input->getUsage() << ";\n";
        }
        oss << ind2 << "end\n";
    }

    oss << ind1 << "endcase\n";
    oss << ind << "end\n";

    return oss.str();
}

std::string ControlBlock::generateIfElseChain(int indent) const {
    std::ostringstream oss;
    std::string ind = getIndent(indent);
    std::string ind1 = getIndent(indent + 1);

    oss << ind << "always @(*) begin\n";

    for (size_t i = 0; i < branches_.size(); ++i) {
        const auto& branch = branches_[i];

        if (i == 0) {
            oss << ind1 << "if (" << branch.condition->getUsage() << ") begin\n";
        } else if (branch.condition) {
            oss << ind1 << "end else if (" << branch.condition->getUsage() << ") begin\n";
        } else {
            oss << ind1 << "end else begin\n";
        }

        // Operations in this branch
        for (const auto& op : branch.operations) {
            oss << ind1 << "    " << op->generateAssignment() << "\n";
        }

        // Assignments in this branch (creating sharing opportunities)
        for (const auto& [output, input] : branch.assignments) {
            oss << ind1 << "    " << output->getUsage() << " = " << input->getUsage() << ";\n";
        }
    }

    if (!branches_.empty()) {
        oss << ind1 << "end\n";
    }

    oss << ind << "end\n";

    return oss.str();
}
