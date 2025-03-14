#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

namespace BgfxSlangCmd {

enum class TokenType { Input, Output, Target, Verbose, Bin2C, Include };
struct Token {
  TokenType Type;
  std::string_view Short;
  std::string_view Long;
};

constexpr std::array tokens = {
    Token{TokenType::Input, "", ""},
    Token{TokenType::Output, "-o", "--output"},
    Token{TokenType::Target, "-t", "--target"},
    Token{TokenType::Verbose, "-v", "--verbose"},
    Token{TokenType::Bin2C, "-b", "--bin2c"},
    Token{TokenType::Include, "-i", "--include"},
};

struct TokenValues {
  TokenType Type;
  std::vector<std::string_view> Values;
};

class CmdLine {
public:
  CmdLine(int argc, char **argv) { process(argc, argv); }

  [[nodiscard]] bool Has(TokenType type) const { return find(type) != nullptr; }

  [[nodiscard]] int64_t GetCount(TokenType type) const {
    const auto *value = find(type);
    return value != nullptr ? value->Values.size() : 0;
  }

  [[nodiscard]] std::string_view GetOne(TokenType type, std::string_view defaultValue = "") const {
    const auto *value = find(type);
    return value != nullptr && !value->Values.empty() ? value->Values.front() : defaultValue;
  }

  [[nodiscard]] const std::vector<std::string_view> *Get(TokenType type) const {
    const auto *value = find(type);
    return value != nullptr ? &value->Values : nullptr;
  }

private:
  std::vector<TokenValues> values;

  void process(int argc, char **argv) {
    values.reserve(tokens.size());

    TokenType currentToken = TokenType::Input;
    values.emplace_back(currentToken);

    for (int i = 1; i < argc; i++) {
      std::string_view arg = argv[i];

      bool tokenFound = false;
      for (const auto &token : tokens) {
        if (arg == token.Short || arg == token.Long) {
          tokenFound = true;
          currentToken = token.Type;
          auto *value = const_cast<TokenValues *>(find(currentToken));
          if (value == nullptr) {
            values.emplace_back(currentToken);
          }
          break;
        }
      }
      if (tokenFound) {
        continue;
      }

      auto *value = const_cast<TokenValues *>(find(currentToken));

      value->Values.push_back(arg);
      currentToken = TokenType::Input;
    }
  }

  [[nodiscard]] const TokenValues *find(TokenType type) const {
    for (const auto &value : values) {
      if (value.Type == type) {
        return &value;
      }
    }

    return nullptr;
  }
};

} // namespace BgfxSlangCmd