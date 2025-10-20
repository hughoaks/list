#ifndef NETLIST_GENERATOR_H
#define NETLIST_GENERATOR_H

#include "config.h"
#include "signal.h"
#include "operation.h"
#include "control_block.h"
#include <vector>
#include <random>
#include <memory>

class NetlistGenerator {
public:
    explicit NetlistGenerator(const GeneratorConfig& config);

    // Generate the netlist
    void generate();

    // Get generated components
    const std::vector<SignalPtr>& getInputs() const { return inputs_; }
    const std::vector<SignalPtr>& getOutputs() const { return outputs_; }
    const std::vector<SignalPtr>& getWires() const { return wires_; }
    const std::vector<SignalPtr>& getRegs() const { return regs_; }
    const std::vector<OperationPtr>& getOperations() const { return operations_; }
    const std::vector<ControlBlockPtr>& getControlBlocks() const { return control_blocks_; }

    std::string getModuleName() const { return config_.module_name; }

private:
    GeneratorConfig config_;
    std::mt19937 rng_;

    std::vector<SignalPtr> inputs_;
    std::vector<SignalPtr> outputs_;
    std::vector<SignalPtr> wires_;
    std::vector<SignalPtr> regs_;
    std::vector<OperationPtr> operations_;
    std::vector<ControlBlockPtr> control_blocks_;

    int signal_counter_;
    int op_counter_;

    // Generation methods
    void generateInputs();
    void generateOutputs();
    void generateDatapath();
    void generatePipeline();
    void generateControlBlocks();
    void generateCaseStatements();
    void generateIfElseChains();
    void generateSharingOpportunities();
    void connectOutputs();
    void assignDepths();

    // Helper methods
    SignalPtr createWire(int width, bool is_signed = false);
    SignalPtr createReg(int width, bool is_signed = false);
    SignalPtr selectRandomSignal(const std::vector<SignalPtr>& candidates);
    SignalPtr selectRandomSignalWithWidth(const std::vector<SignalPtr>& candidates, int width);
    std::vector<SignalPtr> getAvailableSignals(int max_depth = -1) const;

    OpType selectRandomOpType();
    int generateRandomWidth();
    bool randomBool(double probability = 0.5);
    int randomInt(int min, int max);

    // Operation generators
    OperationPtr generateArithmeticOp(const std::vector<SignalPtr>& available);
    OperationPtr generateLogicalOp(const std::vector<SignalPtr>& available);
    OperationPtr generateComparisonOp(const std::vector<SignalPtr>& available);
    OperationPtr generateShiftOp(const std::vector<SignalPtr>& available);
    OperationPtr generateMuxOp(const std::vector<SignalPtr>& available);
    OperationPtr generateConcatOp(const std::vector<SignalPtr>& available);
    OperationPtr generateReductionOp(const std::vector<SignalPtr>& available);

    // Specific arithmetic operations
    OpType selectArithmeticOpType();
    OpType selectShiftOpType();
};

#endif // NETLIST_GENERATOR_H
