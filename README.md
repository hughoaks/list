# Verilog Datapath Generator

A full-featured random Verilog datapath generator written in C++ for testing and benchmarking synthesis tools. This tool generates complex, configurable datapaths with various operation types to challenge synthesis tools in area minimization and timing optimization.

## Features

### Comprehensive Operation Support
- **Arithmetic Operations**: Add, Subtract, Multiply, Divide, Modulo
- **Logical Operations**: AND, OR, XOR, NOT, NAND, NOR, XNOR
- **Comparison Operations**: Equal, Not Equal, Less Than, Greater Than, LTE, GTE
- **Shift Operations**: Shift Left Logical, Shift Right Logical, Shift Right Arithmetic
- **Reduction Operations**: Reduction AND, OR, XOR, NAND, NOR, XNOR
- **Multiplexers**: 2-to-1 and 4-to-1 muxes with ternary operators
- **Concatenation**: Multi-signal concatenation

### Control Flow and Optimization Testing (NEW!)
- **Case Statements**: Generate case/switch statements for FSM and decoder optimization testing
- **If-Else Chains**: Create mutually exclusive conditional branches for testing resource sharing
- **Resource Sharing Opportunities**: Intentionally create operations in mutually exclusive paths that synthesis tools can potentially share (e.g., multiple multipliers that are never active simultaneously)
- **Temporal Sharing**: Generate patterns where expensive operations (multipliers, dividers) can share hardware units

### Configurable Parameters
- Number of inputs/outputs with configurable bit widths
- Number of operations and maximum logic depth
- Pipeline stage support
- Weighted random operation selection
- Signed/unsigned signal support
- Reproducible generation via seed control

### Output Capabilities
- Clean, synthesizable Verilog HDL output
- Optional testbench generation
- Detailed comments and metadata
- Compatible with all major synthesis tools (Yosys, Vivado, Quartus, etc.)

## Building

### Prerequisites
- CMake 3.14 or later
- C++17-compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)

### Build Instructions

```bash
# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# Optional: Install system-wide
sudo make install
```

## Usage

### Basic Usage

Generate a simple datapath with default parameters:
```bash
./verilog_gen
```

Generate with custom parameters:
```bash
./verilog_gen -n 100 -i 16 -O 8 -o my_datapath.v
```

Generate with testbench:
```bash
./verilog_gen -n 50 -t -v
```

### Using Configuration Files

Create a configuration file (see `examples/` directory) and use it:
```bash
./verilog_gen -c examples/basic_config.txt
```

Configuration file format:
```
# Comments start with #
seed = 42
module_name = my_datapath
num_inputs = 8
num_outputs = 4
num_operations = 50
weight_arithmetic = 0.4
weight_logical = 0.2
# ... etc
```

### Command-Line Options

```
Options:
  -c, --config <file>      Load configuration from file
  -o, --output <file>      Output Verilog file (default: output.v)
  -m, --module <name>      Module name (default: random_datapath)
  -n, --num-ops <n>        Number of operations (default: 50)
  -i, --inputs <n>         Number of inputs (default: 8)
  -O, --outputs <n>        Number of outputs (default: 4)
  -s, --seed <n>           Random seed (default: current time)
  -d, --depth <n>          Maximum logic depth (default: 10)
  -p, --pipeline <n>       Number of pipeline stages (default: 0)
  -t, --testbench          Generate testbench
  -v, --verbose            Verbose output
  -h, --help               Show help message
```

## Configuration Parameters

### Module Properties
- `seed`: Random number generator seed (for reproducibility)
- `module_name`: Name of the generated Verilog module
- `num_inputs`: Number of module inputs
- `num_outputs`: Number of module outputs
- `input_width_min/max`: Range of input signal widths (in bits)
- `output_width_min/max`: Range of output signal widths (in bits)

### Datapath Complexity
- `num_operations`: Total number of operations to generate
- `max_depth`: Maximum combinational logic depth
- `num_pipeline_stages`: Number of pipeline stages (0 for combinational only)

