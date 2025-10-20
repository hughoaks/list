#ifndef OPERATION_H
#define OPERATION_H

#include "signal.h"
#include <vector>
#include <memory>

enum class OpType {
    // Arithmetic
    ADD, SUB, MULT, DIV, MOD,

    // Logical
    AND, OR, XOR, NOT, NAND, NOR, XNOR,

    // Comparison
    EQ, NEQ, LT, GT, LTE, GTE,

    // Shift
    SLL, SRL, SRA,  // shift left logical, shift right logical, shift right arithmetic

    // Reduction
    RED_AND, RED_OR, RED_XOR, RED_NAND, RED_NOR, RED_XNOR,

    // Mux
    MUX2, MUX4,

    // Concatenation
    CONCAT,

    // Other
    REPLICATE,
    CONDITIONAL  // ternary ? :
};

class Operation {
public:
    Operation(OpType type, SignalPtr output);

    void addInput(SignalPtr input);
    void addConstant(int value, int width);

    OpType getType() const { return type_; }
    SignalPtr getOutput() const { return output_; }
    const std::vector<SignalPtr>& getInputs() const { return inputs_; }

    // Generate Verilog assignment
    std::string generateAssignment() const;

    // Get operation depth (for scheduling)
    int getDepth() const { return depth_; }
    void setDepth(int depth) { depth_ = depth; }

    // Get pipeline stage
    int getStage() const { return stage_; }
    void setStage(int stage) { stage_ = stage; }

    static std::string opTypeToString(OpType type);
    static bool isUnary(OpType type);
    static bool isBinary(OpType type);
    static bool isTernary(OpType type);
    static int getRequiredOperands(OpType type);

private:
    OpType type_;
    SignalPtr output_;
    std::vector<SignalPtr> inputs_;
    std::vector<std::pair<int, int>> constants_;  // value, width
    int depth_;
    int stage_;

    std::string generateBinaryOp(const std::string& op) const;
    std::string generateUnaryOp(const std::string& op) const;
    std::string generateReductionOp(const std::string& op) const;
    std::string generateMux() const;
    std::string generateConcat() const;
    std::string generateConditional() const;
};

using OperationPtr = std::shared_ptr<Operation>;

#endif // OPERATION_H
