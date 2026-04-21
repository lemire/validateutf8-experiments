# validateutf8-experiments

This project contains benchmarks regarding fast UTF-8 validation. It is for research purposes only: not for production use. If you are not doing research, this repository is not for you!

The algorithm we designed is called lookup. We experimented with several variants (lookup2, lookup3, lookup4) that have similar performance. The lookup approach is one of the fastest ways to validate UTF-8 strings. Please see the file `src/generate/utf8_lookup4_algorithm.h` for details.

The algorithm of this repository has been included in production-ready libraries:

- [simdutf](https://github.com/simdutf/simdutf) C++ library is part of important systems such as Bun, Node.js, WebKit/Safari, etc.
- [SimdUnicode](https://github.com/simdutf/SimdUnicode) is a C# port of the validation algorithm, adapted for the .NET runtime.


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
