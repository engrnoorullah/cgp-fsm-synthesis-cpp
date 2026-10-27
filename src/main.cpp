#include "cgp.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

static std::size_t number(const std::string& text, std::size_t maximum) {
    if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos)
        throw std::runtime_error("expected an unsigned integer");
    std::size_t used = 0;
    const auto value = std::stoull(text,&used);
    if (used != text.size() || value > maximum) throw std::runtime_error("numeric argument out of range");
    return static_cast<std::size_t>(value);
}

static void report(const cgp::Circuit& circuit, const cgp::Table& table) {
    const auto errors = cgp::mismatches(circuit,table);
    std::cout << "mismatches=" << errors << " total_bits=" << table.rows.size()*table.outputs
              << " rmse=" << std::sqrt(static_cast<double>(errors)/(table.rows.size()*table.outputs))
              << " active_nodes=" << cgp::active_nodes(circuit) << '\n';
}

int main(int argc, char** argv) {
    try {
        if (argc < 2) throw std::runtime_error("missing command");
        const std::string command = argv[1];
        if (command == "train" && argc >= 4 && (argc-4)%2 == 0) {
            std::size_t nodes=50, offspring=8, generations=100000, seed=10, mutations=0;
            for (int i=4; i<argc; i+=2) {
                const std::string option=argv[i];
                const auto value=number(argv[i+1],4294967295ULL);
                if (option=="--nodes") nodes=value;
                else if (option=="--offspring") offspring=value;
                else if (option=="--generations") generations=value;
                else if (option=="--seed") seed=value;
                else if (option=="--mutations") mutations=value;
                else throw std::runtime_error("unknown option: "+option);
            }
            std::ifstream input(argv[2]);
            if (!input) throw std::runtime_error("cannot open truth table");
            const auto table=cgp::read_table(input);
            if (nodes==0 || nodes>512) throw std::runtime_error("nodes must be 1-512");
            if (mutations==0) mutations=std::max<std::size_t>(1,(3*nodes+table.outputs)/10);
            const auto result=cgp::evolve(table,nodes,offspring,mutations,generations,static_cast<std::uint32_t>(seed));
            std::ofstream output(argv[3]);
            if (!output) throw std::runtime_error("cannot open output circuit");
            cgp::save(output,result.circuit);
            std::cout << "seed=" << seed << " generations=" << result.generations << '\n';
            report(result.circuit,table);
            return result.mismatches == 0 ? 0 : 2;
        }
        if (command == "check" && argc == 4) {
            std::ifstream circuit_file(argv[2]), table_file(argv[3]);
            if (!circuit_file || !table_file) throw std::runtime_error("cannot open input file");
            const auto circuit=cgp::load(circuit_file);
            const auto table=cgp::read_table(table_file);
            report(circuit,table);
            return cgp::mismatches(circuit,table)==0 ? 0 : 2;
        }
        if (command == "eval" && argc == 4) {
            std::ifstream input(argv[2]);
            if (!input) throw std::runtime_error("cannot open circuit");
            const auto circuit=cgp::load(input);
            for (auto bit : cgp::evaluate(circuit,cgp::parse_bits(argv[3],circuit.inputs))) std::cout << bit;
            std::cout << '\n';
            return 0;
        }
        throw std::runtime_error("invalid command or argument count");
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << "\n"
                  << "Usage:\n  cgp train TABLE MODEL [--seed N] [--nodes N] [--offspring N]\n"
                  << "                         [--mutations N] [--generations N]\n"
                  << "  cgp check MODEL TABLE\n  cgp eval MODEL INPUT_BITS\n";
        return 1;
    }
}
