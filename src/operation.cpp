#include "operation.h"
#include <sstream>
#include <stdexcept>

Operation::Operation(OpType type, SignalPtr output)
    : type_(type)
    , output_(output)
    , depth_(0)
    , stage_(0)
{
}

void Operation::addInput(SignalPtr input) {
    inputs_.push_back(input);
}

void Operation::addConstant(int value, int width) {
    constants_.push_back({value, width});
}

std::string Operation::generateAssignment() const {
    std::ostringstream oss;
    oss << "assign " << output_->getUsage() << " = ";

    switch (type_) {
        // Arithmetic
        case OpType::ADD:
            oss << generateBinaryOp("+");
            break;
        case OpType::SUB:
            oss << generateBinaryOp("-");
            break;
        case OpType::MULT:
            oss << generateBinaryOp("*");
            break;
        case OpType::DIV:
            oss << generateBinaryOp("/");
            break;
        case OpType::MOD:
            oss << generateBinaryOp("%");
            break;

        // Logical
        case OpType::AND:
            oss << generateBinaryOp("&");
            break;
        case OpType::OR:
            oss << generateBinaryOp("|");
            break;
        case OpType::XOR:
            oss << generateBinaryOp("^");
            break;
        case OpType::NOT:
            oss << generateUnaryOp("~");
            break;
        case OpType::NAND:
            oss << "~(" << generateBinaryOp("&") << ")";
            break;
        case OpType::NOR:
            oss << "~(" << generateBinaryOp("|") << ")";
            break;
        case OpType::XNOR:
            oss << "~(" << generateBinaryOp("^") << ")";
            break;

        // Comparison
        case OpType::EQ:
            oss << generateBinaryOp("==");
            break;
        case OpType::NEQ:
            oss << generateBinaryOp("!=");
            break;
        case OpType::LT:
            oss << generateBinaryOp("<");
            break;
        case OpType::GT:
            oss << generateBinaryOp(">");
            break;
        case OpType::LTE:
            oss << generateBinaryOp("<=");
            break;
        case OpType::GTE:
            oss << generateBinaryOp(">=");
            break;

        // Shift
        case OpType::SLL:
            oss << generateBinaryOp("<<");
            break;
        case OpType::SRL:
            oss << generateBinaryOp(">>");
            break;
        case OpType::SRA:
            oss << generateBinaryOp(">>>");
            break;

        // Reduction
        case OpType::RED_AND:
            oss << generateReductionOp("&");
            break;
        case OpType::RED_OR:
            oss << generateReductionOp("|");
            break;
        case OpType::RED_XOR:
            oss << generateReductionOp("^");
            break;
        case OpType::RED_NAND:
            oss << generateReductionOp("~&");
            break;
        case OpType::RED_NOR:
            oss << generateReductionOp("~|");
            break;
        case OpType::RED_XNOR:
            oss << generateReductionOp("~^");
            break;

        // Mux
        case OpType::MUX2:
        case OpType::MUX4:
            oss << generateMux();
            break;

        // Concatenation
        case OpType::CONCAT:
            oss << generateConcat();
            break;

        // Conditional
        case OpType::CONDITIONAL:
            oss << generateConditional();
            break;

        default:
            oss << "/* UNKNOWN OPERATION */";
            break;
    }

    oss << ";";
    return oss.str();
}

std::string Operation::generateBinaryOp(const std::string& op) const {
    if (inputs_.size() < 2) {
        return "/* ERROR: not enough inputs */";
    }
    return "(" + inputs_[0]->getUsage() + " " + op + " " + inputs_[1]->getUsage() + ")";
}

std::string Operation::generateUnaryOp(const std::string& op) const {
    if (inputs_.empty()) {
        return "/* ERROR: no input */";
    }
    return "(" + op + inputs_[0]->getUsage() + ")";
}

std::string Operation::generateReductionOp(const std::string& op) const {
    if (inputs_.empty()) {
        return "/* ERROR: no input */";
    }
    return "(" + op + inputs_[0]->getUsage() + ")";
}

