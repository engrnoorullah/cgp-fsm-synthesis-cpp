#include "../src/cgp.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

template<class F> void rejects(F fn) {
    bool rejected=false;
    try { fn(); } catch (const std::runtime_error&) { rejected=true; }
    assert(rejected);
}

int main() {
    cgp::Circuit nand{2,{{0,0,1}},{2}};
    cgp::Circuit nor{2,{{1,0,1}},{2}};
    for (int a=0;a<=1;++a) for(int b=0;b<=1;++b) {
        assert(cgp::evaluate(nand,{a,b})[0] == !(a && b));
        assert(cgp::evaluate(nor,{a,b})[0] == !(a || b));
    }
    // Different wrong output bits must not cancel when evaluating fitness.
    cgp::Table target{2,2,{{{0,1},{1,0}}}};
    cgp::Circuit identity{2,{{0,0,1}},{0,1}};
    assert(cgp::mismatches(identity,target)==2);
    rejects([&]{ cgp::mismatches(identity,{2,2,{{{0,1},{1}}}}); });
    assert(cgp::active_nodes(identity)==0);
    assert(cgp::active_nodes(nand)==1);
    rejects([]{ cgp::validate({1,{{0,1,0}},{1}}); });
    rejects([]{ cgp::validate({1,{{2,0,0}},{1}}); });
    rejects([]{ cgp::validate({1,{{0,0,0}},{2}}); });
    rejects([&]{ cgp::evaluate(nand,{0}); });
    rejects([&]{ cgp::evaluate(nand,{0,2}); });
    std::stringstream roundtrip;
    cgp::save(roundtrip,nand);
    auto restored=cgp::load(roundtrip);
    assert(cgp::evaluate(restored,{1,1})==cgp::Bits{0});
    std::stringstream partial("CGP1 2 1 1 0 0");
    rejects([&]{cgp::load(partial);});
    std::stringstream bad("1 1\n0 0\n0 1\n");
    rejects([&]{cgp::read_table(bad);});
    std::stringstream nonbinary("1 1\n0 x\n");
    rejects([&]{cgp::read_table(nonbinary);});
    std::mt19937 rng(42);
    auto circuit=cgp::random_circuit(3,3,50,rng);
    for(int i=0;i<500;++i) {
        circuit=cgp::mutate(circuit,25,rng);
        cgp::validate(circuit);
    }
    std::cout << "CGP unit tests passed\n";
}
