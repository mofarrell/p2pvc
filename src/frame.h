#pragma once

#include <array>
#include <vector>

template<typename T, uint32_t Dims>
struct Frame {
  Frame(std::array<uint32_t, Dims> dims)
  : m_dims(dims)
  , m_data([&] {
      uint32_t size = dims[0];
      for (uint32_t i = 1; i < dims.size(); i++) {
        size *= dims[i];
      }
      return size;
    }())
  {}

  uint32_t size() const {
    return m_data.size();
  }
  
  uint32_t dim(uint32_t d) const {
    return m_dims[d];
  }

  const T* data() const {
    return m_data.data();
  }

  T* data() {
    return m_data.data();
  }
   
private:
  std::array<uint32_t, Dims> m_dims;
  std::vector<T> m_data;
};
