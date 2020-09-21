if(__epoxy)
  return()
endif()
set(__epoxy INCLUDED)

function(epoxy TARGET EPOXY_TEMPLATE_PATH EPOXY_IDL_PATH OUTPUT_FILE_NAME)
  get_filename_component(EPOXY_TEMPLATE_PATH ${EPOXY_TEMPLATE_PATH} ABSOLUTE)
  get_filename_component(EPOXY_IDL_PATH ${EPOXY_IDL_PATH} ABSOLUTE)

  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gen")

  set(OUTPUT_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/gen/${OUTPUT_FILE_NAME}")

  add_custom_command(
    COMMENT "Generating Epoxy Interface for ${TARGET}"
    OUTPUT "${OUTPUT_FILE_PATH}"
    COMMAND epoxy
      --output "${OUTPUT_FILE_PATH}"
      --idl "${EPOXY_IDL_PATH}"
      --template-file "${EPOXY_TEMPLATE_PATH}"
    DEPENDS "${EPOXY_IDL_PATH}" "${EPOXY_TEMPLATE_PATH}"
  )

  target_sources(${TARGET} PUBLIC "${OUTPUT_FILE_PATH}")

  target_include_directories(${TARGET} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/gen")
endfunction()


function(epoxy_dart_resource TARGET EPOXY_TEMPLATE_PATH EPOXY_IDL_PATH OUTPUT_FILE_NAME)
  get_filename_component(EPOXY_TEMPLATE_PATH ${EPOXY_TEMPLATE_PATH} ABSOLUTE)
  get_filename_component(EPOXY_IDL_PATH ${EPOXY_IDL_PATH} ABSOLUTE)

  file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/gen/dart")

  set(OUTPUT_DART_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/gen/dart/${OUTPUT_FILE_NAME}")

  add_custom_target(
    "${TARGET}_epoxy_dart_resource"
    COMMENT "Generating Epoxy Dart Interface for ${TARGET}"
    OUTPUT "${OUTPUT_DART_FILE_PATH}"
    COMMAND epoxy
      --output "${OUTPUT_DART_FILE_PATH}"
      --idl "${EPOXY_IDL_PATH}"
      --template-file "${EPOXY_TEMPLATE_PATH}"
    DEPENDS "${EPOXY_IDL_PATH}" "${EPOXY_TEMPLATE_PATH}"
  )

  add_dependencies(${TARGET} ${TARGET}_epoxy_dart_resource)
endfunction()
