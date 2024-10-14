#pragma once

#include "evaluation_strategies.hpp"
#include "local_adjoints.hpp"
#include "tape.hpp"

/// Emulates simultaneous preaccumulations in multiple threads.
template<typename Identifier, typename Gradient>
struct Preaccumulations {
  public:
    size_t nPreaccs;
    size_t preaccSizeMin;
    size_t preaccSizeMax;
    size_t nEvalMin;
    size_t nEvalMax;
    Identifier iMin;
    Identifier iMax;
    size_t randomSeed;

    Preaccumulations(size_t nPreaccs, size_t preaccSizeMin, size_t preaccSizeMax, size_t nEvalMin, size_t nEvalMax,
                     Identifier iMin, Identifier iMax, size_t randomSeed)
        : nPreaccs(nPreaccs), preaccSizeMin(preaccSizeMin), preaccSizeMax(preaccSizeMax), nEvalMin(nEvalMin),
          nEvalMax(nEvalMax), iMin(iMin), iMax(iMax), randomSeed(randomSeed) {}

    /// Run simultaneous preaccumulations with the specified evaluation strategy.
    template<EvaluationStrategy::Strategy evaluationStrategy>
    Gradient run(Gradient const& seed) {
      Gradient result = 0.0;

      #pragma omp parallel
      {
        #pragma omp for reduction(+:result)
        for (size_t i = 0; i < nPreaccs; ++i) {
          // produce a random seed specific to this preaccumulation
          std::mt19937 preaccSeedGenerator(randomSeed + i);
          size_t preaccSeed = preaccSeedGenerator();

          // generate a tape, mimicking the preaccumulation-associated recording
          std::mt19937 generator(preaccSeed);
          std::uniform_int_distribution<size_t> preaccSizeDistribution(preaccSizeMin, preaccSizeMax);
          auto tape = Tape<Identifier, Gradient>::generate(preaccSizeDistribution(generator), iMin, iMax, preaccSeed);

          // evaluate the tape, possibly multiple times to emulate multiple preaccumulation inputs/outputs
          std::uniform_int_distribution<size_t> nEvalDistribution(nEvalMin, nEvalMax);
          size_t nEval = nEvalDistribution(generator);
          for (size_t i = 0; i < nEval; ++i) {
            result += EvaluationStrategy::evaluate<Identifier, Gradient, evaluationStrategy>(*tape, seed + 0.1 * std::sin(i));
          }
        }

        EvaluationStrategy::clearAdjoints<Identifier, Gradient, evaluationStrategy>();
      }

      return result;
    }
};
