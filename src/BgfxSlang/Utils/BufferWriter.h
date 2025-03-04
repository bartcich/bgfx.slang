#pragma once

#include "IWriter.h"
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace BgfxSlang {

class BufferWriter : public IWriter {
public:
  std::span<uint8_t> GetData() { return buffer; }
  void Clear() { buffer.clear(); }

private:
  std::vector<uint8_t> buffer;

  void write(const void *data, size_t size) override {
    const auto *ptr = static_cast<const uint8_t *>(data);
    buffer.insert(buffer.end(), ptr, ptr + size);
  }
};

} // namespace BgfxSlang