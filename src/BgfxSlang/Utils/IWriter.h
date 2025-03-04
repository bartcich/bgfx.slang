#pragma once

#include <cstddef>
#include <string_view>
#include <type_traits>

namespace BgfxSlang {

class IWriter {
public:
  virtual ~IWriter() = default;

  template <typename T>
    requires std::is_trivially_copyable_v<T>
  inline void Write(const T &data) {
    Write(&data, sizeof(T));
  }

  inline void Write(std::string_view data) { Write(data.data(), data.size()); }

  inline void Write(const void *data, size_t size) { write(data, size); }

private:
  virtual void write(const void *data, size_t size) = 0;
};

} // namespace BgfxSlang