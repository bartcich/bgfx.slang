#pragma once

#include "FileWriter.h"
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <ios>
#include <string>
#include <string_view>
#include <vector>

namespace BgfxSlang {

class Bin2cWriter : public FileWriter {
public:
  explicit Bin2cWriter(std::string_view varName) : varName(varName) {};
  ~Bin2cWriter() override {
    if (file.is_open()) {
      Close();
    };
  }

  inline bool Open(const std::string_view &path) override {

    if (file.is_open()) {
      return false;
    }

    file.open(path.data(), std::ios::binary);

    return file.is_open();
  };

  inline void Close() override {
    writeContentToFile();
    file.close();
  }

private:
  std::string varName;
  std::vector<uint8_t> buffer;
  std::ofstream file;
  constexpr static size_t bytesPerLine = 16;

  void write(const void *data, size_t size) override {
    const auto *ptr = static_cast<const uint8_t *>(data);
    buffer.insert(buffer.end(), ptr, ptr + size);
  }

  void writeContentToFile() {
    auto size = buffer.size();
    file << "static const uint8_t " << varName << "[" << size << "] =\n{\n";

    for (size_t i = 0; i < buffer.size(); i += bytesPerLine) {
      file << "  ";
      for (size_t j = i; j < i + bytesPerLine && j < buffer.size(); ++j) {
        file << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[j]) << ", ";
      }

      // Optionally add spacing if the last line has fewer than 16 bytes.
      size_t bytesInLine = ((i + bytesPerLine) <= buffer.size()) ? bytesPerLine : (buffer.size() - i);
      if (bytesInLine < bytesPerLine) {
        int missing = static_cast<int>(bytesPerLine - bytesInLine);
        const auto missingSpaces = std::string(missing * 6, ' ');
        file << missingSpaces;
      }

      // write ASCII representation
      file << "// ";
      for (size_t j = i; j < i + bytesPerLine && j < buffer.size(); ++j) {
        auto c = buffer[j];
        file << (std::isprint(c) != 0 ? static_cast<char>(c) : '.');
      }

      file << "\n";
    }

    file << "};\n";
  }
};

} // namespace BgfxSlang