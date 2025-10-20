#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <random>

struct GeneratorConfig {
    // Randomization
    unsigned int seed;

    // Module properties
    std::string module_name;
    int num_inputs;
    int num_outputs;
    int input_width_min;
    int input_width_max;
    int output_width_min;
    int output_width_max;

    // Datapath complexity
    int num_operations;
    int max_depth;  // Maximum logic depth
    int num_pipeline_stages;  // 0 for combinational only

    // Operation weights (probability of selecting each operation type)
    double weight_arithmetic;  // +, -, *, /, %
    double weight_logical;     // &, |, ^, ~
    double weight_comparison;  // ==, !=, <, >, <=, >=
    double weight_shift;       // <<, >>, <<<, >>>
    double weight_mux;         // ternary operator
    double weight_concat;      // {a, b}
    double weight_reduction;   // &, |, ^

    // Arithmetic operation weights
    double weight_add;
    double weight_sub;
    double weight_mult;
    double weight_div;
    double weight_mod;

    // Shift operation weights
    double weight_sll;  // shift left logical
    double weight_srl;  // shift right logical
    double weight_sra;  // shift right arithmetic

    // Additional features
    bool use_parameters;
    bool use_signed;
    bool use_tristate;
    bool generate_testbench;

    // Control flow features (for testing synthesis optimization)
    bool generate_case_statements;
    bool generate_if_else_chains;
    bool generate_sharing_opportunities;  // Create ops that can share resources
    int num_case_statements;
    int num_if_else_chains;
    int cases_per_statement;  // Number of cases in each case statement

    // Output options
    std::string output_file;
    bool verbose;

    // Constructor with defaults
    GeneratorConfig();

    // Load from file
    bool loadFromFile(const std::string& filename);

    // Validation
    bool validate() const;

    // Print configuration
    void print() const;
};

#endif // CONFIG_H
