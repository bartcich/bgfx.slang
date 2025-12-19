# bgfx.slang - slang shader compiler for bgfx

## What is it?
bgfx.slang is a library and command line tool that compiles shaders written in [slang](https://github.com/shader-slang/slang) to format compatible with the [bgfx](https://github.com/bkaradzic/bgfx).

**It's still in experimental state and it might not work as expected**

Currently supported backends:
- DirectX
- Vulkan
- OpenGL - through SPIR-V cross compilation and regex replacements
- OpenGLES - through SPIR-V cross compilation and regex replacements

Latest slang version tested: 2025.22.1

## Content

- [How to build](#how-to-build)
  - [Using with vcpkg](#using-with-vcpkg)
- [How to write shaders](#how-to-write-shaders)
- [How to use tool](#how-to-use-tool)
  - [Tool with cmake](#tool-with-cmake)
- [How to use library](#how-to-use-library)

## How to build

The easiest way to build bgfx.slang is to use CMake.

```
mkdir build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES=Release
cmake --build build --config Release
```

This will build both the library and the command line tool. By default it will also download slang binaries and build spriv-cross from source. You can also use `BGFXSLANG_EXTERNAL_LIBS` option to use your own slang and spirv-cross builds.

### Using with vcpkg

This library is too young to be included in official vcpkg repo. But you can add it as custom port. See [vcpkg-port-example/bgfx-slang](vcpkg-port-example/bgfx-slang) for example portfile.

## How to write shaders

Please refer to the [provided examples](examples/) for more details on how to write shaders for bgfx using slang.

There are couple of important things to note:
- unlike bgfx shaderc, bgfx.slang does not require you to keep the Vertex Shaders Attributes names. When necessary, it will remap them automatically (glsl and gles) based on semantics. The only exception are the instance buffer input attributes which must have `data` string in their names (also applies only to glsl and gles).
- there is no `bgfx_shader.sh` or `bgfx_compute.sh`. Most of the differences between backends should be handled automatically by slang. This means that you need to define predefined uniforms that you use in your shader (there's no easy way to exclude unused uniforms with slang).
- you can use user attributes to tag your entry points for easier identification in your engine code (for example you can tag your shadow pass shaders with `[Pass("CastShadow")]` attribute). See [User attributes](#user-attributes) section above for more details.
- Slang does not support OpenGLES directly and emits only latest OpenGL code version. All the uniforms are always combined into constant buffer and bgfx requires old style plain uniforms declarations. Because of that, OpenGL and OpenGLES backends are implemented using SPIR-V cross compilation and manual code parsing and modifications. This means that some constructs might not work as expected. It is also possible that using different slang versions might lead to corrupted code generation for these backends.

## How to use tool

Basic usage:
```
bgfx-slang-cmd input.slang -t dx -t spirv -o path/{{target}}/{{stage}}_{{name}}.bin
```

Options:
- `-o, --output <output>` - output path template. Supported template variables: `{{name}}`, `{{filename}}`, `{{entryPoint}}`, `{{stage}}`, `{{target}}`
- `-t, --target <target>` - target backend. Supported targets: `dx`, `spirv`, `glsl`, `gles` (or [specific version](src/BgfxSlang/Target.h#L64)). You can specify multiple targets by using this option multiple times.
- `-v, --verbose` - enable verbose output
- `-b, --bin2c <variable_name_format>` - generate C header with binary data - should be followed by variable name format for example `{{name}}_{{stage}}_{{target}}`
- `-i, --include <path>` - additional include path for slang compiler (directory where your slang libraries are located). Can be specified multiple times.
- `-s, --stage <stage>` - specify shader stage to compile. Supported stages: `vs`, `fs`, `cs`. If not specified, all stages found in the input file will be compiled.

### Tool with cmake
The cmake configuration provides function to combine tool execution with your project build similar to what [bgfx.cmake](https://github.com/bkaradzic/bgfx.cmake) does for bgfx shaderc.

Example usage:

```cmake
bgfx_slang_compile_shaders(
    INPUT_SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shader.slang
    INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/slang/includes
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders
    OUTPUT_PATTERN {{target}}/{{stage}}_{{name}}.bin
)
```

## How to use library

Link the library with Cmake:

```cmake
target_link_libraries(your-target PUBLIC bgfx-slang::bgfx-slang)
```

Simplest usage example:

```cpp
#include <bgfx-slang/Compiler.h>
#include <bgfx-slang/Utils/BufferWriter.h>

const std::string includePath = "path/to/your/slang/includes";
const std::string inputPath = "path/to/your/shader.slang";


BgfxSlang::Compiler compiler;
compiler.AddModulesSearchPath(includePath);

compiler.AddTarget("dx");
compiler.AddTarget("spirv");

compiler.LoadProgramFromPath(inputPath);

for (int targetIdx = 0; targetIdx < compiler.GetTargetCount(); targetIdx++) {
  auto target = compiler.GetTarget(targetIdx);

   for (int entryPointIdx = 0; entryPointIdx < compiler.GetEntryPointCount(); entryPointIdx++) {
    BgfxSlang::BufferWriter writer;
    const auto *entryPoint = compiler.GetEntryPointByIndex(entryPointIdx);
    compiler.Compile(entryPoint->Idx, targetIdx, writer);

    auto data = writer.GetData(); // <- here is your compiled shader binary for current target and entry point
   }
}
```

For more advanced usage please check the [source code of the command line tool](tools/main.cpp).

### Additional functions available in library that are not used in the command line tool:

#### Entry point caching

It is possible to cache entry points to avoid not necessary compilations. The library provides hash of the entry point generated by slang:

```cpp
const auto hash = entryPoint->GetHash(target.Format);
```

#### User attributes

Slang allows to define user attributes for entry points.

```hlsl
[__AttributeUsage(_AttributeTargets.Function)]
public struct PassAttribute {
  string name;
}

[Pass("CastShadow")]
[shader("vertex")]
float4 vertexCastShadows(VertexInput input) : SV_Position {
  float3 position = calcPosition(input.position);
  return mul(u_modelViewProj, float4(position, 1.0));
};
```
 
 You can access them through the library:

```cpp
#include <bgfx-slang/Attributes.h>


// list all the attributes of an entry point and their arguments
for (const auto &attr : entryPoint.Attributes) {
  const auto name = attr.GetName();
  const auto argsCount = attr.GetArgumentCount();
  
  for (int i = 0; i < argsCount; i++) {
    const auto argType = attr.GetArgumentType(i);

    if (argType == BgfxSlang::ArgumentType::String) {
      const auto value = attr.GetArgumentValueString(i);
    } else if (argType == BgfxSlang::ArgumentType::Int) {
      const auto value = attr.GetArgumentValueInt(i);
    } else if (argType == BgfxSlang::ArgumentType::Float) {
      const auto value = attr.GetArgumentValueFloat(i);
    }
  }
}
```

Or if you know the attribute name you can get it directly:

```cpp
if (entryPoint.HasUserAttribute("Pass")) {
  const auto *attr = entryPoint.GetUserAttribute("Pass");
  const auto passName = attr->GetArgumentValueString(0);
}
```