### Operation Weights
Control the probability of generating each operation type (will be automatically normalized):
- `weight_arithmetic`: Arithmetic operations (+, -, *, /, %)
- `weight_logical`: Logical operations (&, |, ^, ~)
- `weight_comparison`: Comparison operations (==, !=, <, >, etc.)
- `weight_shift`: Shift operations (<<, >>, >>>)
- `weight_mux`: Multiplexer operations
- `weight_concat`: Concatenation operations
- `weight_reduction`: Reduction operations (&, |, ^)

### Fine-Grained Arithmetic Weights
- `weight_add`: Addition operations
- `weight_sub`: Subtraction operations
- `weight_mult`: Multiplication operations
- `weight_div`: Division operations
- `weight_mod`: Modulo operations

### Fine-Grained Shift Weights
- `weight_sll`: Shift left logical
- `weight_srl`: Shift right logical
- `weight_sra`: Shift right arithmetic

### Control Flow and Resource Sharing (NEW!)
Control structures for testing synthesis optimization:
- `generate_case_statements`: Enable case statement generation (true/false)
- `generate_if_else_chains`: Enable if-else chain generation (true/false)
- `generate_sharing_opportunities`: Create resource sharing opportunities (true/false)
- `num_case_statements`: Number of case statements to generate
- `num_if_else_chains`: Number of if-else chains to generate
- `cases_per_statement`: Number of cases in each case statement (default: 4)

**Why these features matter for synthesis testing:**
- **Case statements** test FSM optimization, decoder synthesis, and state machine inference
- **If-else chains** create mutually exclusive paths where synthesis tools can share expensive operations
- **Sharing opportunities** test how well tools identify and exploit temporal resource sharing (e.g., one multiplier shared across multiple mutually exclusive operations)

## Example Configurations

### Basic Datapath (Area Optimization Test)
```bash
./verilog_gen -c examples/basic_config.txt
```
Generates a simple datapath with balanced operation mix, suitable for testing basic area optimization capabilities.

### Complex Datapath (Stress Test)
```bash
./verilog_gen -c examples/complex_config.txt
```
Generates a large, complex datapath with many operations and wide data paths, suitable for stress testing synthesis runtime and memory usage.

### DSP-Heavy Datapath (DSP Mapping Test)
```bash
./verilog_gen -c examples/dsp_heavy_config.txt
```
Generates a multiplier-heavy datapath to test DSP block inference and mapping in FPGAs.

### Resource Sharing Test (NEW!)
```bash
./verilog_gen -c examples/sharing_config.txt
```
Generates a design with case statements, if-else chains, and mutually exclusive operations to test synthesis tool resource sharing optimization. This creates scenarios where:
- Multiple multipliers exist in different case branches (can potentially share a single multiplier)
- Operations in mutually exclusive if-else branches can share functional units
- Synthesis tools must identify temporal sharing opportunities

Example output includes:
- **Case statements** with operations in each branch that could share resources
- **If-else chains** with expensive operations (multipliers) in mutually exclusive paths
- **Sharing groups** with operations controlled by different enable signals

This is ideal for benchmarking how well synthesis tools perform resource sharing and area optimization.

## Synthesis Testing

### With Yosys (Open Source)
```bash
# Generate the design
./verilog_gen -n 100 -o test_datapath.v

# Synthesize with Yosys
yosys -p "read_verilog test_datapath.v; synth -top random_datapath; stat"

# For FPGA targeting (e.g., Lattice iCE40)
yosys -p "read_verilog test_datapath.v; synth_ice40 -top random_datapath; stat"
```

### With Xilinx Vivado
```tcl
# Create Vivado TCL script (synth.tcl):
read_verilog test_datapath.v
synth_design -top random_datapath -part xc7a35tcpg236-1
report_utilization
report_timing_summary

# Run synthesis
vivado -mode batch -source synth.tcl
```

### With Intel Quartus
```bash
# Add to Quartus project and compile
quartus_sh --flow compile <project_name>
```

## Use Cases

### 1. Synthesis Tool Benchmarking
Compare area and timing results across different synthesis tools:
```bash
# Generate the same design with fixed seed
./verilog_gen -s 42 -n 200 -o benchmark.v

# Synthesize with multiple tools and compare results
```

