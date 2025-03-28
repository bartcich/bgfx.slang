function(_bgfx_slang_get_target_path_name TARGET TARGET_PATH_NAME)
  string(REPLACE gles_300 essl TARGET ${TARGET})
  string(REPLACE glsl_150 glsl TARGET ${TARGET})
  string(REPLACE dx dx11 TARGET ${TARGET})
  set(${TARGET_PATH_NAME} ${TARGET} PARENT_SCOPE)
endfunction()

function(_bgfx_slang_get_target_header_var_name TARGET TARGET_VAR_NAME)
  string(REPLACE gles_300 essl TARGET ${TARGET})
  string(REPLACE glsl_150 glsl TARGET ${TARGET})
  string(REPLACE spirv spv TARGET ${TARGET})
  string(REPLACE dx dx11 TARGET ${TARGET})
  set(${TARGET_VAR_NAME} ${TARGET} PARENT_SCOPE)
endfunction()

function(bgfx_slang_compile_shaders)
  set(options AS_HEADERS VERBOSE)
  set(oneValueArgs OUTPUT_DIR OUTPUT_PATTERN OUT_FILES_VAR HEADER_VAR_PATTERN)
  set(multiValueArgs TYPES INPUT_SHADERS INCLUDE_DIRS)
  cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" "${ARGN}")

  set(TARGETS glsl_150 gles_300 spirv)
  if (WIN32 OR MINGW OR MSYS OR CYGWIN)
    list(APPEND TARGETS dx)
  endif()

  find_program(BGFX_SLANG_CMD_EXECUTABLE
    NAMES bgfx-slang-cmd
  )

  set(ALL_OUTPUTS "")
  foreach(INPUT_SHADER_FILE ${ARGS_INPUT_SHADERS})
    source_group("Shaders" FILES ${INPUT_SHADER_FILE})
    get_filename_component(SHADER_FILE_NAME_WE ${INPUT_SHADER_FILE} NAME_WE)
    get_filename_component(SHADER_FILE_BASENAME ${INPUT_SHADER_FILE} NAME)
    get_filename_component(SHADER_FILE_ABSOLUTE ${INPUT_SHADER_FILE} ABSOLUTE)

    set(CLI "")
    
    if (ARGS_AS_HEADERS)
      set(HEADER_SUFFIX .h)
      if (ARGS_HEADER_VAR_PATTERN)
        list(APPEND CLI "-b" "${ARGS_HEADER_VAR_PATTERN}")
      else()
        list(APPEND CLI "-b" "{{stage}}_{{name}}_{{target}}")
      endif()
    endif()

    if (ARGS_OUTPUT_PATTERN)
      set(OUTPUT_PATTERN ${ARGS_OUTPUT_DIR}/${ARGS_OUTPUT_PATTERN}${HEADER_SUFFIX})
    else()
      set(OUTPUT_PATTERN ${ARGS_OUTPUT_DIR}/{{target}}/{{stage}}_${SHADER_FILE_BASENAME}.bin${HEADER_SUFFIX})
    endif()
    
    # input shader
    list(APPEND CLI "${INPUT_SHADER_FILE}")
    list(APPEND CLI "-o" "${OUTPUT_PATTERN}")
    
    set(OUTPUTS "")
    set(STAGES "")
    #stages
    foreach (TYPE ${ARGS_TYPES})
    if (TYPE STREQUAL "VERTEX")
      list(APPEND CLI "-s" "vs")
      list(APPEND STAGES "vs")
    elseif (TYPE STREQUAL "FRAGMENT")
      list(APPEND CLI "-s" "fs")
      list(APPEND STAGES "fs")
    elseif (TYPE STREQUAL "COMPUTE")
      list(APPEND CLI "-s" "cs")
      list(APPEND STAGES "cs")
    endif()
    endforeach()

    # targets
    foreach(TARGET ${TARGETS})
    _bgfx_slang_get_target_path_name(${TARGET} TARGET_PATH_NAME)
      string(REPLACE {{target}} ${TARGET_PATH_NAME} TARGET_OUTPUT_PATH ${OUTPUT_PATTERN})
      string(REPLACE {{name}} ${SHADER_FILE_NAME_WE} TARGET_OUTPUT_PATH ${TARGET_OUTPUT_PATH})
      string(REPLACE {{fileName}} ${SHADER_FILE_BASENAME} TARGET_OUTPUT_PATH ${TARGET_OUTPUT_PATH})
      list(APPEND CLI "-t" "${TARGET}")
      foreach(STAGE ${STAGES})
        string(REPLACE {{stage}} ${STAGE} STAGE_OUTPUT_PATH ${TARGET_OUTPUT_PATH})
        list(APPEND OUTPUTS ${STAGE_OUTPUT_PATH})
      endforeach()
    endforeach()

    # include directories
    if (ARGS_INCLUDE_DIRS)
      foreach(INCLUDE_DIR ${ARGS_INCLUDE_DIRS})
        list(APPEND CLI "-i" "${INCLUDE_DIR}")
      endforeach()
    endif()

    if (ARGS_VERBOSE)
      list(APPEND CLI "-v")
    endif()

    list(APPEND ALL_OUTPUTS ${OUTPUTS})

    add_custom_command(
      OUTPUT ${OUTPUTS}
      COMMAND ${BGFX_SLANG_CMD_EXECUTABLE} ${CLI}
      MAIN_DEPENDENCY ${SHADER_FILE_ABSOLUTE}
    )
  endforeach()

  if(DEFINED ARGS_OUT_FILES_VAR)
    set(${ARGS_OUT_FILES_VAR} ${ALL_OUTPUTS} PARENT_SCOPE)
  endif()
endfunction()