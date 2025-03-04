#pragma once

#include <initializer_list>
#include <string>
#include <string_view>

namespace BgfxSlangCmd {

struct StringFormatParam {
  std::string_view Target;
  std::string_view Value;
};

std::string formatString(std::string_view format, std::initializer_list<StringFormatParam> params);

} // namespace BgfxSlangCmd