### 2. Optimization Algorithm Testing
Test new optimization algorithms or settings:
```bash
# Generate challenging designs with different characteristics
./verilog_gen -c dsp_heavy_config.txt    # Test DSP mapping
./verilog_gen -c complex_config.txt      # Test general optimization
```

### 3. Regression Testing
Ensure synthesis tool updates don't degrade quality:
```bash
# Generate a suite of test designs
for seed in {1..100}; do
    ./verilog_gen -s $seed -o test_$seed.v
done
```

### 4. Educational Purposes
Study synthesis optimization techniques:
```bash
# Generate simple designs to understand optimization
./verilog_gen -n 20 -v -t -o simple.v
# Synthesize and analyze the results
```

## Output Format

The generated Verilog files include:
- Header comments with generation metadata
- Module declaration with all ports
- Internal signal declarations (wires and registers)
- Combinational logic assignments
- Sequential logic blocks (if pipeline stages > 0)

Example output structure:
```verilog
// ============================================================================
// Random Verilog Datapath Generator
// Generated: 2025-01-15 10:30:45
// ============================================================================

module random_datapath (
    input [7:0] in_0,
    input [15:0] in_1,
    // ...
    output [15:0] out_0,
    output [7:0] out_1
);

    // Internal wires
    wire [15:0] wire_0;
    wire [31:0] wire_1;
    // ...

    // Combinational Logic
    assign wire_0 = (in_0 + in_1);
    assign wire_1 = (wire_0 * in_2);
    // ...

endmodule
```

## Implementation Details

### Architecture
- **Config System**: Flexible configuration with file and CLI support
- **Signal Management**: Type-safe signal representation (input/output/wire/reg)
- **Operation AST**: Clean operation abstraction with automatic Verilog generation
- **Netlist Generator**: Weighted random generation with depth tracking
- **Verilog Emitter**: Clean code generation with proper formatting

### Random Generation Strategy
1. Generate input/output signals with random widths
2. Create operations based on weighted probabilities
3. Select random available signals as operands
4. Generate output wires with appropriate widths
5. Track signal dependencies and depths
6. Connect final operations to module outputs

### Width Inference
The generator automatically infers appropriate output widths:
- **Arithmetic**: Max of input widths (multiply extends)
- **Logical**: Max of input widths
- **Comparison**: Always 1-bit
- **Shift**: Same as shifted operand
- **Reduction**: Always 1-bit
- **Mux**: Max of data input widths
- **Concat**: Sum of input widths

## Advanced Features

### Custom Operation Distributions
Create focused benchmarks by adjusting operation weights in the configuration file. For example, to create a multiplier-intensive design:

```
weight_arithmetic = 0.8
weight_mult = 0.7  # Within arithmetic ops, favor multipliers
weight_add = 0.2
weight_sub = 0.1
```

### Reproducible Generation
Use the same seed to generate identical designs:
```bash
./verilog_gen -s 42 -o design1.v
./verilog_gen -s 42 -o design2.v
# design1.v and design2.v are identical
```

### Batch Generation
Generate many random designs for statistical analysis:
```bash
#!/bin/bash
for i in {1..100}; do
    ./verilog_gen -s $i -n 100 -o designs/design_$i.v
    # Synthesize and collect statistics
done
```

## Limitations and Future Work

### Current Limitations
- No explicit FSM generation
- Limited memory element support (only pipeline registers)
- No hierarchical module instantiation
- No parameterized modules

### Planned Enhancements
- [ ] Hierarchical design generation
- [ ] Memory (RAM/ROM) instances
- [ ] FSM generation
- [ ] More sophisticated width matching
- [ ] Dependency-aware depth assignment
- [ ] Pipeline stage balancing
- [ ] Custom constraint support
- [ ] Verilog-2005 and SystemVerilog output modes

## Contributing

Contributions are welcome! Areas of interest:
- Additional operation types
- Better width inference algorithms
- Pipeline stage optimization
- Testbench improvements
- Support for more synthesis tools

## License

This project is open source. Feel free to use, modify, and distribute.

## Author

Generated with Claude Code - An AI-powered development assistant.

## Acknowledgments

This tool was created to support synthesis tool development and benchmarking. Special thanks to the open-source EDA community.
