#include <iostream>

#include "local_adjoints.hpp"
#include "tape.hpp"

/// Simple tests for the local adjoints demonstrator code.
int main(int argc, char** argv) {
  using Identifier = int;
  using Gradient = double;

  int const size = 10;
  int const iMin = 20;
  int const iMax = 80;
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
  {
    LocalAdjoints::TemporaryMapStdMap<Identifier, Gradient> adjoints;
    std::cout << std::setw(60) << "temporary map, std::map"
              << std::setw(10) << tape->evaluate(adjoints, seed) << std::endl;
  }

  {
    LocalAdjoints::TemporaryMapStdUnorderedMap<Identifier, Gradient> adjoints;
    std::cout << std::setw(60) << "temporary map, std::unordered_map"
              << std::setw(10) << tape->evaluate(adjoints, seed) << std::endl;
  }

  {
    LocalAdjoints::TemporaryVector<Identifier, Gradient> adjoints;
    adjoints.resize(iMax + 1);
    std::cout << std::setw(60) << "temporary vector"
              << std::setw(10) << tape->evaluate(adjoints, seed) << std::endl;
  }

  {
    LocalAdjoints::PersistentVector<Identifier, Gradient> adjoints;
    adjoints.resize(iMax + 1);
    std::cout << std::setw(60) << "persistent vector"
              << std::setw(10) << tape->evaluate(adjoints, seed) << std::endl;
  }

  {
    LocalAdjoints::PersistentVectorOffset<Identifier, Gradient> adjoints(iMin);
    adjoints.resize(iMax - iMin + 1);
    std::cout << std::setw(60) << "persistent vector with offset"
              << std::setw(10) << tape->evaluate(adjoints, seed) << std::endl;
  }

  {
    Tape<Identifier, Gradient> localTapeCopy(*tape);
    localTapeCopy.remapIdentifiers<std::map<Identifier, Identifier>>();
    LocalAdjoints::TemporaryVector<Identifier, Gradient> adjoints;
    adjoints.resize(localTapeCopy.getMaxIdentifier() + 1);
    std::cout << std::setw(60) << "editing with std::map, temporary vector"
              << std::setw(10) << localTapeCopy.evaluate(adjoints, seed) << std::endl;
  }

  {
    Tape<Identifier, Gradient> localTapeCopy(*tape);
    localTapeCopy.remapIdentifiers<std::unordered_map<Identifier, Identifier>>();
    LocalAdjoints::TemporaryVector<Identifier, Gradient> adjoints;
    adjoints.resize(localTapeCopy.getMaxIdentifier() + 1);
    std::cout << std::setw(60) << "editing with std::unordered_map, temporary vector"
              << std::setw(10) << localTapeCopy.evaluate(adjoints, seed) << std::endl;
  }

  std::cout << std::endl;

  std::cout << "Tape after identifier remapping." << std::endl;
  tape->remapIdentifiers<std::map<Identifier, Identifier>>();
  tape->print();
  std::cout << std::endl;

  return 0;
}
