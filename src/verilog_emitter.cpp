#include "verilog_emitter.h"
#include <iomanip>
#include <ctime>

VerilogEmitter::VerilogEmitter(const NetlistGenerator& generator)
    : generator_(generator)
{
}

void VerilogEmitter::emit(std::ostream& os) const {
    emitHeader(os);
    emitModuleDeclaration(os);
    emitSignalDeclarations(os);
    emitCombinationalLogic(os);

    // Emit control blocks (case statements, if-else chains)
    const auto& control_blocks = generator_.getControlBlocks();
    if (!control_blocks.empty()) {
        os << "    // ========================================\n";
        os << "    // Control Flow Structures\n";
        os << "    // (for testing synthesis optimization)\n";
        os << "    // ========================================\n\n";

        for (const auto& cb : control_blocks) {
            os << cb->generateVerilog(1) << "\n";
        }
    }

    emitSequentialLogic(os);
    emitFooter(os);
}

void VerilogEmitter::emitHeader(std::ostream& os) const {
    os << "// ============================================================================\n";
    os << "// Random Verilog Datapath Generator\n";
    os << "// Generated: " << getCurrentTimestamp() << "\n";
    os << "// ============================================================================\n";
    os << "// This file was automatically generated for synthesis tool benchmarking.\n";
    os << "// Module: " << generator_.getModuleName() << "\n";
    os << "// Inputs: " << generator_.getInputs().size() << "\n";
    os << "// Outputs: " << generator_.getOutputs().size() << "\n";
    os << "// Operations: " << generator_.getOperations().size() << "\n";
    os << "// ============================================================================\n\n";
}

void VerilogEmitter::emitModuleDeclaration(std::ostream& os) const {
    os << "module " << generator_.getModuleName() << " (\n";

    // Inputs
    const auto& inputs = generator_.getInputs();
    for (size_t i = 0; i < inputs.size(); ++i) {
        os << "    input ";
        if (inputs[i]->isSigned()) os << "signed ";
        if (inputs[i]->getWidth() > 1) {
            os << "[" << (inputs[i]->getWidth() - 1) << ":0] ";
        }
        os << inputs[i]->getName();
        if (i < inputs.size() - 1 || !generator_.getOutputs().empty()) {
            os << ",";
        }
        os << "\n";
    }

    // Outputs
    const auto& outputs = generator_.getOutputs();
    for (size_t i = 0; i < outputs.size(); ++i) {
        os << "    output ";
        if (outputs[i]->isSigned()) os << "signed ";
        if (outputs[i]->getWidth() > 1) {
            os << "[" << (outputs[i]->getWidth() - 1) << ":0] ";
        }
        os << outputs[i]->getName();
        if (i < outputs.size() - 1) {
            os << ",";
        }
        os << "\n";
    }

    os << ");\n\n";
}

void VerilogEmitter::emitSignalDeclarations(std::ostream& os) const {
    // Wire declarations
    const auto& wires = generator_.getWires();
    if (!wires.empty()) {
        os << "    // Internal wires\n";
        for (const auto& wire : wires) {
            os << "    " << wire->getDeclaration() << ";\n";
        }
        os << "\n";
    }

    // Register declarations
    const auto& regs = generator_.getRegs();
    if (!regs.empty()) {
        os << "    // Registers\n";
        for (const auto& reg : regs) {
            os << "    " << reg->getDeclaration() << ";\n";
        }
        os << "\n";
    }
}

void VerilogEmitter::emitCombinationalLogic(std::ostream& os) const {
    const auto& operations = generator_.getOperations();

    if (operations.empty()) {
        os << "    // No operations generated\n\n";
        return;
    }

    os << "    // ========================================\n";
    os << "    // Combinational Logic\n";
    os << "    // ========================================\n\n";

    for (const auto& op : operations) {
        os << "    " << op->generateAssignment() << "\n";
    }

    os << "\n";
}

