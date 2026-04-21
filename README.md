# validateutf8-experiments

This project contains benchmarks regarding fast UTF-8 validation. It is for research purposes only: not for production use. If you are not doing research, this repository is not for you!

The algorithm we designed is called lookup. We experimented with several variants (lookup2, lookup3, lookup4) that have similar performance. The lookup approach is one of the fastest ways to validate UTF-8 strings. Please see the file `src/generate/utf8_lookup4_algorithm.h` for details.

The algorithm of this repository has been included in production-ready libraries:

- [simdutf](https://github.com/simdutf/simdutf) C++ library is part of important systems such as Bun, Node.js, WebKit/Safari, etc.
- [SimdUnicode](https://github.com/simdutf/SimdUnicode) is a C# port of the validation algorithm, adapted for the .NET runtime.

## Code organization

The repository is organized around three main concerns: SIMD validation algorithms, benchmarking, and testing.

- `src/`: core UTF-8 validation code.
  - `src/generic/`: generic algorithm definitions (`utf8_lookup2_algorithm.h`, `utf8_lookup3_algorithm.h`, `utf8_lookup4_algorithm.h`, etc.).
  - `src/avx2/`, `src/sse/`, `src/neon/`: architecture-specific SIMD wrappers and implementations.
- `benchmarks/`: benchmark drivers and supporting utilities.
  - `benchmarks/benchmark.cpp`: runs algorithm comparisons on synthetic and real data.
  - `benchmarks/benchstream.cpp`: streaming-oriented benchmark.
  - `benchmarks/random_utf8.*`: randomized UTF-8 data generation.
- `tests/`: unit tests (`tests/unit.cpp`) validating correctness across implementations.
- `examples/`: sample real-world UTF-8 inputs used by benchmarks.
- `dependencies/`: third-party code used by benchmarks.
- `.github/workflows/`: CI configuration for build and test automation.

Typical workflow:

1. Build with CMake.
2. Run `./build/unit` to validate correctness.
3. Run `./build/benchmark` and `./build/benchstream` to compare performance.


## Hardware requirements

The code supports multiple processor architectures including x64 with AVX2, SSE, as well as ARM64 with NEON. 


## Reproducible experiments

To ensure that experiments are reproducible, we rely on a docker image. We recommend that you install docker under Linux. 

## Testing

Run the unit tests locally with:

```bash
cmake -B build
cmake --build build
ctest --test-dirs build
```

Starting in a bash shell do:

```
git clone https://github.com/lemire/validateutf8-experiments.git
cmake -B build
cmake --build build
./build/benchmark
./build/benchstream 1000
```

In some cases, you may need to run the benchmarks in privileged mode (sudo) to get performance counters.


## Example

```
./build/benchmark 

Running UTF8 validation benchmark.
The speed is normalized by the number of input bytes.

== file: examples/hongkong.html (1807 KB) ==
name               | ins/byte | br.miss/KB |    GHz |    GB/s | margin% | ins/cyc
-------------------+----------+------------+--------+---------+---------+--------
memcpy             |    0.000 |      0.001 |  3.497 |  17.862 |    1.69 |   0.001
fushia             |    6.641 |      8.187 |  3.493 |   2.612 |   26.77 |   4.966
fushia_ascii       |    2.738 |      7.617 |  3.493 |   4.236 |    4.04 |   3.320
fushia_ascii2      |    2.594 |      7.440 |  3.493 |   5.327 |    4.96 |   3.955
fushia_ascii4      |    2.707 |      8.602 |  3.493 |   4.363 |    5.53 |   3.381
utf8lib            |    6.724 |      8.057 |  3.493 |   2.570 |    2.80 |   4.947
dfa                |    9.000 |      0.003 |  3.489 |   0.581 |    1.15 |   1.500
fdfa               |    9.000 |      0.003 |  3.490 |   0.496 |    1.09 |   1.278
bdfa               |    8.000 |      0.002 |  3.489 |   0.581 |    1.18 |   1.333
dfa2               |    7.500 |      0.002 |  3.490 |   1.163 |    2.63 |   2.499
dfa3               |    7.000 |      0.003 |  3.489 |   1.722 |    1.44 |   3.456
dfa4               |    9.000 |      0.004 |  3.489 |   0.581 |    1.17 |   1.499
zwegneravx         |    0.440 |      2.777 |  3.496 |  19.748 |    3.40 |   2.487
lookup2            |    0.447 |      1.743 |  3.495 |  18.458 |    5.26 |   2.363
lookup3            |    0.417 |      1.711 |  3.495 |  19.325 |    2.79 |   2.305
lookup4            |    0.413 |      1.728 |  3.495 |  20.934 |    3.99 |   2.476
basic              |    0.609 |      1.645 |  3.495 |  15.483 |    2.19 |   2.698
range              |    0.567 |      1.600 |  3.494 |  16.024 |    3.42 |   2.599

== file: examples/twitter.json (631 KB) ==
name               | ins/byte | br.miss/KB |    GHz |    GB/s | margin% | ins/cyc
-------------------+----------+------------+--------+---------+---------+--------
memcpy             |    0.000 |      0.005 |  3.509 |  44.150 |    3.35 |   0.005
fushia             |    7.158 |      3.431 |  3.493 |   2.811 |   21.55 |   5.760
fushia_ascii       |    3.641 |      2.114 |  3.470 |   4.544 |    3.74 |   4.767
fushia_ascii2      |    3.469 |      1.639 |  3.495 |   6.142 |    3.20 |   6.096
fushia_ascii4      |    3.487 |      1.823 |  3.494 |   5.484 |    4.52 |   5.472
utf8lib            |    7.310 |      3.894 |  3.493 |   2.577 |    0.54 |   5.392
dfa                |    9.000 |      0.006 |  3.490 |   0.582 |    0.17 |   1.500
fdfa               |    9.000 |      0.006 |  3.490 |   0.496 |    0.93 |   1.278
bdfa               |    8.000 |      0.006 |  3.490 |   0.582 |    1.12 |   1.333
dfa2               |    7.500 |      0.006 |  3.493 |   1.164 |    0.99 |   2.499
dfa3               |    7.000 |      0.008 |  3.493 |   1.730 |    1.27 |   3.467
dfa4               |    9.000 |      0.010 |  3.490 |   0.582 |    1.15 |   1.500
zwegneravx         |    0.465 |      0.048 |  3.510 |  41.029 |    1.96 |   5.434
lookup2            |    0.432 |      0.057 |  3.506 |  30.675 |    2.18 |   3.776
lookup3            |    0.403 |      0.048 |  3.507 |  33.500 |    1.54 |   3.846
lookup4            |    0.400 |      0.052 |  3.508 |  37.000 |    2.05 |   4.220
basic              |    0.586 |      0.062 |  3.500 |  22.538 |    2.22 |   3.772
range              |    0.546 |      0.081 |  3.301 |  22.668 |    2.03 |   3.750
```

## Reference

- John Keiser, Daniel Lemire, [Validating UTF-8 In Less Than One Instruction Per Byte](https://arxiv.org/abs/2010.03090), Software: Practice and Experience 51 (5), 2021

## Citation

If you use this repository, please cite it as follows:

```bibtex
@misc{lemire2021validateutf8experiments,
  author = {Lemire, Daniel},
  title = {validateutf8-experiments: Fast {UTF-8} Validation Benchmarks},
  year = {2021},
  url = {https://github.com/lemire/validateutf8-experiments},
}
```

## Credit

A lot of the hard work is due to Keiser. Some of the code is based on code by Muła. The first SIMD UTF-8 validator was based on work by Willets. Some of our improvments were motivated by work by Zwegner who produced some of the finest SIMD-based UTF-8 validators.
