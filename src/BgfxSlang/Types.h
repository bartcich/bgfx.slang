#pragma once

#include "BgfxSlang/Target.h"
#include "TextureData.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

namespace BgfxSlang {

enum class UniformType : uint8_t {
  Sampler,
  End,
  Vec4,
  Mat3,
  Mat4,

  Unknown,
};

inline std::string_view uniformTypeToString(UniformType type) {
  switch (type) {
  case UniformType::Sampler:
    return "sampler";
  case UniformType::Vec4:
    return "vec4";
  case UniformType::Mat3:
    return "mat3";
  case UniformType::Mat4:
    return "mat4";
  default:
    return "unknown";
  }
}

struct Uniform {
  std::string Name;
  UniformType Type;
  uint8_t Count;
  uint16_t RegIndex;
  uint16_t RegCount;
  TextureComponentType TexComponent = TextureComponentType::Float;
  TextureDimension TexDimension = TextureDimension::Dimension1D;
  TextureFormat TexFormat = TextureFormat::BC1;
};

enum class Attrib : uint8_t {
  Position,
  Normal,
  Tangent,
  Bitangent,
  Color0,
  Color1,
  Color2,
  Color3,
  Indices,
  Weight,
  TexCoord0,
  TexCoord1,
  TexCoord2,
  TexCoord3,
  TexCoord4,
  TexCoord5,
  TexCoord6,
  TexCoord7,

  Unknown,
  Internal
};

inline std::string_view attribToString(Attrib attr) {
  switch (attr) {
  case Attrib::Position:
    return "POSITION";
  case Attrib::Normal:
    return "NORMAL";
  case Attrib::Tangent:
    return "TANGENT";
  case Attrib::Bitangent:
    return "BITANGENT";
  case Attrib::Color0:
    return "COLOR0";
  case Attrib::Color1:
    return "COLOR1";
  case Attrib::Color2:
    return "COLOR2";
  case Attrib::Color3:
    return "COLOR3";
  case Attrib::Indices:
    return "INDICES";
  case Attrib::Weight:
    return "WEIGHT";
  case Attrib::TexCoord0:
    return "TEXCOORD0";
  case Attrib::TexCoord1:
    return "TEXCOORD1";
  case Attrib::TexCoord2:
    return "TEXCOORD2";
  case Attrib::TexCoord3:
    return "TEXCOORD3";
  case Attrib::TexCoord4:
    return "TEXCOORD4";
  case Attrib::TexCoord5:
    return "TEXCOORD5";
  case Attrib::TexCoord6:
    return "TEXCOORD6";
  case Attrib::TexCoord7:
    return "TEXCOORD7";
  default:
    return "unknown";
  }
}

struct Param {
  std::string Name;
  Attrib Attr;
};

struct AttribToId {
  Attrib Attr;
  uint16_t Id;
};

constexpr std::array<AttribToId, 23> attribToIdMap = {{
    {Attrib::Position, 0x0001},
    {Attrib::Normal, 0x0002},
    {Attrib::Tangent, 0x0003},
    {Attrib::Bitangent, 0x0004},
    {Attrib::Color0, 0x0005},
    {Attrib::Color1, 0x0006},
    {Attrib::Color2, 0x0018},
    {Attrib::Color3, 0x0019},
    {Attrib::Indices, 0x000e},
    {Attrib::Weight, 0x000f},
    {Attrib::TexCoord0, 0x0010},
    {Attrib::TexCoord1, 0x0011},
    {Attrib::TexCoord2, 0x0012},
    {Attrib::TexCoord3, 0x0013},
    {Attrib::TexCoord4, 0x0014},
    {Attrib::TexCoord5, 0x0015},
    {Attrib::TexCoord6, 0x0016},
    {Attrib::TexCoord7, 0x0017},
}};

inline uint16_t attribToId(Attrib attr) { return attribToIdMap.at(static_cast<size_t>(attr)).Id; }

inline uint16_t paramToId(const Param &param, TargetFormat target) {
  if (target == TargetFormat::SpirV && param.Name.find("data") != std::string::npos) {
    return std::numeric_limits<uint16_t>::max();
  }
  return attribToId(param.Attr);
}

} // namespace BgfxSlang