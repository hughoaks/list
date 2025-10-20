#include "config.h"
#include "netlist_generator.h"
#include "verilog_emitter.h"
#include <iostream>
#include <fstream>
#include <cstring>

void printUsage(const char* program_name) {
    std::cout << "Verilog Datapath Generator - Random netlist generator for synthesis benchmarking\n\n";
    std::cout << "Usage: " << program_name << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -c, --config <file>      Load configuration from file\n";
    std::cout << "  -o, --output <file>      Output Verilog file (default: output.v)\n";
    std::cout << "  -m, --module <name>      Module name (default: random_datapath)\n";
    std::cout << "  -n, --num-ops <n>        Number of operations (default: 50)\n";
    std::cout << "  -i, --inputs <n>         Number of inputs (default: 8)\n";
    std::cout << "  -O, --outputs <n>        Number of outputs (default: 4)\n";
    std::cout << "  -s, --seed <n>           Random seed (default: current time)\n";
    std::cout << "  -d, --depth <n>          Maximum logic depth (default: 10)\n";
    std::cout << "  -p, --pipeline <n>       Number of pipeline stages (default: 0)\n";
    std::cout << "  -t, --testbench          Generate testbench\n";
    std::cout << "  -v, --verbose            Verbose output\n";
    std::cout << "  -h, --help               Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " -n 100 -i 16 -O 8 -o large_datapath.v\n";
    std::cout << "  " << program_name << " -c my_config.txt -t\n";
    std::cout << "  " << program_name << " -s 12345 -n 200 -v\n\n";
}

int main(int argc, char* argv[]) {
    GeneratorConfig config;
    bool generate_testbench = false;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                if (!config.loadFromFile(argv[++i])) {
                    std::cerr << "Failed to load configuration file\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: --config requires an argument\n";
                return 1;
            }
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                config.output_file = argv[++i];
            } else {
                std::cerr << "Error: --output requires an argument\n";
                return 1;
            }
        } else if (arg == "-m" || arg == "--module") {
            if (i + 1 < argc) {
                config.module_name = argv[++i];
            } else {
                std::cerr << "Error: --module requires an argument\n";
                return 1;
            }
        } else if (arg == "-n" || arg == "--num-ops") {
            if (i + 1 < argc) {
                config.num_operations = std::atoi(argv[++i]);
            } else {
                std::cerr << "Error: --num-ops requires an argument\n";
                return 1;
            }
        } else if (arg == "-i" || arg == "--inputs") {
            if (i + 1 < argc) {
                config.num_inputs = std::atoi(argv[++i]);
            } else {
                std::cerr << "Error: --inputs requires an argument\n";
                return 1;
            }
        } else if (arg == "-O" || arg == "--outputs") {
            if (i + 1 < argc) {
                config.num_outputs = std::atoi(argv[++i]);
            } else {
                std::cerr << "Error: --outputs requires an argument\n";
                return 1;
            }
        } else if (arg == "-s" || arg == "--seed") {
            if (i + 1 < argc) {
                config.seed = std::atoi(argv[++i]);
            } else {
                std::cerr << "Error: --seed requires an argument\n";
                return 1;
            }
        } else if (arg == "-d" || arg == "--depth") {
            if (i + 1 < argc) {
                config.max_depth = std::atoi(argv[++i]);
            } else {
                std::cerr << "Error: --depth requires an argument\n";
                return 1;
            }
        } else if (arg == "-p" || arg == "--pipeline") {
            if (i + 1 < argc) {
                config.num_pipeline_stages = std::atoi(argv[++i]);
            } else {
                std::cerr << "Error: --pipeline requires an argument\n";
                return 1;
            }
        } else if (arg == "-t" || arg == "--testbench") {
            generate_testbench = true;
        } else if (arg == "-v" || arg == "--verbose") {
            config.verbose = true;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    // Validate configuration
    if (!config.validate()) {
        std::cerr << "Invalid configuration\n";
        return 1;
    }

    // Print configuration if verbose
    if (config.verbose) {
        config.print();
    }

    try {
        // Generate netlist
        NetlistGenerator generator(config);
        generator.generate();

        // Emit Verilog
        VerilogEmitter emitter(generator);

        // Write to file
        std::ofstream output_file(config.output_file);
        if (!output_file.is_open()) {
            std::cerr << "Error: Could not open output file: " << config.output_file << "\n";
            return 1;
        }

        emitter.emit(output_file);
        output_file.close();

        std::cout << "Successfully generated: " << config.output_file << "\n";

        // Generate testbench if requested
        if (generate_testbench) {
            std::string tb_filename = "tb_" + config.output_file;
            std::ofstream tb_file(tb_filename);
            if (!tb_file.is_open()) {
                std::cerr << "Warning: Could not create testbench file: " << tb_filename << "\n";
            } else {
                emitter.emitTestbench(tb_file);
                tb_file.close();
                std::cout << "Successfully generated testbench: " << tb_filename << "\n";
            }
        }

        if (config.verbose) {
            std::cout << "\nGeneration complete!\n";
            std::cout << "To synthesize with your favorite tool:\n";
            std::cout << "  Yosys:    yosys -p 'synth -top " << config.module_name << "' " << config.output_file << "\n";
            std::cout << "  Vivado:   vivado -mode batch -source synth.tcl\n";
            std::cout << "  Quartus:  quartus_sh --flow compile <project>\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
