#pragma once

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

/** @brief Simplified tape.
 *
 *  The implementation resembles a Jacobian tape of a computation with a single input, a single output, and only unary
 *  operations.
 *
 *  input -> o -> o -> o -> ... -> o -> o -> o -> output
 */
template<typename Identifier, typename Gradient>
struct Tape {
  public:
    std::vector<Identifier> identifiers;  /// virtual memory addresses to access
    std::vector<Gradient> jacobians;      /// partials to multiply

    /// Performs the tape evaluation on the given adjoint variables with the given seed.
    /// Reads and writes each adjoint memory location exactly once.
    /// Auto-zeroes adjoint variables.
    template<typename Adjoints>
    Gradient evaluate(Adjoints& adjoints, Gradient seed) {
      adjoints[identifiers[0]] = seed * jacobians[0];
      for (size_t i = 1; i < identifiers.size(); ++i) {
        auto identifier = identifiers[i];
        auto predecessor = identifiers[i - 1];

        Gradient temp = adjoints[predecessor];  // account for the case identifier == predecessor
        adjoints[predecessor] = 0.0;
        adjoints[identifier] = temp * jacobians[i];
      }
      Gradient result = adjoints[identifiers.back()];
      adjoints[identifiers.back()] = 0.0;
      return result;
    }

    /// Edit the tape and remap identifiers to a contiguous range.
    template<typename Map>
    void remapIdentifiers() {
      Identifier nextIdentifier = 1;
      Map identifierMap;

      for (auto& identifier : identifiers) {
        auto result = identifierMap.insert({identifier, nextIdentifier});
        if (result.second) {  // insertion was performed, increment identifier
          nextIdentifier++;
        }
        identifier = result.first->second;
      }
    }

    Identifier getMaxIdentifier() {
      Identifier currentMax = 0;
      for (auto const& identifier : identifiers) {
        currentMax = std::max(currentMax, identifier);
      }
      return currentMax;
    }

    /// Generate a tape of a given size, drawing random identifiers uniformly from the specified range.
    /// Produces Jacobians in a neighborhood of 1.0.
    /// Deterministic with respect to the specified seed.
    static std::shared_ptr<Tape> generate(size_t size, Identifier iMin, Identifier iMax, size_t randomSeed) {
      std::shared_ptr<Tape> result(new Tape);

      std::mt19937 generator(randomSeed);
      std::uniform_int_distribution<Identifier> distribution(iMin, iMax);

      result->identifiers.resize(size);
      result->jacobians.resize(size);

      for (size_t i = 0; i < size; ++i) {
        result->identifiers[i] = distribution(generator);
        result->jacobians[i] = 1.0 + 0.1 * std::sin(result->identifiers[i]);
      }

      return result;
    }

    /// Tape printing for debugging purposes.
    void print() {
      for (size_t i = 0; i < identifiers.size(); ++i) {
        std::cout << std::setw(10) << identifiers[i] << " " << jacobians[i] << std::endl;
      }
    }
};



