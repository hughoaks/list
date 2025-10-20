#ifndef VERILOG_EMITTER_H
#define VERILOG_EMITTER_H

#include "netlist_generator.h"
#include <string>
#include <ostream>

class VerilogEmitter {
public:
    explicit VerilogEmitter(const NetlistGenerator& generator);

    // Emit complete Verilog module
    void emit(std::ostream& os) const;

    // Emit testbench
    void emitTestbench(std::ostream& os) const;

private:
    const NetlistGenerator& generator_;

    void emitHeader(std::ostream& os) const;
    void emitModuleDeclaration(std::ostream& os) const;
    void emitSignalDeclarations(std::ostream& os) const;
    void emitCombinationalLogic(std::ostream& os) const;
    void emitSequentialLogic(std::ostream& os) const;
    void emitFooter(std::ostream& os) const;

    std::string getCurrentTimestamp() const;
};

#endif // VERILOG_EMITTER_H
