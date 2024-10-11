#pragma once

#include "local_adjoints.hpp"
#include "tape.hpp"

/// Defines and implements the different tape evaluation strategies for preaccumulation.
namespace EvaluationStrategy {

  /// Evaluation strategies for preaccumulations.
  enum Strategy {
    TEMPORARY_VECTOR = 0,
    PERSISTENT_VECTOR = 1,
    PERSISTENT_VECTOR_OFFSET = 2,
    TEMPORARY_MAP = 3,
    TEMPORARY_UNORDERED_MAP = 4,
    TEMPORARY_MAP_EDITING = 5,
    TEMPORARY_UNORDERED_MAP_EDITING = 6
  };

  namespace Implementation {
    /// Implement evaluation of a given tape with the specified evaluation strategy, specialized for each strategy.
    template<typename Identifier, typename Gradient, Strategy evaluationStrategy>
    struct Evaluate {
      public:
        // static Gradient evaluate(Tape<Identifier, Gradient>& tape, Gradient const& seed) {}
    };

    template<typename Identifier, typename Gradient>
    struct Evaluate<Identifier, Gradient, Strategy::TEMPORARY_VECTOR> {
      public:
        static Gradient evaluate(Tape<Identifier, Gradient>& tape, Gradient const& seed) {
          LocalAdjoints::TemporaryVector<Identifier, Gradient> adjoints;
          adjoints.resize(tape.getMaxIdentifier() + 1);
          return tape.evaluate(adjoints, seed);
        }
    };

    template<typename Identifier, typename Gradient>
    struct Evaluate<Identifier, Gradient, Strategy::PERSISTENT_VECTOR> {
      public:
        static Gradient evaluate(Tape<Identifier, Gradient>& tape, Gradient const& seed) {
          LocalAdjoints::PersistentVector<Identifier, Gradient> adjoints;
          adjoints.resize(tape.getMaxIdentifier() + 1);
          return tape.evaluate(adjoints, seed);
        }
    };

    template<typename Identifier, typename Gradient>
    struct Evaluate<Identifier, Gradient, Strategy::PERSISTENT_VECTOR_OFFSET> {
      public:
        static Gradient evaluate(Tape<Identifier, Gradient>& tape, Gradient const& seed) {
          LocalAdjoints::PersistentVectorOffset<Identifier, Gradient> adjoints(tape.getMinIdentifier());
          adjoints.resize(tape.getMaxIdentifier() - tape.getMinIdentifier() + 1);
          return tape.evaluate(adjoints, seed);
        }
    };

    template<typename Identifier, typename Gradient>
    struct Evaluate<Identifier, Gradient, Strategy::TEMPORARY_MAP> {
      public:
        static Gradient evaluate(Tape<Identifier, Gradient>& tape, Gradient const& seed) {
          LocalAdjoints::TemporaryMapStdMap<Identifier, Gradient> adjoints;
          return tape.evaluate(adjoints, seed);
        }
    };

    template<typename Identifier, typename Gradient>
    struct Evaluate<Identifier, Gradient, Strategy::TEMPORARY_UNORDERED_MAP> {
      public:
        static Gradient evaluate(Tape<Identifier, Gradient>& tape, Gradient const& seed) {
          LocalAdjoints::TemporaryMapStdUnorderedMap<Identifier, Gradient> adjoints;
          return tape.evaluate(adjoints, seed);
        }
    };

    template<typename Identifier, typename Gradient>
    struct Evaluate<Identifier, Gradient, Strategy::TEMPORARY_MAP_EDITING> {
      public:
        static Gradient evaluate(Tape<Identifier, Gradient>& tape, Gradient const& seed) {
          tape.template remapIdentifiers<std::map<Identifier, Identifier>>();  // only remaps on first call
          LocalAdjoints::TemporaryVector<Identifier, Gradient> adjoints;
          adjoints.resize(tape.getMaxIdentifier() + 1);
          return tape.evaluate(adjoints, seed);
        }
    };

    template<typename Identifier, typename Gradient>
    struct Evaluate<Identifier, Gradient, Strategy::TEMPORARY_UNORDERED_MAP_EDITING> {
      public:
        static Gradient evaluate(Tape<Identifier, Gradient>& tape, Gradient const& seed) {
          tape.template remapIdentifiers<std::unordered_map<Identifier, Identifier>>();  // only remaps on first call
          LocalAdjoints::TemporaryVector<Identifier, Gradient> adjoints;
          adjoints.resize(tape.getMaxIdentifier() + 1);
          return tape.evaluate(adjoints, seed);
        }
    };
  }

  /// Evaluate a given tape with the specified evaluation strategy, specialized for the strategies.
  template<typename Identifier, typename Gradient, Strategy evaluationStrategy>
  Gradient evaluate(Tape<Identifier, Gradient>& tape, Gradient const& seed) {
    return Implementation::Evaluate<Identifier, Gradient, evaluationStrategy>::evaluate(tape, seed);
  }

  namespace Implementation {
    /// Implement cleanup of adjoints specific to the evaluation strategy, specialized for the strategies as needed.
    template<typename Identifier, typename Gradient, Strategy evaluationStrategy>
    struct ClearAdjoints {
      public:
        static void clearAdjoints() {}
    };

    template<typename Identifier, typename Gradient>
    struct ClearAdjoints<Identifier, Gradient, Strategy::PERSISTENT_VECTOR>  {
      public:
        static void clearAdjoints() {
          LocalAdjoints::PersistentVector<Identifier, Gradient> adjoints;
          adjoints.clear();
        }
    };

    template<typename Identifier, typename Gradient>
    struct ClearAdjoints<Identifier, Gradient, Strategy::PERSISTENT_VECTOR_OFFSET>  {
      public:
        static void clearAdjoints() {
          LocalAdjoints::PersistentVectorOffset<Identifier, Gradient> adjoints(0);
          adjoints.clear();
        }
    };
  }

  /// Cleanup of adjoints specific to the evaluation strategy.
  template<typename Identifier, typename Gradient, Strategy evaluationStrategy>
  void clearAdjoints() {
    Implementation::ClearAdjoints<Identifier, Gradient, evaluationStrategy>::clearAdjoints();
  }
}
