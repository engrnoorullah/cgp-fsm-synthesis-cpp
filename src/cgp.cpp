#include "cgp.hpp"

#include <algorithm>
#include <istream>
#include <limits>
#include <ostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace cgp {
namespace {
std::size_t draw(std::mt19937& rng, std::size_t upper) {
    return std::uniform_int_distribution<std::size_t>(0, upper - 1)(rng);
}
void dimensions(std::size_t inputs, std::size_t outputs, std::size_t nodes) {
    if (inputs == 0 || inputs > 16 || outputs == 0 || outputs > 16 ||
        nodes == 0 || nodes > 512)
        throw std::runtime_error("expected 1-16 inputs/outputs and 1-512 nodes");
}
void check_table(const Table& table) {
    dimensions(table.inputs, table.outputs, 1);
    if (table.rows.empty() || table.rows.size() > 65536)
        throw std::runtime_error("expected 1-65536 truth-table rows");
    std::set<Bits> seen;
    for (const auto& row : table.rows) {
        if (row.input.size() != table.inputs || row.output.size() != table.outputs)
            throw std::runtime_error("truth-table width mismatch");
        for (int bit : row.input) if (bit != 0 && bit != 1)
            throw std::runtime_error("input is not a bit");
        for (int bit : row.output) if (bit != 0 && bit != 1)
            throw std::runtime_error("output is not a bit");
        if (!seen.insert(row.input).second)
            throw std::runtime_error("duplicate truth-table input");
    }
}
}

Bits parse_bits(const std::string& text, std::size_t expected) {
    if (text.size() != expected) throw std::runtime_error("bit-vector width mismatch");
    Bits bits;
    for (char c : text) {
        if (c != '0' && c != '1') throw std::runtime_error("only 0 and 1 are supported");
        bits.push_back(c - '0');
    }
    return bits;
}

Table read_table(std::istream& in) {
    Table table{};
    if (!(in >> table.inputs >> table.outputs)) throw std::runtime_error("missing table dimensions");
    dimensions(table.inputs, table.outputs, 1);
    std::string input, output;
    while (in >> input) {
        if (!(in >> output)) throw std::runtime_error("missing desired output");
        if (table.rows.size() >= 65536) throw std::runtime_error("too many rows");
        table.rows.push_back({parse_bits(input, table.inputs),parse_bits(output, table.outputs)});
    }
    if (!in.eof()) throw std::runtime_error("cannot read truth table");
    check_table(table);
    return table;
}

void validate(const Circuit& circuit) {
    dimensions(circuit.inputs, circuit.outputs.size(), circuit.nodes.size());
    for (std::size_t i = 0; i < circuit.nodes.size(); ++i) {
        const auto& node = circuit.nodes[i];
        if ((node.function != 0 && node.function != 1) ||
            node.a >= circuit.inputs + i || node.b >= circuit.inputs + i)
            throw std::runtime_error("invalid function or forward node connection");
    }
    for (auto index : circuit.outputs)
        if (index >= circuit.inputs + circuit.nodes.size())
            throw std::runtime_error("output index out of range");
}

Bits evaluate(const Circuit& circuit, const Bits& input) {
    validate(circuit);
    if (input.size() != circuit.inputs) throw std::runtime_error("wrong number of inputs");
    Bits values = input;
    for (auto bit : input) if (bit != 0 && bit != 1) throw std::runtime_error("input is not a bit");
    for (const auto& node : circuit.nodes) {
        const bool a = values[node.a] != 0, b = values[node.b] != 0;
        values.push_back(node.function == 0 ? !(a && b) : !(a || b));
    }
    Bits output;
    for (auto index : circuit.outputs) output.push_back(values[index]);
    return output;
}

std::size_t mismatches(const Circuit& circuit, const Table& table) {
    if (circuit.inputs != table.inputs || circuit.outputs.size() != table.outputs)
        throw std::runtime_error("circuit and table dimensions differ");
    std::size_t count = 0;
    for (const auto& row : table.rows) {
        if (row.output.size() != table.outputs)
            throw std::runtime_error("truth-table output width mismatch");
        const auto actual = evaluate(circuit, row.input);
        for (std::size_t i = 0; i < actual.size(); ++i) count += actual[i] != row.output[i];
    }
    return count;
}

