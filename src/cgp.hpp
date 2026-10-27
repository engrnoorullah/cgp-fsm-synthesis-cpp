#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <random>
#include <string>
#include <vector>

namespace cgp {

using Bits = std::vector<int>;
struct Row { Bits input, output; };
struct Table { std::size_t inputs, outputs; std::vector<Row> rows; };
struct Node { int function; std::size_t a, b; }; // 0: NAND, 1: NOR
struct Circuit {
    std::size_t inputs;
    std::vector<Node> nodes;
    std::vector<std::size_t> outputs;
};
struct Result { Circuit circuit; std::size_t mismatches, generations; };

Table read_table(std::istream& in);
void validate(const Circuit& circuit);
Bits evaluate(const Circuit& circuit, const Bits& input);
std::size_t mismatches(const Circuit& circuit, const Table& table);
std::size_t active_nodes(const Circuit& circuit);
Circuit random_circuit(std::size_t inputs, std::size_t outputs,
                       std::size_t nodes, std::mt19937& rng);
Circuit mutate(const Circuit& parent, std::size_t mutations, std::mt19937& rng);
Result evolve(const Table& table, std::size_t nodes, std::size_t offspring,
              std::size_t mutations, std::size_t generations, std::uint32_t seed);
void save(std::ostream& out, const Circuit& circuit);
Circuit load(std::istream& in);
Bits parse_bits(const std::string& text, std::size_t expected);

} // namespace cgp
