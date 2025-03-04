#pragma once

#include "IWriter.h"
#include <cstddef>
#include <fstream>
#include <ios>
#include <string_view>

namespace BgfxSlang {

class FileWriter : public IWriter {

public:
  explicit FileWriter() = default;
  ~FileWriter() override {
    if (file.is_open()) {
      file.close();
    }
  };

  inline bool Open(const std::string_view &path) {

    if (file.is_open()) {
      return false;
    }

    file.open(path.data(), std::ios::binary);

    return file.is_open();
  };

  inline void Close() { file.close(); }

private:
  void write(const void *data, size_t size) override { file.write(static_cast<const char *>(data), size); }

  std::ofstream file;
};

} // namespace BgfxSlang