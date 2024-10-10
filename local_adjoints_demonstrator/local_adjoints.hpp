#pragma once

#include <omp.h>
#include <map>
#include <unordered_map>
#include <vector>

namespace LocalAdjoints {

  /// General interface for implementations of adjoint variables.
  template<typename Identifier, typename Gradient>
  struct AdjointsInterface {
    public:
      Gradient& operator[](Identifier identifier);
      Gradient const& operator[](Identifier identifier) const;
      void resize(size_t size);
      void clear();
  };

  /// Template for temporary mapped adjoint variables (underlying map to be specified).
  template<typename Identifier, typename Gradient, typename BasicMap>
  struct TemporaryMap : public AdjointsInterface<Identifier, Gradient> {
    public:
      BasicMap map;

      Gradient& operator[](Identifier identifier) {
        return map[identifier];
      }

      Gradient const& operator[](Identifier identifier) const {
        return map[identifier];
      }

      void resize(size_t) {}
      void clear() {
        map.clear();
      }
  };

  /// Temporary mapped adjoint variables via std::map.
  template<typename Identifier, typename Gradient>
  using TemporaryMapStdMap = TemporaryMap<Identifier, Gradient, std::map<Identifier, Gradient>>;

  /// Temporary mapped adjoint variables via std::unordered_map.
  template<typename Identifier, typename Gradient>
  using TemporaryMapStdUnorderedMap = TemporaryMap<Identifier, Gradient, std::unordered_map<Identifier, Gradient>>;

  /// Temporary vector of adjoint variables.
  template<typename Identifier, typename Gradient>
  struct TemporaryVector : public AdjointsInterface<Identifier, Gradient> {
    public:
      std::vector<Gradient> vector;

      Gradient& operator[](Identifier identifier) {
        return vector[identifier];
      }

      Gradient const& operator[](Identifier identifier) const {
        return vector[identifier];
      }

      void resize(size_t size) {
        vector.resize(size);
      }

      void clear() {
        vector = std::vector<Gradient>();
      }
  };

  /// Base class for persistent vector-based adjoint variables (underlying thread-local memory reused across instances).
  template<typename Identifier, typename Gradient>
  struct PersistentVectorBase : public AdjointsInterface<Identifier, Gradient> {
    public:
      static std::vector<Gradient>* vector;
      #pragma omp threadprivate(vector)

      void resize(size_t size) {
        vector->resize(size);
      }

      void clear() {
        *vector = std::vector<Gradient>();
      }
  };

  template<typename Identifier, typename Gradient>
  std::vector<Gradient>* PersistentVectorBase<Identifier, Gradient>::vector = new std::vector<Gradient>();

  /// Persistent vector of adjoint variables.
  template<typename Identifier, typename Gradient>
  struct PersistentVector : public PersistentVectorBase<Identifier, Gradient> {
    public:
      using Base = PersistentVectorBase<Identifier, Gradient>;

      Gradient& operator[](Identifier identifier) {
        return Base::vector->operator [](identifier);
      }

      Gradient const& operator[](Identifier identifier) const {
        return Base::vector->operator [](identifier);
      }
  };

  /// Persistent vector of adjoint variables, addressing with offset.
  template<typename Identifier, typename Gradient>
  struct PersistentVectorOffset : public PersistentVectorBase<Identifier, Gradient> {
    public:
      using Base = PersistentVectorBase<Identifier, Gradient>;

      Identifier offset;

      PersistentVectorOffset(Identifier offset) : offset(offset) {}

      Gradient& operator[](Identifier identifier) {
        return Base::vector->operator [](identifier);
      }

      Gradient const& operator[](Identifier identifier) const {
        return Base::vector->operator [](identifier);
      }
  };
}
