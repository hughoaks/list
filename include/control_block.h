#ifndef CONTROL_BLOCK_H
#define CONTROL_BLOCK_H

#include "signal.h"
#include "operation.h"
#include <vector>
#include <memory>
#include <string>

// Forward declaration
class ControlBlock;
using ControlBlockPtr = std::shared_ptr<ControlBlock>;

enum class ControlType {
    CASE_STATEMENT,
    IF_ELSE_CHAIN,
    ALWAYS_COMB,
    ALWAYS_FF
};

// Represents a single case in a case statement
struct CaseItem {
    int value;  // Case value
    std::vector<OperationPtr> operations;
    std::vector<std::pair<SignalPtr, SignalPtr>> assignments;  // output, input

    CaseItem(int val) : value(val) {}
};

// Represents a conditional branch (if/else if/else)
struct ConditionalBranch {
    SignalPtr condition;  // nullptr for "else" branch
    std::vector<OperationPtr> operations;
    std::vector<std::pair<SignalPtr, SignalPtr>> assignments;

    ConditionalBranch(SignalPtr cond = nullptr) : condition(cond) {}
};

class ControlBlock {
public:
    ControlBlock(ControlType type, SignalPtr select = nullptr);

    ControlType getType() const { return type_; }
    SignalPtr getSelect() const { return select_; }

    // For case statements
    void addCase(int value);
    void addCaseOperation(int case_value, OperationPtr op);
    void addCaseAssignment(int case_value, SignalPtr output, SignalPtr input);
    void setDefaultCase(const std::vector<std::pair<SignalPtr, SignalPtr>>& assignments);
    const std::vector<CaseItem>& getCases() const { return cases_; }
    const std::vector<std::pair<SignalPtr, SignalPtr>>& getDefaultAssignments() const {
        return default_assignments_;
    }

    // For if-else chains
    void addBranch(SignalPtr condition);
    void addElseBranch();
    void addBranchOperation(size_t branch_idx, OperationPtr op);
    void addBranchAssignment(size_t branch_idx, SignalPtr output, SignalPtr input);
    const std::vector<ConditionalBranch>& getBranches() const { return branches_; }

    // Get all signals written by this control block
    std::vector<SignalPtr> getWrittenSignals() const;

    // Generate Verilog code
    std::string generateVerilog(int indent = 0) const;

private:
    ControlType type_;
    SignalPtr select_;  // For case statements or if-else selector

    // For case statements
    std::vector<CaseItem> cases_;
    std::vector<std::pair<SignalPtr, SignalPtr>> default_assignments_;

    // For if-else chains
    std::vector<ConditionalBranch> branches_;

    std::string getIndent(int level) const;
    std::string generateCaseStatement(int indent) const;
    std::string generateIfElseChain(int indent) const;
};

#endif // CONTROL_BLOCK_H
