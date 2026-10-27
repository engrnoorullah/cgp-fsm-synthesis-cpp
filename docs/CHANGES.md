# Changes from the thesis listing

The `original/` listing is retained as evidence, not as a build target. It contains the text extracted from the thesis appendix and the references following it. Its formatting is the Word extraction, not an original source-file layout.

The maintained edition makes the following changes:

- Replaces `conio.h`, `D:\\...` paths and variable-length stack arrays with standard C++ containers and command-line file paths.
- Removes global row counters and evaluates each input row explicitly.
- Corrects the listing's undeclared `k`, `find0` and `outdesor` references through explicit parsing and data structures.
- Replaces the printed `RMSE` implementation. Its descending-loop conditions do not visit the intended elements, and comparing separate output magnitudes can conceal incorrect bits. Fitness is now the total count of mismatched Boolean outputs. Reported RMSE is `sqrt(mismatches / total_output_bits)`, which has the same ranking and zero-error condition as that count. This is a disclosed behavioural correction, not a claim of bit-for-bit reproduction.
- Makes connection bounds explicit for every node during both generation and mutation.
- Adds a finite generation limit and saves the best candidate when the search does not converge.
- Adds a versioned circuit format, independent checking, active-node reporting and complete small truth-table examples.
- Adds tests for NAND/NOR outputs, feed-forward constraints, mutation validity, persistence, invalid inputs and all XOR rows.

The selection still permits a neutral offspring to replace its parent. It does not add gate-count selection, power estimation, don't-care expansion or the original benchmark dataset. Results from this edition should not be presented as a rerun of the 2014 experiments.
