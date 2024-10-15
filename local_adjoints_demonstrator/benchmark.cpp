#include "benchmark.hpp"

/// Benchmarking executable.
/// Mandatory arguments: nPreaccs preaccSizeMin preaccSizeMax nEvalMin nEvalMax iMin iMax nWarmups nRuns strategy
/// Optional arguments: randomSeed
/// Strategies are numbered starting with zero in the order as in EvaluationStrategy::Strategy.
int main(int argc, char** argv) {
  using EvaluationStrategy::Strategy;
  using Identifier = int;
  using Gradient = double;

  if (argc < 11) {
    std::cout << "Usage: ./benchmark nPreaccs preaccSizeMin preaccSizeMax nEvalMin nEvalMax iMin iMax nWarmups nRuns"
              << " strategy [randomSeed] " << std::endl << std::endl;
    std::cout << "nPreaccs: number of preaccumulations" << std::endl;
    std::cout << "preaccSizeMin: minimum size of preaccumulations" << std::endl;
    std::cout << "preaccSizeMax: maximum size of preaccumulations" << std::endl;
    std::cout << "nEvalMin: minimum number of evaluations per preaccumulation" << std::endl;
    std::cout << "nEvalMax: maximum number of evaluations per preaccumulation" << std::endl;
    std::cout << "iMin: minimum identifier" << std::endl;
    std::cout << "iMax: maximum identifier" << std::endl;
    std::cout << "nWarmups: number of discarded warmup runs" << std::endl;
    std::cout << "nRuns: number of benchmark runs" << std::endl;
    std::cout << "strategy:" << std::endl;
    std::cout << "  0: temporary vector" << std::endl;
    std::cout << "  1: persistent vector" << std::endl;
    std::cout << "  2: persistent vector with offset" << std::endl;
    std::cout << "  3: temporary map, std::map" << std::endl;
    std::cout << "  4: temporary map, std::unordered_map" << std::endl;
    std::cout << "  5: editing with std::map, temporary vector" << std::endl;
    std::cout << "  6: editing with std::unordered_map, temporary vector" << std::endl;
    std::cout << "randomSeed: specify a random seed, defaults to 42, generated workload is deterministic w.r.t. this seed"
              << std::endl << std::endl;
    std::cout << "Output: [strategy] [number of threads] [nWarmups] [nRuns] [average time] [minimum time] "
                 "[maximum time] [memory hwm] [checksum]" << std::endl << std::endl;
    std::cout << "Set number of threads by setting OMP_NUM_THREADS." << std::endl;
    return 1;
  }

  size_t nPreaccs = std::stol(argv[1]);
  size_t preaccSizeMin = std::stol(argv[2]);
  size_t preaccSizeMax = std::stol(argv[3]);
  size_t nEvalMin = std::stol(argv[4]);
  size_t nEvalMax = std::stol(argv[5]);
  size_t iMin = std::stol(argv[6]);
  size_t iMax = std::stol(argv[7]);
  size_t nWarmups = std::stol(argv[8]);
  size_t nRuns = std::stol(argv[9]);
  size_t strategy = std::stoi(argv[10]);

  size_t randomSeed = 42;
  if (argc == 12) {
    randomSeed = std::stol(argv[11]);
  }

  Preaccumulations<Identifier, Gradient> preaccs(nPreaccs, preaccSizeMin, preaccSizeMax, nEvalMin, nEvalMax, iMin, iMax,
                                                 randomSeed);

  Benchmark<Identifier, Gradient> benchmark(nWarmups, nRuns);

  std::cout << std::setw(5) << strategy;
  switch (strategy) {
    case 0:
      std::cout << benchmark.run<Strategy::TEMPORARY_VECTOR>(preaccs) << std::endl;
      break;
    case 1:
      std::cout << benchmark.run<Strategy::PERSISTENT_VECTOR>(preaccs) << std::endl;
      break;
    case 2:
      std::cout << benchmark.run<Strategy::PERSISTENT_VECTOR_OFFSET>(preaccs) << std::endl;
      break;
    case 3:
      std::cout << benchmark.run<Strategy::TEMPORARY_MAP>(preaccs) << std::endl;
      break;
    case 4:
      std::cout << benchmark.run<Strategy::TEMPORARY_UNORDERED_MAP>(preaccs) << std::endl;
      break;
    case 5:
      std::cout << benchmark.run<Strategy::TEMPORARY_MAP_EDITING>(preaccs) << std::endl;
      break;
    case 6:
      std::cout << benchmark.run<Strategy::TEMPORARY_UNORDERED_MAP_EDITING>(preaccs) << std::endl;
      break;
  }

  return 0;
}
