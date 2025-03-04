#include "TextureData.h"
#include <cstdint>
#include <slang.h>

namespace BgfxSlang {
TextureComponentType convertTextureComponentType(slang::TypeReflection *type) {
  auto texType = type->getResourceResultType()->getScalarType();
  switch (texType) {
  case slang::TypeReflection::ScalarType::Float16:
  case slang::TypeReflection::ScalarType::Float32:
  case slang::TypeReflection::ScalarType::Float64:
    return TextureComponentType::Float;
  case slang::TypeReflection::ScalarType::Int8:
  case slang::TypeReflection::ScalarType::Int16:
  case slang::TypeReflection::ScalarType::Int32:
  case slang::TypeReflection::ScalarType::Int64:
    return TextureComponentType::Int;
  case slang::TypeReflection::ScalarType::UInt8:
  case slang::TypeReflection::ScalarType::UInt16:
  case slang::TypeReflection::ScalarType::UInt32:
  case slang::TypeReflection::ScalarType::UInt64:
    return TextureComponentType::Uint;
  default:
    return TextureComponentType::Float;
  }
}

TextureDimension convertTextureDimension(slang::TypeReflection *type) {
  auto shape = type->getResourceShape();
  switch (shape) {
  case SLANG_TEXTURE_1D:
    return TextureDimension::Dimension1D;
  case SLANG_TEXTURE_2D: // original bgfx compiler vulkan compiler reports simple BgfxSampler as array so we're doing same here
  case SLANG_TEXTURE_2D_ARRAY:
    return TextureDimension::Dimension2DArray;
  case SLANG_TEXTURE_CUBE:
    return TextureDimension::DimensionCube;
  case SLANG_TEXTURE_CUBE_ARRAY:
    return TextureDimension::DimensionCubeArray;
  case SLANG_TEXTURE_3D:
    return TextureDimension::Dimension3D;
  default:
    return TextureDimension::Unknown;
  }
}

TextureFormat convertTextureFormat(slang::TypeLayoutReflection *containerLayout, uint64_t index) {
  SlangImageFormat imageFormat = containerLayout->getBindingRangeImageFormat(index);

  switch (imageFormat) {
  case SLANG_IMAGE_FORMAT_unknown:
    return TextureFormat::Unknown;
  case SLANG_IMAGE_FORMAT_rgba32f:
    return TextureFormat::RGBA32F;
  case SLANG_IMAGE_FORMAT_rgba16f:
    return TextureFormat::RGBA16F;
  case SLANG_IMAGE_FORMAT_rg32f:
    return TextureFormat::RG32F;
  case SLANG_IMAGE_FORMAT_rg16f:
    return TextureFormat::RG16F;
  case SLANG_IMAGE_FORMAT_r11f_g11f_b10f:
    return TextureFormat::RG11B10F;
  case SLANG_IMAGE_FORMAT_r32f:
    return TextureFormat::R32F;
  case SLANG_IMAGE_FORMAT_r16f:
    return TextureFormat::R16F;
  case SLANG_IMAGE_FORMAT_rgba16:
    return TextureFormat::RGBA16;
  case SLANG_IMAGE_FORMAT_rgb10_a2:
    return TextureFormat::RGB10A2;
  case SLANG_IMAGE_FORMAT_rgba8:
    return TextureFormat::RGBA8;
  case SLANG_IMAGE_FORMAT_rg16:
    return TextureFormat::RG16;
  case SLANG_IMAGE_FORMAT_rg8:
    return TextureFormat::RG8;
  case SLANG_IMAGE_FORMAT_r16:
    return TextureFormat::R16;
  case SLANG_IMAGE_FORMAT_r8:
    return TextureFormat::R8;
  case SLANG_IMAGE_FORMAT_rgba16_snorm:
    return TextureFormat::RGBA16S;
  case SLANG_IMAGE_FORMAT_rgba8_snorm:
    return TextureFormat::RGBA8S;
  case SLANG_IMAGE_FORMAT_rg16_snorm:
    return TextureFormat::RG16S;
  case SLANG_IMAGE_FORMAT_rg8_snorm:
    return TextureFormat::RG8S;
  case SLANG_IMAGE_FORMAT_r16_snorm:
    return TextureFormat::R16S;
  case SLANG_IMAGE_FORMAT_r8_snorm:
    return TextureFormat::R8S;
  case SLANG_IMAGE_FORMAT_rgba32i:
    return TextureFormat::RGBA32I;
  case SLANG_IMAGE_FORMAT_rgba16i:
    return TextureFormat::RGBA16I;
  case SLANG_IMAGE_FORMAT_rgba8i:
    return TextureFormat::RGBA8I;
  case SLANG_IMAGE_FORMAT_rg32i:
    return TextureFormat::RG32I;
  case SLANG_IMAGE_FORMAT_rg16i:
    return TextureFormat::RG16I;
  case SLANG_IMAGE_FORMAT_rg8i:
    return TextureFormat::RG8I;
  case SLANG_IMAGE_FORMAT_r32i:
    return TextureFormat::R32I;
  case SLANG_IMAGE_FORMAT_r16i:
    return TextureFormat::R16I;
  case SLANG_IMAGE_FORMAT_r8i:
    return TextureFormat::R8I;
  case SLANG_IMAGE_FORMAT_rgba32ui:
    return TextureFormat::RGBA32U;
  case SLANG_IMAGE_FORMAT_rgba16ui:
    return TextureFormat::RGBA16U;
  case SLANG_IMAGE_FORMAT_rgb10_a2ui:
    return TextureFormat::RGB10A2;
  case SLANG_IMAGE_FORMAT_rgba8ui:
    return TextureFormat::RGBA8U;
  case SLANG_IMAGE_FORMAT_rg32ui:
    return TextureFormat::RG32U;
  case SLANG_IMAGE_FORMAT_rg16ui:
    return TextureFormat::RG16U;
  case SLANG_IMAGE_FORMAT_rg8ui:
    return TextureFormat::RG8U;
  case SLANG_IMAGE_FORMAT_r32ui:
    return TextureFormat::R32U;
  case SLANG_IMAGE_FORMAT_r16ui:
    return TextureFormat::R16U;
  case SLANG_IMAGE_FORMAT_r8ui:
    return TextureFormat::R8U;
  case SLANG_IMAGE_FORMAT_r64ui:
  case SLANG_IMAGE_FORMAT_r64i:
    return TextureFormat::Unknown;
  case SLANG_IMAGE_FORMAT_bgra8:
    return TextureFormat::BGRA8;
  default:
    return TextureFormat::Unknown;
  }
}
} // namespace BgfxSlang