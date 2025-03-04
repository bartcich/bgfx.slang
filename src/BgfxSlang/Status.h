#pragma once

#include <slang-com-ptr.h>
#include <slang.h>
#include <string>
#include <string_view>
#include <utility>

namespace BgfxSlang {

enum class StatusCode {
  Ok,
  Warning,
  Error,
};

class Status {

public:
  Status() : code(StatusCode::Ok) {}
  explicit Status(StatusCode code) : code(code) {}
  explicit Status(StatusCode code, const std::string &message) : code(code), message(message) {}
  Status(StatusCode code, std::string &&message) : code(code), message(std::move(message)) {}
  explicit Status(StatusCode code, ISlangBlob *diagnostics)
      : code(code), message(static_cast<const char *>(diagnostics->getBufferPointer()), diagnostics->getBufferSize()) {}

  [[nodiscard]] inline bool IsOk() const { return code == StatusCode::Ok; }
  [[nodiscard]] inline bool IsWarning() const { return code == StatusCode::Warning; }
  [[nodiscard]] inline bool IsError() const { return code == StatusCode::Error; }

  [[nodiscard]] inline std::string_view GetMessage() const { return message; }

private:
  StatusCode code;
  std::string message;
};

} // namespace BgfxSlang