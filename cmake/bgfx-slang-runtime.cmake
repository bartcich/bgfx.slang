function(bgfx_slang_copy_runtime TARGET)
  if (NOT TARGET "${TARGET}")
    message(FATAL_ERROR "bgfx_slang_copy_runtime: target '${TARGET}' does not exist")
  endif()

  if (WIN32)
    add_custom_command(TARGET ${TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${TARGET}> "$<TARGET_FILE_DIR:${TARGET}>"
      COMMAND_EXPAND_LISTS
      VERBATIM
    )
  endif()

  add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
      "-DSLANG_RUNTIME_DIR=$<TARGET_FILE_DIR:slang::slang>"
      "-DOUTPUT_DIR=$<TARGET_FILE_DIR:${TARGET}>"
      -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/CopySlangModules.cmake"
    VERBATIM
  )
endfunction()
