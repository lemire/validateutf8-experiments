# validateutf8-experiments

This project contains benchmarks regarding fast UTF-8 validation. 




## Requirements

- Linux/macOS with make and a recent C++ compiler.
- A PC with a recent x64 processor.

## Usage

```
$ git clone https://github.com/lemire/validateutf8-experiments.git
$ cd validateutf8-experiments
$ make
$ ./unit
tests ok.
$ ./benchmark

Running UTF8 validation benchmark.
The speed is normalized by the number of input bytes.
file: examples/hongkong.html (1807KB)
                                  fushia                                   6.641 ins/byte,    0.759 % branch miss/byte,      3.377 GHz,    1.252 GB/s
                                     dfa                                   9.000 ins/byte,    0.000 % branch miss/byte,      3.390 GHz,    0.678 GB/s
                              lookup2avx                                   0.421 ins/byte,    0.240 % branch miss/byte,      3.389 GHz,   15.874 GB/s
                              lookup3avx                                   0.425 ins/byte,    0.229 % branch miss/byte,      3.389 GHz,   16.302 GB/s
file: examples/twitter.json (631KB)
                                  fushia                                   7.158 ins/byte,    0.391 % branch miss/byte,      3.339 GHz,    1.184 GB/s
                                     dfa                                   9.000 ins/byte,    0.001 % branch miss/byte,      3.388 GHz,    0.677 GB/s
                              lookup2avx                                   0.408 ins/byte,    0.042 % branch miss/byte,      3.373 GHz,   26.732 GB/s
                              lookup3avx                                   0.413 ins/byte,    0.027 % branch miss/byte,      3.371 GHz,   28.848 GB/s

Running UTF8 validation benchmark.
The speed is normalized by the number of input bytes.
Input size: (UTF8) 16384
- ASCII characters
                                  fushia                                   6.005 ins/byte,    0.031 % branch miss/byte,      3.359 GHz,    1.667 GB/s
                                     dfa                                   9.005 ins/byte,    0.031 % branch miss/byte,      3.379 GHz,    0.674 GB/s
                              lookup2avx                                   0.196 ins/byte,    0.031 % branch miss/byte,      2.556 GHz,   39.963 GB/s
                              lookup3avx                                   0.212 ins/byte,    0.031 % branch miss/byte,      2.565 GHz,   40.962 GB/s
- 1 or 2 UTF8 bytes
                                  fushia                                  10.674 ins/byte,   24.126 % branch miss/byte,      3.385 GHz,    0.369 GB/s
                                     dfa                                   9.005 ins/byte,    0.031 % branch miss/byte,      3.379 GHz,    0.674 GB/s
                              lookup2avx                                   1.149 ins/byte,    0.031 % branch miss/byte,      3.165 GHz,   10.416 GB/s
                              lookup3avx                                   1.118 ins/byte,    0.031 % branch miss/byte,      3.156 GHz,   11.056 GB/s
- 1, 2, 3 UTF8 bytes
                                  fushia                                  12.212 ins/byte,   24.832 % branch miss/byte,      3.386 GHz,    0.364 GB/s
                                     dfa                                   9.005 ins/byte,    0.031 % branch miss/byte,      3.379 GHz,    0.674 GB/s
                              lookup2avx                                   1.153 ins/byte,    0.031 % branch miss/byte,      3.163 GHz,   10.417 GB/s
                              lookup3avx                                   1.121 ins/byte,    0.031 % branch miss/byte,      3.167 GHz,   11.057 GB/s
- 1, 2, 3, 4 UTF8 bytes
                                  fushia                                  11.996 ins/byte,   16.061 % branch miss/byte,      3.382 GHz,    0.509 GB/s
                                     dfa                                   9.005 ins/byte,    0.031 % branch miss/byte,      3.378 GHz,    0.674 GB/s
                              lookup2avx                                   1.153 ins/byte,    0.031 % branch miss/byte,      3.161 GHz,   10.418 GB/s
                              lookup3avx                                   1.121 ins/byte,    0.031 % branch miss/byte,      3.154 GHz,   10.984 GB/s

```





## Credit

A lot of the hard work is due to John Keiser. Some of the code is based on code by Wojciech Muła.
