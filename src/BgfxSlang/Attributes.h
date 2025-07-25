#pragma once

#include <cstddef>
#include <slang.h>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace BgfxSlang {

enum class ArgumentType {
  Unknown,
  Int,
  Float,
  String,
};

class UserAttribute {
public:
  explicit UserAttribute(std::string_view name) : name(name) {}

  [[nodiscard]] inline std::string_view GetName() const { return name; }

  void AddArgumentValue(int value) { args.emplace_back(ArgumentType::Int, value); }
  void AddArgumentValue(float value) { args.emplace_back(ArgumentType::Float, value); }
  void AddArgumentValue(std::string_view value) { args.emplace_back(ArgumentType::String, std::string(value)); }

  [[nodiscard]] size_t GetArgumentCount() const { return args.size(); }

  [[nodiscard]] ArgumentType GetArgumentType(size_t index) const {
    if (index < args.size()) {
      return args[index].Type;
    }
    return ArgumentType::Unknown;
  }

  [[nodiscard]] int GetArgumentValueInt(size_t index) const {
    if (index < args.size() && args[index].Type == ArgumentType::Int) {
      return std::get<int>(args[index].Value);
    }
    return 0;
  }

  [[nodiscard]] float GetArgumentValueFloat(size_t index) const {
    if (index < args.size() && args[index].Type == ArgumentType::Float) {
      return std::get<float>(args[index].Value);
    }
    return 0.0f;
  }

  [[nodiscard]] std::string_view GetArgumentValueString(size_t index) const {
    if (index < args.size() && args[index].Type == ArgumentType::String) {
      return std::get<std::string>(args[index].Value);
    }
    return {};
  }

  static UserAttribute FromSlangAttribute(slang::UserAttribute *attr) {
    UserAttribute userAttr(attr->getName());
    for (int i = 0; i < attr->getArgumentCount(); ++i) {
      switch (attr->getArgumentType(i)->getKind()) {
      case slang::TypeReflection::Kind::Struct: {
        size_t stringSize = 0;
        const auto *stringValue = attr->getArgumentValueString(i, &stringSize);
        std::string_view strValue(stringValue + 1, stringSize - 2);
        userAttr.AddArgumentValue(strValue);
        break;
      }
      case slang::TypeReflection::Kind::Scalar: {
        if (attr->getArgumentType(i)->getScalarType() == slang::TypeReflection::ScalarType::Int32) {
          int v = 0;
          attr->getArgumentValueInt(i, &v);
          userAttr.AddArgumentValue(v);
        } else if (attr->getArgumentType(i)->getScalarType() == slang::TypeReflection::ScalarType::Float32) {
          float v = 0.0f;
          attr->getArgumentValueFloat(i, &v);
          userAttr.AddArgumentValue(v);
        }
        break;
      }
      default:
        break;
      }
    }
    return userAttr;
  }

private:
  struct Arg {
    ArgumentType Type;
    std::variant<int, float, std::string> Value;
    Arg(ArgumentType t, const std::variant<int, float, std::string> &v) : Type(t), Value(v) {}
  };

  std::string name;
  std::vector<Arg> args;
};

} // namespace BgfxSlang