std::size_t active_nodes(const Circuit& circuit) {
    validate(circuit);
    std::vector<bool> used(circuit.inputs + circuit.nodes.size(), false);
    for (auto output : circuit.outputs) used[output] = true;
    std::size_t count = 0;
    for (std::size_t i = circuit.nodes.size(); i-- > 0;) {
        if (!used[circuit.inputs + i]) continue;
        ++count;
        used[circuit.nodes[i].a] = used[circuit.nodes[i].b] = true;
    }
    return count;
}

Circuit random_circuit(std::size_t inputs, std::size_t outputs,
                       std::size_t nodes, std::mt19937& rng) {
    dimensions(inputs, outputs, nodes);
    Circuit circuit{inputs, {}, {}};
    for (std::size_t i = 0; i < nodes; ++i)
        circuit.nodes.push_back({static_cast<int>(draw(rng,2)),draw(rng,inputs+i),draw(rng,inputs+i)});
    for (std::size_t i = 0; i < outputs; ++i) circuit.outputs.push_back(draw(rng,inputs+nodes));
    return circuit;
}

Circuit mutate(const Circuit& parent, std::size_t mutations, std::mt19937& rng) {
    validate(parent);
    Circuit child = parent;
    const auto genes = child.nodes.size()*3 + child.outputs.size();
    for (std::size_t i = 0; i < mutations; ++i) {
        const auto gene = draw(rng,genes);
        if (gene >= child.nodes.size()*3) {
            child.outputs[gene-child.nodes.size()*3] = draw(rng,child.inputs+child.nodes.size());
        } else {
            const auto node = gene/3;
            if (gene%3 == 0) child.nodes[node].function = static_cast<int>(draw(rng,2));
            else if (gene%3 == 1) child.nodes[node].a = draw(rng,child.inputs+node);
            else child.nodes[node].b = draw(rng,child.inputs+node);
        }
    }
    return child;
}

Result evolve(const Table& table, std::size_t nodes, std::size_t offspring,
              std::size_t mutations, std::size_t generations, std::uint32_t seed) {
    check_table(table);
    if (offspring == 0 || offspring > 64 || mutations == 0 || mutations > 10000 ||
        generations > 1000000) throw std::runtime_error("invalid search limits");
    std::mt19937 rng(seed);
    auto parent = random_circuit(table.inputs,table.outputs,nodes,rng);
    auto best = mismatches(parent,table);
    for (std::size_t generation = 0; generation < generations && best != 0; ++generation) {
        auto selected = parent;
        auto selected_error = best;
        // All offspring descend from the same parent; equal-fitness offspring may replace it.
        for (std::size_t i = 0; i < offspring; ++i) {
            auto candidate = mutate(parent,mutations,rng);
            const auto error = mismatches(candidate,table);
            if (error <= selected_error) { selected = std::move(candidate); selected_error = error; }
        }
        parent = std::move(selected);
        best = selected_error;
        if (best == 0) return {parent,best,generation+1};
    }
    return {parent,best,best == 0 ? 0 : generations};
}

void save(std::ostream& out, const Circuit& circuit) {
    validate(circuit);
    out << "CGP1 " << circuit.inputs << ' ' << circuit.outputs.size() << ' ' << circuit.nodes.size() << '\n';
    for (auto node : circuit.nodes) out << node.function << ' ' << node.a << ' ' << node.b << '\n';
    for (auto index : circuit.outputs) out << index << ' ';
    out << '\n';
    if (!out) throw std::runtime_error("cannot write circuit");
}

Circuit load(std::istream& in) {
    std::string magic, extra;
    std::size_t inputs, outputs, nodes;
    if (!(in >> magic >> inputs >> outputs >> nodes) || magic != "CGP1")
        throw std::runtime_error("invalid circuit header");
    dimensions(inputs,outputs,nodes);
    Circuit circuit{inputs,std::vector<Node>(nodes),std::vector<std::size_t>(outputs)};
    for (auto& node : circuit.nodes)
        if (!(in >> node.function >> node.a >> node.b)) throw std::runtime_error("incomplete circuit");
    for (auto& index : circuit.outputs)
        if (!(in >> index)) throw std::runtime_error("missing circuit outputs");
    if (in >> extra) throw std::runtime_error("trailing circuit data");
    validate(circuit);
    return circuit;
}
} // namespace cgp
