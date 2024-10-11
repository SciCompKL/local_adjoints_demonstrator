#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <omp.h>
#include <ostream>

#include "preaccumulations.hpp"

template<typename Gradient>
struct PerformanceData {
  public:
    size_t nThreads;
    size_t nWarmups;
    size_t nRuns;

    double runtimeAvg, runtimeMin, runtimeMax;
    double memoryHwm;

    Gradient result;
};

template<typename Gradient>
std::ostream& operator<<(std::ostream& out, PerformanceData<Gradient> const& data) {
  out << std::setw(5) << data.nThreads
      << std::setw(5) << data.nWarmups
      << std::setw(5) << data.nRuns
      << std::setw(16) << data.runtimeAvg
      << std::setw(16) << data.runtimeMin
      << std::setw(16) << data.runtimeMax
      << std::setw(16) << data.memoryHwm
      << std::setw(16) << data.result;
  return out;
}

template<typename Identifier, typename Gradient>
struct Benchmark {
  public:
    size_t nWarmups;
    size_t nRuns;

    Benchmark(size_t nWarmups, size_t nRuns) : nWarmups(nWarmups), nRuns(nRuns) {}

    /// Get memory high water mark in MB.
    double getMemoryHWM() {
      std::ifstream process("/proc/self/status");

      std::string buffer;
      do {
        process >> buffer;
      } while (buffer != "VmHWM:");

      size_t result = 0;
      process >> result;

      return static_cast<double>(result) / 1024.0;
    }

    /// Benchmarks simultaneous preaccumulations.
    template<EvaluationStrategy::Strategy strategy>
    PerformanceData<Gradient> run(Preaccumulations<Identifier, Gradient>& preaccs) {
      PerformanceData<Gradient> data;
      data.nThreads = omp_get_max_threads();
      data.nWarmups = nWarmups;
      data.nRuns = nRuns;
      data.runtimeAvg = 0.0;
      data.runtimeMin = std::numeric_limits<double>::max();
      data.runtimeMax = std::numeric_limits<double>::min();
      data.result = 0.0;

      for (size_t i = 0; i < nWarmups; ++i) {
        data.result += preaccs.template run<strategy>(1.0);
      }

      for (size_t i = 0; i < nRuns; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        data.result += preaccs.template run<strategy>(1.0);
        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

        data.runtimeAvg = (data.runtimeAvg * i + elapsed) / (i + 1);
        data.runtimeMin = std::min(data.runtimeMin, elapsed);
        data.runtimeMax = std::max(data.runtimeMax, elapsed);
      }

      data.memoryHwm = getMemoryHWM();

      return data;
    }
};
