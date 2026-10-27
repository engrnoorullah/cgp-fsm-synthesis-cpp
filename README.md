# Cartesian genetic programming for FSM logic synthesis

This repository presents a runnable C++ edition of the CGP method from my 2014 MSc research at the University of Engineering and Technology, Peshawar. It evolves the combinational next-state and output logic of a finite state machine using two-input NAND and NOR gates.

The thesis is titled *Power Optimization of Finite State Machines Using Cartesian Genetic Programming*. Its experiments compared circuit gate counts. This program does not measure electrical power, prove a globally minimum circuit, or reproduce the thesis benchmark results. The small examples here are supplied to make the code easy to build and inspect.

## Build and run

Requirements: a C++17 compiler and Make. Python 3 is used only for CLI tests. No third-party libraries are required. The current version was tested with GCC on Linux.

```sh
make
./build/cgp train examples/xor.table build/xor.cgp --seed 10 --generations 10000
./build/cgp check build/xor.cgp examples/xor.table
./build/cgp eval build/xor.cgp 01
make test
```

The `eval` command in this example prints `1`. Training prints the seed, generation count, mismatched output bits, Boolean RMSE and number of active gates. A zero mismatch count means an exact match on the supplied table. Exit status is `0` for an exact match, `2` when a search or check is not exact, and `1` for invalid input or another error. An unfinished search still saves its best circuit.

Train the FSM example with:

```sh
./build/cgp train examples/sequence_101.table build/sequence_101.cgp --seed 10 --generations 100000
./build/cgp check build/sequence_101.cgp examples/sequence_101.table
```

## Method and data

Each node stores a function gene and two connection genes. Connections can reference primary inputs or preceding nodes, which keeps the graph acyclic. Output genes select primary inputs or node outputs. The search uses one parent and eight mutated offspring by default; equal-fitness offspring may replace the parent. Mutation selects genes with replacement and can draw the same value again, as in ordinary point mutation.

The defaults are 50 nodes, seed 10, at most 100,000 generations and mutation draws equal to 10% of the genotype length, rounded down with a minimum of one. Override these using `--nodes`, `--seed`, `--generations`, `--offspring` and `--mutations`. The search is stochastic: different seeds or standard-library implementations can produce different circuits. A generation bound is not a wall-clock limit.

The table format starts with the number of inputs and outputs, followed by input/output bit strings. For example:

```text
2 1
00 0
01 1
10 1
11 0
```

Duplicate inputs, non-binary values and inconsistent widths are rejected. Don't-care conditions are not supported. An exact match establishes correctness only for the provided rows; it does not establish anything about omitted inputs.

`sequence_101.table` describes a four-state Moore sequence detector. Inputs are the two current-state bits followed by the input bit. Outputs are the two next-state bits followed by the current-state detection bit. States `00`, `01`, `10`, `11` represent no prefix, `1`, `10`, and `101`, respectively. Output is high in state `11`. The table covers all eight state/input combinations. The synthesiser learns this combinational mapping; it does not clock state registers or perform state encoding.

Only nodes reachable from the output genes count as active gates. Gate count is reported, but it is not a secondary optimisation objective in this edition.

## Source history

My original C++ listing is preserved as text in `original/thesis_appendix_b.txt`, extracted from Appendix B of the supplied MSc thesis Word document. The listing was embedded in the thesis; a separate original `.cpp` file was not present in the supplied archive. Checksums are recorded in `original/SHA256.json`.

The current `src/` implementation was cleaned and partly reimplemented in 2026. It retains the NAND/NOR representation and parent-plus-offspring search, while replacing fixed Windows paths, non-standard arrays and the interactive menu. It also corrects errors in the printed fitness routine and adds input validation, bounded search and tests. See [the change record](docs/CHANGES.md). The original listing, rather than the new file timestamps, documents the historical work.

Author of the original MSc work: Noor Ullah. CGP is an established method; the representation and evolutionary approach are not claimed as inventions of this repository. See J. F. Miller and P. Thomson, “Cartesian Genetic Programming,” EuroGP 2000, and the references in the thesis.

No redistribution licence has been selected for this repository. Public availability alone does not grant an open-source licence.