void VerilogEmitter::emitSequentialLogic(std::ostream& os) const {
    const auto& regs = generator_.getRegs();

    if (regs.empty()) {
        return;
    }

    os << "    // ========================================\n";
    os << "    // Sequential Logic (Pipeline Registers)\n";
    os << "    // ========================================\n\n";

    os << "    always @(posedge clk or negedge rst_n) begin\n";
    os << "        if (!rst_n) begin\n";
    for (const auto& reg : regs) {
        os << "            " << reg->getName() << " <= 0;\n";
    }
    os << "        end else begin\n";
    os << "            // Pipeline stage updates\n";
    for (const auto& reg : regs) {
        os << "            // " << reg->getName() << " <= ...;\n";
    }
    os << "        end\n";
    os << "    end\n\n";
}

void VerilogEmitter::emitFooter(std::ostream& os) const {
    os << "endmodule\n";
}

void VerilogEmitter::emitTestbench(std::ostream& os) const {
    os << "// ============================================================================\n";
    os << "// Testbench for " << generator_.getModuleName() << "\n";
    os << "// Generated: " << getCurrentTimestamp() << "\n";
    os << "// ============================================================================\n\n";

    os << "`timescale 1ns / 1ps\n\n";

    os << "module tb_" << generator_.getModuleName() << ";\n\n";

    // Declare testbench signals
    const auto& inputs = generator_.getInputs();
    const auto& outputs = generator_.getOutputs();

    os << "    // Testbench signals\n";
    for (const auto& input : inputs) {
        os << "    reg ";
        if (input->isSigned()) os << "signed ";
        if (input->getWidth() > 1) {
            os << "[" << (input->getWidth() - 1) << ":0] ";
        }
        os << input->getName() << ";\n";
    }

    for (const auto& output : outputs) {
        os << "    wire ";
        if (output->isSigned()) os << "signed ";
        if (output->getWidth() > 1) {
            os << "[" << (output->getWidth() - 1) << ":0] ";
        }
        os << output->getName() << ";\n";
    }

    os << "\n    // Instantiate DUT\n";
    os << "    " << generator_.getModuleName() << " dut (\n";

    for (size_t i = 0; i < inputs.size(); ++i) {
        os << "        ." << inputs[i]->getName() << "(" << inputs[i]->getName() << ")";
        if (i < inputs.size() - 1 || !outputs.empty()) os << ",";
        os << "\n";
    }

    for (size_t i = 0; i < outputs.size(); ++i) {
        os << "        ." << outputs[i]->getName() << "(" << outputs[i]->getName() << ")";
        if (i < outputs.size() - 1) os << ",";
        os << "\n";
    }

    os << "    );\n\n";

    os << "    // Test stimulus\n";
    os << "    initial begin\n";
    os << "        $dumpfile(\"" << generator_.getModuleName() << ".vcd\");\n";
    os << "        $dumpvars(0, tb_" << generator_.getModuleName() << ");\n\n";

    os << "        // Initialize inputs\n";
    for (const auto& input : inputs) {
        os << "        " << input->getName() << " = 0;\n";
    }

    os << "\n        // Apply random test vectors\n";
    os << "        repeat (100) begin\n";
    os << "            #10;\n";
    for (const auto& input : inputs) {
        os << "            " << input->getName() << " = $random;\n";
    }
    os << "        end\n\n";

    os << "        #100 $finish;\n";
    os << "    end\n\n";

    os << "    // Monitor outputs\n";
    os << "    initial begin\n";
    os << "        $monitor(\"Time=%0t\", $time";
    for (const auto& output : outputs) {
        os << ", \" " << output->getName() << "=%h\", " << output->getName();
    }
    os << ");\n";
    os << "    end\n\n";

    os << "endmodule\n";
}

std::string VerilogEmitter::getCurrentTimestamp() const {
    std::time_t now = std::time(nullptr);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}
