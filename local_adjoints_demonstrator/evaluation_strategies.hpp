#pragma once

#include <list>

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
    /// Implement evaluations of a given tape with the specified evaluation strategy, specialized for each strategy.
    template<typename Identifier, typename Gradient, Strategy evaluationStrategy>
    struct Evaluate {
      public:
        // static Gradient evaluate(Tape<Identifier, Gradient>& tape, std::list<Gradient> const& seeds) {}
    };

    template<typename Identifier, typename Gradient>
    struct Evaluate<Identifier, Gradient, Strategy::TEMPORARY_VECTOR> {
      public:
        static Gradient evaluate(Tape<Identifier, Gradient>& tape, std::list<Gradient> const& seeds) {
          LocalAdjoints::TemporaryVector<Identifier, Gradient> adjoints;
          adjoints.resize(tape.getMaxIdentifier() + 1);
          Gradient result = 0.0;
          for (auto const& seed : seeds) {
            result += tape.evaluate(adjoints, seed);
          }
          return result;
        }
    };

    template<typename Identifier, typename Gradient>
    struct Evaluate<Identifier, Gradient, Strategy::PERSISTENT_VECTOR> {
      public:
        static Gradient evaluate(Tape<Identifier, Gradient>& tape, std::list<Gradient> const& seeds) {
          LocalAdjoints::PersistentVector<Identifier, Gradient> adjoints;
          adjoints.resize(tape.getMaxIdentifier() + 1);
          Gradient result = 0.0;
          for (auto const& seed : seeds) {
            result += tape.evaluate(adjoints, seed);
          }
          return result;
        }
    };

    template<typename Identifier, typename Gradient>
    struct Evaluate<Identifier, Gradient, Strategy::PERSISTENT_VECTOR_OFFSET> {
      public:
        static Gradient evaluate(Tape<Identifier, Gradient>& tape, std::list<Gradient> const& seeds) {
          LocalAdjoints::PersistentVectorOffset<Identifier, Gradient> adjoints(tape.getMinIdentifier());
          adjoints.resize(tape.getMaxIdentifier() - tape.getMinIdentifier() + 1);
          Gradient result = 0.0;
          for (auto const& seed : seeds) {
            result += tape.evaluate(adjoints, seed);
          }
          return result;
        }
    };

    template<typename Identifier, typename Gradient>
    struct Evaluate<Identifier, Gradient, Strategy::TEMPORARY_MAP> {
      public:
        static Gradient evaluate(Tape<Identifier, Gradient>& tape, std::list<Gradient> const& seeds) {
          LocalAdjoints::TemporaryMapStdMap<Identifier, Gradient> adjoints;
          Gradient result = 0.0;
          for (auto const& seed : seeds) {
            result += tape.evaluate(adjoints, seed);
          }
          return result;
        }
    };

    template<typename Identifier, typename Gradient>
    struct Evaluate<Identifier, Gradient, Strategy::TEMPORARY_UNORDERED_MAP> {
      public:
        static Gradient evaluate(Tape<Identifier, Gradient>& tape, std::list<Gradient> const& seeds) {
          LocalAdjoints::TemporaryMapStdUnorderedMap<Identifier, Gradient> adjoints;
          Gradient result = 0.0;
          for (auto const& seed : seeds) {
            result += tape.evaluate(adjoints, seed);
          }
          return result;
        }
    };

    template<typename Identifier, typename Gradient>
    struct Evaluate<Identifier, Gradient, Strategy::TEMPORARY_MAP_EDITING> {
      public:
        static Gradient evaluate(Tape<Identifier, Gradient>& tape, std::list<Gradient> const& seeds) {
          tape.template remapIdentifiers<std::map<Identifier, Identifier>>();
          LocalAdjoints::TemporaryVector<Identifier, Gradient> adjoints;
          adjoints.resize(tape.getMaxIdentifier() + 1);
          Gradient result = 0.0;
          for (auto const& seed : seeds) {
            result += tape.evaluate(adjoints, seed);
          }
          return result;
        }
    };

    template<typename Identifier, typename Gradient>
    struct Evaluate<Identifier, Gradient, Strategy::TEMPORARY_UNORDERED_MAP_EDITING> {
      public:
        static Gradient evaluate(Tape<Identifier, Gradient>& tape, std::list<Gradient> const& seeds) {
          tape.template remapIdentifiers<std::unordered_map<Identifier, Identifier>>();
          LocalAdjoints::TemporaryVector<Identifier, Gradient> adjoints;
          adjoints.resize(tape.getMaxIdentifier() + 1);
          Gradient result = 0.0;
          for (auto const& seed : seeds) {
            result += tape.evaluate(adjoints, seed);
          }
          return result;
        }
    };
  }

  /// Evaluate a given tape, possibly multiple times, with the specified evaluation strategy and the given seeds.
  template<typename Identifier, typename Gradient, Strategy evaluationStrategy>
  Gradient evaluate(Tape<Identifier, Gradient>& tape, std::list<Gradient> const& seeds) {
    return Implementation::Evaluate<Identifier, Gradient, evaluationStrategy>::evaluate(tape, seeds);
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
