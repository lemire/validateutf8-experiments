# validateutf8-experiments

This project contains benchmarks regarding fast UTF-8 validation. It is for research purposes only: not for production use. If you are not doing research, this repository is not for you!

## Software requirements

An x64 system with Linux, docker, bash and git.

(It is possible to run these experiments using macOS or Windows as the host, but there might unnecessary be virtualization overhead that might affect the results slightly.)

## Hardware requirements


Though the code is generic, the benchmarks assume that you have a processor with AVX2 support (there is no runtime check). Thus please ensure that you have a recent Intel or AMD processor with AVX2 support. We recommend AMD Zen 2 or better, or Intel Skylake and better.

Under Linux, you might be able to know about your CPU with the following command:

```
lscpu | grep Model
```

- We recommend that you have bare-metal access to the hardware. Cloud nodes are often noisier.
- We recommend against testing on a laptop.
- It is best if you can set the governor (or the equivalent) to `Performance`.
- Some people prefer to disable hyper-threading and to lock the process on a given processor: if you are using a quiet machine, we believe that it is unnecessary with this benchmark.


## Reproducible experiments

To ensure that experiments are reproducible, we rely on a docker image. We recommend that you install docker under Linux. 





Starting in a bash shell do:

```
git clone https://github.com/lemire/validateutf8-experiments.git
cd validateutf8-experiments
./run bash
make
./unit
./benchmark
./benchstream 1000
```


## Want a production-ready function?

The validateutf8-experiments repository is for research purposes only.

