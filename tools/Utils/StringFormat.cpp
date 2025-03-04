#include "StringFormat.h"
#include <cstddef>
#include <initializer_list>
#include <string>
#include <string_view>

namespace BgfxSlangCmd {
std::string formatString(std::string_view format, std::initializer_list<StringFormatParam> params) {
  std::string str{format};
  for (const auto &param : params) {
    size_t pos = 0;
    while ((pos = str.find(param.Target, pos)) != std::string::npos) {
      str.replace(pos, param.Target.size(), param.Value);
      pos += param.Value.size();
    }
  }
  return str;
}
} // namespace BgfxSlangCmd