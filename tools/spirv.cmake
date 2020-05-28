if(__spirv)
  return()
endif()
set(__spirv INCLUDED)

function(compile_shader TARGET SHADER_PATH)

  get_filename_component(SHADER_NAME ${SHADER_PATH} NAME)

  get_filename_component(SHADER_IN_PATH ${SHADER_PATH} ABSOLUTE)
  set(SHADER_OUT_PATH ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.spv)
  set(SHADER_ORIG_OUT_PATH ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME})
  get_filename_component(SHADER_OUT_DIR ${SHADER_OUT_PATH} DIRECTORY)

  file(MAKE_DIRECTORY ${SHADER_OUT_DIR})
  file(CREATE_LINK ${SHADER_IN_PATH} ${SHADER_ORIG_OUT_PATH})

  add_custom_command(
    OUTPUT ${SHADER_OUT_PATH}
    COMMAND glslc_exe ${SHADER_IN_PATH} -o ${SHADER_OUT_PATH}
    DEPENDS ${SHADER_IN_PATH}
    IMPLICIT_DEPENDS CXX ${SHADER_IN_PATH}
  )

  set_source_files_properties(${SHADER_OUT_PATH} PROPERTIES GENERATED TRUE)
  target_sources(${TARGET}
    PRIVATE
      ${SHADER_ORIG_OUT_PATH}
      ${SHADER_OUT_PATH}
  )

endfunction()


function(compile_shaders TARGET)
  foreach(shader ${ARGN})
    compile_shader(${TARGET} ${shader})
  endforeach()
endfunction()