If you want access to a fast validation function for production use, you can rely on the simdjson library. [It is as simple as the following](https://github.com/simdjson/simdjson/blob/master/doc/basics.md#utf-8-validation-alone):

```C++
  const char * some_string = "[ 1, 2, 3, 4] ";
  size_t length = strlen(some_string);
  bool is_ok = simdjson::validate_utf8(some_string, length);
```

See https://github.com/simdjson/


The simdjson library supports a wide-range of platforms and offers runtime dispatching as well as the most up-to-date algorithms. It is not necessary that your data is made of JSON though this was the original motivation.

## Example

```
$ git clone https://github.com/lemire/validateutf8-experiments.git
$ cd validateutf8-experiments
$ make
$ ./unit
tests ok.
$ ./benchmark
$ ./benchmark

Running UTF8 validation benchmark.
The speed is normalized by the number of input bytes.
file: examples/hongkong.html (1807KB)
Overhead ==> nanoseconds : 20.000, instructions 0.000
                                  memcpy                                  48.281 GB/s  ( 1.94 %)
                                  fushia                                   1.323 GB/s  (11.26 %)
                            fushia_ascii                                   3.660 GB/s  ( 0.88 %)
                           fushia_ascii2                                   4.334 GB/s  ( 1.30 %)
                           fushia_ascii4                                   4.210 GB/s  ( 0.94 %)
                                 utf8lib                                   0.377 GB/s  ( 0.19 %)
                                     dfa                                   0.664 GB/s  ( 0.96 %)
                                    dfa2                                   1.130 GB/s  ( 0.53 %)
                                    dfa3                                   1.968 GB/s  ( 7.13 %)
                                    dfa4                                   0.678 GB/s  ( 0.42 %)
                              zwegneravx                                  14.039 GB/s  ( 1.05 %)
                              lookup2avx                                  17.426 GB/s  ( 2.40 %)
                              lookup3avx                                  17.672 GB/s  ( 2.86 %)
                              lookup4avx                                  18.185 GB/s  ( 2.48 %)
                                basicavx                                  13.374 GB/s  ( 2.13 %)
                                rangeavx                                  14.758 GB/s  ( 3.07 %)
file: examples/twitter.json (631KB)
Overhead ==> nanoseconds : 20.000, instructions 0.000
                                  memcpy                                  41.992 GB/s  ( 2.83 %)
                                  fushia                                   1.537 GB/s  (14.57 %)
                            fushia_ascii                                   3.437 GB/s  ( 1.10 %)
                           fushia_ascii2                                   4.414 GB/s  ( 1.09 %)
                           fushia_ascii4                                   4.077 GB/s  ( 1.33 %)
                                 utf8lib                                   0.394 GB/s  ( 0.92 %)
                                     dfa                                   0.667 GB/s  ( 2.14 %)
                                    dfa2                                   1.131 GB/s  ( 0.48 %)
                                    dfa3                                   1.970 GB/s  ( 1.67 %)
                                    dfa4                                   0.679 GB/s  ( 0.65 %)
                              zwegneravx                                  24.299 GB/s  ( 1.24 %)
                              lookup2avx                                  27.537 GB/s  ( 4.99 %)
                              lookup3avx                                  29.649 GB/s  ( 3.38 %)
                              lookup4avx                                  30.436 GB/s  ( 7.83 %)
                                basicavx                                  18.816 GB/s  ( 1.29 %)
                                rangeavx                                  22.463 GB/s  ( 6.10 %)

Running UTF8 validation benchmark.
The speed is normalized by the number of input bytes.
Input size: (UTF8) 16384
- ASCII characters
Overhead ==> nanoseconds : 20.000, instructions 0.000
                                  memcpy                                  74.477 GB/s  (10.12 %)
                                  fushia                                   1.697 GB/s  ( 0.91 %)
                            fushia_ascii                                   9.038 GB/s  ( 1.15 %)
                           fushia_ascii2                                  17.986 GB/s  ( 2.22 %)
                           fushia_ascii4                                  35.620 GB/s  ( 1.98 %)
                                 utf8lib                                   0.424 GB/s  ( 0.54 %)
                                     dfa                                   0.653 GB/s  ( 0.30 %)
                                    dfa2                                   1.132 GB/s  ( 0.39 %)
                                    dfa3                                   1.961 GB/s  ( 1.85 %)
                                    dfa4                                   0.680 GB/s  ( 1.95 %)
                              zwegneravx                                  52.855 GB/s  ( 6.15 %)
                              lookup2avx                                  71.239 GB/s  ( 6.48 %)
                              lookup3avx                                  71.239 GB/s  ( 6.74 %)
                              lookup4avx                                  71.239 GB/s  ( 6.28 %)
                                basicavx                                  71.239 GB/s  ( 6.30 %)
                                rangeavx                                  68.271 GB/s  ( 2.70 %)
- 1 or 2 UTF8 bytes
Overhead ==> nanoseconds : 20.000, instructions 0.000
                                  memcpy                                  74.482 GB/s  ( 9.76 %)
                                  fushia                                   0.416 GB/s  ( 3.77 %)
                            fushia_ascii                                   0.362 GB/s  ( 2.14 %)
                           fushia_ascii2                                   0.390 GB/s  ( 3.67 %)
                           fushia_ascii4                                   0.364 GB/s  ( 2.91 %)
                                 utf8lib                                   0.230 GB/s  ( 1.01 %)
                                     dfa                                   0.653 GB/s  ( 0.31 %)
                                    dfa2                                   1.132 GB/s  ( 0.99 %)
                                    dfa3                                   1.975 GB/s  ( 0.95 %)
                                    dfa4                                   0.680 GB/s  ( 0.24 %)
                              zwegneravx                                   9.401 GB/s  ( 1.47 %)
                              lookup2avx                                  11.857 GB/s  ( 1.46 %)
                              lookup3avx                                  12.395 GB/s  ( 1.83 %)
                              lookup4avx                                  13.747 GB/s  ( 1.66 %)
                                basicavx                                   6.441 GB/s  ( 1.31 %)
                                rangeavx                                   7.751 GB/s  ( 1.49 %)
- 1, 2, 3 UTF8 bytes
Overhead ==> nanoseconds : 20.000, instructions 0.000
                                  memcpy                                  74.145 GB/s  ( 8.25 %)
                                  fushia                                   0.392 GB/s  ( 1.53 %)
                            fushia_ascii                                   0.372 GB/s  ( 2.83 %)
                           fushia_ascii2                                   0.363 GB/s  ( 3.29 %)
                           fushia_ascii4                                   0.364 GB/s  ( 3.22 %)
                                 utf8lib                                   0.258 GB/s  ( 0.76 %)
                                     dfa                                   0.657 GB/s  ( 0.94 %)
                                    dfa2                                   1.131 GB/s  ( 1.19 %)
                                    dfa3                                   1.973 GB/s  ( 1.29 %)
                                    dfa4                                   0.679 GB/s  ( 0.30 %)
                              zwegneravx                                   9.401 GB/s  ( 1.28 %)
                              lookup2avx                                  11.857 GB/s  ( 1.85 %)
                              lookup3avx                                  12.489 GB/s  ( 2.57 %)
                              lookup4avx                                  13.632 GB/s  ( 1.65 %)
                                basicavx                                   6.441 GB/s  ( 1.20 %)
                                rangeavx                                   7.792 GB/s  ( 1.99 %)
- 1, 2, 3, 4 UTF8 bytes
Overhead ==> nanoseconds : 20.000, instructions 0.000
                                  memcpy                                  74.486 GB/s  (10.24 %)
                                  fushia                                   0.580 GB/s  ( 3.17 %)
                            fushia_ascii                                   0.608 GB/s  ( 6.12 %)
                           fushia_ascii2                                   0.537 GB/s  ( 5.54 %)
                           fushia_ascii4                                   0.490 GB/s  ( 3.89 %)
                                 utf8lib                                   0.286 GB/s  ( 1.89 %)
                                     dfa                                   0.653 GB/s  ( 0.23 %)
                                    dfa2                                   1.131 GB/s  ( 0.41 %)
                                    dfa3                                   1.971 GB/s  ( 1.19 %)
                                    dfa4                                   0.680 GB/s  ( 0.32 %)
                              zwegneravx                                   9.402 GB/s  ( 1.05 %)
                              lookup2avx                                  11.857 GB/s  ( 0.92 %)
                              lookup3avx                                  12.396 GB/s  ( 0.98 %)
                              lookup4avx                                  13.736 GB/s  ( 1.94 %)
                                basicavx                                   6.441 GB/s  ( 1.11 %)
                                rangeavx                                   7.792 GB/s  ( 1.73 %)
```





## Credit

A lot of the hard work is due to Keiser. Some of the code is based on code by Muła. The first SIMD UTF-8 validator was based on work by Willets. Some of our improvments were motivated by work by Zwegner who produced some of the finest SIMD-based UTF-8 validators.
