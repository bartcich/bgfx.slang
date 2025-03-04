#pragma once

#include "IWriter.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string_view>
#include <vector>

namespace BgfxSlang {

class ConsoleWriter : public IWriter {
public:
private:
  std::vector<uint8_t> buffer;

  void write(const void *data, size_t size) override {
    std::string_view log{static_cast<const char *>(data), size};
    std::cout << log << '\n';
  }
};

} // namespace BgfxSlang