std::string Operation::generateMux() const {
    if (inputs_.size() < 3) {
        return "/* ERROR: not enough inputs for mux */";
    }

    if (type_ == OpType::MUX2) {
        // sel ? a : b
        return "(" + inputs_[0]->getUsage() + " ? " +
               inputs_[1]->getUsage() + " : " + inputs_[2]->getUsage() + ")";
    } else {
        // MUX4: {sel[1], sel[0]} selects from 4 inputs
        std::ostringstream oss;
        oss << "(" << inputs_[0]->getBit(1) << " ? (";
        oss << inputs_[0]->getBit(0) << " ? " << inputs_[4]->getUsage() << " : " << inputs_[3]->getUsage();
        oss << ") : (" << inputs_[0]->getBit(0) << " ? " << inputs_[2]->getUsage() << " : " << inputs_[1]->getUsage();
        oss << "))";
        return oss.str();
    }
}

std::string Operation::generateConcat() const {
    if (inputs_.size() < 2) {
        return "/* ERROR: not enough inputs for concat */";
    }

    std::ostringstream oss;
    oss << "{";
    for (size_t i = 0; i < inputs_.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << inputs_[i]->getUsage();
    }
    oss << "}";
    return oss.str();
}

std::string Operation::generateConditional() const {
    if (inputs_.size() < 3) {
        return "/* ERROR: not enough inputs for conditional */";
    }
    return "(" + inputs_[0]->getUsage() + " ? " +
           inputs_[1]->getUsage() + " : " + inputs_[2]->getUsage() + ")";
}

std::string Operation::opTypeToString(OpType type) {
    switch (type) {
        case OpType::ADD: return "ADD";
        case OpType::SUB: return "SUB";
        case OpType::MULT: return "MULT";
        case OpType::DIV: return "DIV";
        case OpType::MOD: return "MOD";
        case OpType::AND: return "AND";
        case OpType::OR: return "OR";
        case OpType::XOR: return "XOR";
        case OpType::NOT: return "NOT";
        case OpType::NAND: return "NAND";
        case OpType::NOR: return "NOR";
        case OpType::XNOR: return "XNOR";
        case OpType::EQ: return "EQ";
        case OpType::NEQ: return "NEQ";
        case OpType::LT: return "LT";
        case OpType::GT: return "GT";
        case OpType::LTE: return "LTE";
        case OpType::GTE: return "GTE";
        case OpType::SLL: return "SLL";
        case OpType::SRL: return "SRL";
        case OpType::SRA: return "SRA";
        case OpType::RED_AND: return "RED_AND";
        case OpType::RED_OR: return "RED_OR";
        case OpType::RED_XOR: return "RED_XOR";
        case OpType::RED_NAND: return "RED_NAND";
        case OpType::RED_NOR: return "RED_NOR";
        case OpType::RED_XNOR: return "RED_XNOR";
        case OpType::MUX2: return "MUX2";
        case OpType::MUX4: return "MUX4";
        case OpType::CONCAT: return "CONCAT";
        case OpType::CONDITIONAL: return "CONDITIONAL";
        default: return "UNKNOWN";
    }
}

bool Operation::isUnary(OpType type) {
    return type == OpType::NOT ||
           type == OpType::RED_AND || type == OpType::RED_OR || type == OpType::RED_XOR ||
           type == OpType::RED_NAND || type == OpType::RED_NOR || type == OpType::RED_XNOR;
}

bool Operation::isBinary(OpType type) {
    return type == OpType::ADD || type == OpType::SUB || type == OpType::MULT ||
           type == OpType::DIV || type == OpType::MOD ||
           type == OpType::AND || type == OpType::OR || type == OpType::XOR ||
           type == OpType::NAND || type == OpType::NOR || type == OpType::XNOR ||
           type == OpType::EQ || type == OpType::NEQ || type == OpType::LT ||
           type == OpType::GT || type == OpType::LTE || type == OpType::GTE ||
           type == OpType::SLL || type == OpType::SRL || type == OpType::SRA;
}

bool Operation::isTernary(OpType type) {
    return type == OpType::MUX2 || type == OpType::CONDITIONAL;
}

int Operation::getRequiredOperands(OpType type) {
    if (isUnary(type)) return 1;
    if (isBinary(type)) return 2;
    if (isTernary(type)) return 3;
    if (type == OpType::MUX4) return 5;  // 1 select + 4 data
    if (type == OpType::CONCAT) return 2;  // minimum 2
    return 0;
}
