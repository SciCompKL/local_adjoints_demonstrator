#include <iostream>
#include <string>

#include "benchmark.hpp"
#include "evaluation_strategies.hpp"
#include "local_adjoints.hpp"
#include "preaccumulations.hpp"
#include "tape.hpp"

template<typename Identifier, typename Gradient, EvaluationStrategy::Strategy strategy>
void testEvaluation(std::string const& name, Tape<Identifier, Gradient>& tape, Gradient const& seed) {
  std::cout << std::setw(60) << name << std::setw(10)
            << EvaluationStrategy::evaluate<Identifier, Gradient, strategy>(tape, seed) << std::endl;
}

template<typename Identifier, typename Gradient, EvaluationStrategy::Strategy strategy>
void testPreacc(std::string const& name, Preaccumulations<Identifier, Gradient>& preaccs, Gradient const& seed) {
  std::cout << std::setw(60) << name << std::setw(10) << preaccs.template run<strategy>(1.0) << std::endl;
}

template<typename Identifier, typename Gradient, EvaluationStrategy::Strategy strategy>
void testBenchmark(std::string const& name, Benchmark<Identifier, Gradient> benchmark,
                   Preaccumulations<Identifier, Gradient>& preaccs) {
  auto result = benchmark.template run<strategy>(preaccs);
  std::cout << std::setw(60) << name << result << std::endl;
}

/// Simple tests for the local adjoints demonstrator code.
int main(int argc, char** argv) {
  using Identifier = int;
  using Gradient = double;

  size_t const size = 10;
  int iMin = 20;
  int iMax = 80;
  double seed = 1.0;

  size_t randomSeed = 42;
  if (argc > 1) {
    randomSeed = std::stol(argv[1]);
  }

  auto tape = Tape<Identifier, Gradient>::generate(size, iMin, iMax, randomSeed);

  std::cout << "Example tape." << std::endl;
  tape->print();
  Gradient J = 1.0;
  for (auto const& jacobian : tape->jacobians) {
    J *= jacobian;
  }
  std::cout << "Evaluation should yield " << J << "." << std::endl;
  std::cout << std::endl;

  std::cout << "Evaluations with all adjoint variants." << std::endl;

  using EvaluationStrategy::Strategy;

  testEvaluation<Identifier, Gradient, Strategy::TEMPORARY_MAP>("temporary map, std::map", *tape, seed);
  testEvaluation<Identifier, Gradient, Strategy::TEMPORARY_UNORDERED_MAP>("temporary map, std::unordered_map", *tape,
                                                                        seed);
  testEvaluation<Identifier, Gradient, Strategy::TEMPORARY_VECTOR>("temporary vector", *tape, seed);
  testEvaluation<Identifier, Gradient, Strategy::PERSISTENT_VECTOR>("persistent vector", *tape, seed);
  testEvaluation<Identifier, Gradient, Strategy::PERSISTENT_VECTOR_OFFSET>("persistent vector with offset", *tape, seed);

  Tape<Identifier, Gradient> localTapeCopy = *tape;
  testEvaluation<Identifier, Gradient, Strategy::TEMPORARY_MAP_EDITING>("editing with std::map, temporary vector",
                                                                        localTapeCopy, seed);

  localTapeCopy = *tape;
  testEvaluation<Identifier, Gradient, Strategy::TEMPORARY_UNORDERED_MAP_EDITING>(
      "editing with std::unordered_map, temporary vector", localTapeCopy, seed);

  std::cout << std::endl;

  std::cout << "Tape after identifier remapping." << std::endl;
  tape->remapIdentifiers<std::map<Identifier, Identifier>>();
  tape->print();
  std::cout << std::endl;

  std::cout << "Simultaneous preaccumulations." << std::endl;

  size_t const nPreaccs = 10000;
  size_t const preaccSizeMin = 8000;
  size_t const preaccSizeMax = 12000;
  size_t const nEvalMin = 1;
  size_t const nEvalMax = 10;
  iMin = 1;
  iMax = 1000;

  Preaccumulations<Identifier, Gradient> preaccs(nPreaccs, preaccSizeMin, preaccSizeMax, nEvalMin, nEvalMax, iMin, iMax,
                                                 randomSeed);

  testPreacc<Identifier, Gradient, Strategy::TEMPORARY_MAP>("temporary map, std::map", preaccs, seed);
  testPreacc<Identifier, Gradient, Strategy::TEMPORARY_UNORDERED_MAP>("temporary map, std::unordered_map", preaccs,
                                                                          seed);
  testPreacc<Identifier, Gradient, Strategy::TEMPORARY_VECTOR>("temporary vector", preaccs, seed);
  testPreacc<Identifier, Gradient, Strategy::PERSISTENT_VECTOR>("persistent vector", preaccs, seed);
  testPreacc<Identifier, Gradient, Strategy::PERSISTENT_VECTOR_OFFSET>("persistent vector with offset", preaccs, seed);
  testPreacc<Identifier, Gradient, Strategy::TEMPORARY_MAP_EDITING>("editing with std::map, temporary vector",
                                                                    preaccs, seed);
  testPreacc<Identifier, Gradient, Strategy::TEMPORARY_UNORDERED_MAP_EDITING>(
      "editing with std::unordered_map, temporary vector", preaccs, seed);

  std::cout << std::endl;

  std::cout << "Benchmarking simultaneous preaccumulations." << std::endl;

  size_t const nWarmups = 1;
  size_t const nRuns = 3;

  Benchmark<Identifier, Gradient> benchmark(nWarmups, nRuns);

  /// note that memory high water marks are not representative as all tests run in the same executable
  testBenchmark<Identifier, Gradient, Strategy::TEMPORARY_MAP>("temporary map, std::map", benchmark, preaccs);
  testBenchmark<Identifier, Gradient, Strategy::TEMPORARY_UNORDERED_MAP>("temporary map, std::unordered_map", benchmark,
                                                                         preaccs);
  testBenchmark<Identifier, Gradient, Strategy::TEMPORARY_VECTOR>("temporary vector", benchmark, preaccs);
  testBenchmark<Identifier, Gradient, Strategy::PERSISTENT_VECTOR>("persistent vector", benchmark, preaccs);
  testBenchmark<Identifier, Gradient, Strategy::PERSISTENT_VECTOR_OFFSET>("persistent vector with offset", benchmark,
                                                                          preaccs);
  testBenchmark<Identifier, Gradient, Strategy::TEMPORARY_MAP_EDITING>("editing with std::map, temporary vector",
                                                                       benchmark, preaccs);
  testBenchmark<Identifier, Gradient, Strategy::TEMPORARY_UNORDERED_MAP_EDITING>(
      "editing with std::unordered_map, temporary vector", benchmark, preaccs);

  return 0;
}
