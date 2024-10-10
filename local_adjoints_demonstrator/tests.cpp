#include <iostream>

#include "evaluation_strategies.hpp"
#include "local_adjoints.hpp"
#include "preaccumulations.hpp"
#include "tape.hpp"

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

  std::cout << std::setw(60) << "temporary map, std::map"
            << std::setw(10)
            << EvaluationStrategy::evaluate<Identifier, Gradient, Strategy::TEMPORARY_MAP>(*tape, seed)
            << std::endl;

  std::cout << std::setw(60) << "temporary map, std::unordered_map"
            << std::setw(10)
            << EvaluationStrategy::evaluate<Identifier, Gradient, Strategy::TEMPORARY_UNORDERED_MAP>(*tape, seed)
            << std::endl;

  std::cout << std::setw(60) << "temporary vector"
            << std::setw(10)
            << EvaluationStrategy::evaluate<Identifier, Gradient, Strategy::TEMPORARY_VECTOR>(*tape, seed)
            << std::endl;

  std::cout << std::setw(60) << "persistent vector"
            << std::setw(10)
            << EvaluationStrategy::evaluate<Identifier, Gradient, Strategy::PERSISTENT_VECTOR>(*tape, seed)
            << std::endl;

  std::cout << std::setw(60) << "persistent vector with offset"
            << std::setw(10)
            << EvaluationStrategy::evaluate<Identifier, Gradient, Strategy::PERSISTENT_VECTOR_OFFSET>(*tape, seed)
            << std::endl;

  {
    Tape<Identifier, Gradient> localTapeCopy(*tape);
    std::cout << std::setw(60) << "editing with std::map, temporary vector"
              << std::setw(10)
              << EvaluationStrategy::evaluate<Identifier, Gradient, Strategy::TEMPORARY_MAP_EDITING>(localTapeCopy,
                                                                                                     seed)
              << std::endl;
  }

  {
    Tape<Identifier, Gradient> localTapeCopy(*tape);
    std::cout << std::setw(60) << "editing with std::unordered_map, temporary vector"
              << std::setw(10)
              << EvaluationStrategy::evaluate<Identifier, Gradient, Strategy::TEMPORARY_UNORDERED_MAP_EDITING>(
                     localTapeCopy, seed)
              << std::endl;
  }

  std::cout << std::endl;

  std::cout << "Tape after identifier remapping." << std::endl;
  tape->remapIdentifiers<std::map<Identifier, Identifier>>();
  tape->print();
  std::cout << std::endl;

  std::cout << "Simultaneous preaccumulations." << std::endl;

  size_t const nPreaccs = 10000;
  size_t const preaccSizeMin = 80;
  size_t const preaccSizeMax = 120;
  size_t const nEvalMin = 1;
  size_t const nEvalMax = 10;
  iMin = 1;
  iMax = 1000;

  Preaccumulations<Identifier, Gradient> preaccs(nPreaccs, preaccSizeMin, preaccSizeMax, nEvalMin, nEvalMax, iMin, iMax,
                                                 randomSeed);
  std::cout << std::setw(10) << preaccs.run<Strategy::PERSISTENT_VECTOR>(1.0) << std::endl << std::endl;

  return 0;
}
