#include "config.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

GeneratorConfig::GeneratorConfig()
    : seed(static_cast<unsigned int>(std::time(nullptr)))
    , module_name("random_datapath")
    , num_inputs(8)
    , num_outputs(4)
    , input_width_min(8)
    , input_width_max(32)
    , output_width_min(8)
    , output_width_max(32)
    , num_operations(50)
    , max_depth(10)
    , num_pipeline_stages(0)
    , weight_arithmetic(0.3)
    , weight_logical(0.2)
    , weight_comparison(0.1)
    , weight_shift(0.15)
    , weight_mux(0.15)
    , weight_concat(0.05)
    , weight_reduction(0.05)
    , weight_add(0.3)
    , weight_sub(0.3)
    , weight_mult(0.25)
    , weight_div(0.1)
    , weight_mod(0.05)
    , weight_sll(0.4)
    , weight_srl(0.4)
    , weight_sra(0.2)
    , use_parameters(false)
    , use_signed(true)
    , use_tristate(false)
    , generate_testbench(false)
    , output_file("output.v")
    , verbose(false)
{
}

bool GeneratorConfig::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int line_num = 0;
    while (std::getline(file, line)) {
        line_num++;

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        // Parse key=value
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            // Parse configuration values
            try {
                if (key == "seed") seed = std::stoul(value);
                else if (key == "module_name") module_name = value;
                else if (key == "num_inputs") num_inputs = std::stoi(value);
                else if (key == "num_outputs") num_outputs = std::stoi(value);
                else if (key == "input_width_min") input_width_min = std::stoi(value);
                else if (key == "input_width_max") input_width_max = std::stoi(value);
                else if (key == "output_width_min") output_width_min = std::stoi(value);
                else if (key == "output_width_max") output_width_max = std::stoi(value);
                else if (key == "num_operations") num_operations = std::stoi(value);
                else if (key == "max_depth") max_depth = std::stoi(value);
                else if (key == "num_pipeline_stages") num_pipeline_stages = std::stoi(value);
                else if (key == "weight_arithmetic") weight_arithmetic = std::stod(value);
                else if (key == "weight_logical") weight_logical = std::stod(value);
                else if (key == "weight_comparison") weight_comparison = std::stod(value);
                else if (key == "weight_shift") weight_shift = std::stod(value);
                else if (key == "weight_mux") weight_mux = std::stod(value);
                else if (key == "weight_concat") weight_concat = std::stod(value);
                else if (key == "weight_reduction") weight_reduction = std::stod(value);
                else if (key == "output_file") output_file = value;
                else if (key == "verbose") verbose = (value == "true" || value == "1");
                else {
                    std::cerr << "Warning: Unknown config key '" << key << "' at line " << line_num << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing line " << line_num << ": " << e.what() << std::endl;
                return false;
            }
        }
    }

    return validate();
}

bool GeneratorConfig::validate() const {
    if (num_inputs < 1 || num_inputs > 1000) {
        std::cerr << "Error: num_inputs must be between 1 and 1000" << std::endl;
        return false;
    }

    if (num_outputs < 1 || num_outputs > 1000) {
        std::cerr << "Error: num_outputs must be between 1 and 1000" << std::endl;
        return false;
    }

    if (input_width_min < 1 || input_width_min > input_width_max) {
        std::cerr << "Error: Invalid input width range" << std::endl;
        return false;
    }

    if (output_width_min < 1 || output_width_min > output_width_max) {
        std::cerr << "Error: Invalid output width range" << std::endl;
        return false;
    }

    if (num_operations < 1) {
        std::cerr << "Error: num_operations must be at least 1" << std::endl;
        return false;
    }

    double total_weight = weight_arithmetic + weight_logical + weight_comparison +
                         weight_shift + weight_mux + weight_concat + weight_reduction;
    if (total_weight <= 0.0) {
        std::cerr << "Error: Total operation weight must be positive" << std::endl;
        return false;
    }

    return true;
}

void GeneratorConfig::print() const {
    std::cout << "=== Generator Configuration ===" << std::endl;
    std::cout << "Seed: " << seed << std::endl;
    std::cout << "Module: " << module_name << std::endl;
    std::cout << "Inputs: " << num_inputs << " (width: " << input_width_min << "-" << input_width_max << ")" << std::endl;
    std::cout << "Outputs: " << num_outputs << " (width: " << output_width_min << "-" << output_width_max << ")" << std::endl;
    std::cout << "Operations: " << num_operations << std::endl;
    std::cout << "Max depth: " << max_depth << std::endl;
    std::cout << "Pipeline stages: " << num_pipeline_stages << std::endl;
    std::cout << "Output file: " << output_file << std::endl;
    std::cout << "================================" << std::endl;
}
