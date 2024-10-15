## Local Adjoints Demonstrator

This demonstrator code supports the paper [Local Adjoints for Simultaneous Preaccumulations with Shared Inputs](https://arxiv.org/abs/2405.07819). It can be used to quickly explore and assess different storage strategies for local adjoint variables.

- According to user-provided parameters, generate synthetic preaccumulation-like workloads.
- Execute the workloads concurrently, with options to use different strategies for local adjoint variables.
- Benchmark the memory and runtime performance.

While the code neither implements nor uses actual automatic differentiation, the synthetically generated preaccumulation workloads resemble what a Jacobian taping approach would record and evaluate for single-input single-output chains of unary operations. A single work item consists of one such chain and the number of times it should be evaluated, emulating preaccumulations with multiple inputs and outputs. Parallel execution corresponds to distributing these work items to threads in an OpenMP worksharing loop.

## Build 

The code is intended to be used on Linux systems (memory is measured via `/proc/self/status`). In the `local_adjoints_demonstrator` subfolder, simply run

```
make
```

to build both a `test` and a `benchmark` executable. You can run `./test` to get an indication that everything works as intended.

## Run

The `benchmark` executable takes the following mandatory positional arguments.

| argument | meaning |
|-----------|---------|
| 1 | number of synthetic chains to generate |
| 2 | minimum length of the chain |
| 3 | maximum length of the chain |
| 4 | minimum number of evaluations per chain |
| 5 | maximum number of evaluations per chain |
| 6 | minimum identifier (virtual address) |
| 7 | maximum identifier (virtual address) |
| 8 | number of warmup runs |
| 9 | number of measured benchmark runs |
| 10 | integer that identifies the strategy for local adjoint variables |

Actual preaccumulation sizes, numbers of evaluations, and identifiers are drawn from a uniform distribution on integers between the respective lower and upper bounds. Each run performs the entire preaccumulation workload, but only benchmark runs contribute to the measurements.

The following strategies are implemented. Please refer to the paper for detailed explanations.

| integer | strategy |
|---------|----------|
| 0 | temporary vector |
| 1 | persistent vector |
| 2 | persistent vector with offset |
| 3 | temporary map, std::map |
| 4 | temporary map, std::unordered_map |
| 5 | editing with std::map, temporary vector |
| 6 | editing with std::unordered_map, temporary vector |

The `benchmark` executable takes an integer as an optional 11th argument that serves as a random seed. The generated workload is deterministic with respect to the random seed.

The following example produces 100000 chains, with lengths between 800 and 1200, 1 to 5 evaluations, with identifiers between 2 and 200000, executes 1 warmup run and 3 benchmark runs, and uses strategy 4 for local adjoints. The random seed is 12345.

```
./benchmark 100000 800 1200 1 5 2 200000 1 3 4 12345
```

The output looks for example as follows

```
    4   32    1    3        0.747064        0.745677        0.748683            3.75     1.22313e+06
```

and indicates, in this order, the strategy, the number of threads, the number of warmup runs, the number of benchmark runs, average runtime, minimum runtime, maximum runtime, all in seconds, memory high water mark in MB, and a checksum to verify the determinism. The checksum is independent of the strategy for local adjoints and scales linearly with the number of runs (including both warmup and benchmark runs).

The number of threads can be changed by setting `OMP_NUM_THREADS`, e.g., `export OMP_NUM_THREADS=12`, prior to the benchmark run.


