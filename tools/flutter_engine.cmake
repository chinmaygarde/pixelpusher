
if(__flutter_engine)
  return()
endif()
set(__flutter_engine INCLUDED)


function(depend_on_flutter_engine TARGET)
  add_library("flutter_engine_${TARGET}" SHARED IMPORTED)

  if(APPLE)
    set(FLUTTER_ENGINE_DYLIB   "${FLUTTER_ENGINE_DIR}/libflutter_engine.dylib")

    set_target_properties("flutter_engine_${TARGET}" PROPERTIES
        IMPORTED_LOCATION   "${FLUTTER_ENGINE_DYLIB}"
    )
  else()
    set(FLUTTER_ENGINE_DLL     "${FLUTTER_ENGINE_DIR}/flutter_engine.dll")
    set(FLUTTER_ENGINE_LIB     "${FLUTTER_ENGINE_DIR}/flutter_engine.dll.lib")

    set_target_properties("flutter_engine_${TARGET}" PROPERTIES
        IMPORTED_LOCATION   "${FLUTTER_ENGINE_DLL}"
        IMPORTED_IMPLIB     "${FLUTTER_ENGINE_LIB}"
    )
  endif()

  target_include_directories("flutter_engine_${TARGET}" INTERFACE
    "${FLUTTER_ENGINE_DIR}"
  )

  target_link_libraries(${TARGET} PRIVATE "flutter_engine_${TARGET}")
endfunction()

function(copy_flutter_engine_resources TARGET)
  if(APPLE)
    add_custom_target("flutter_engine_resources_${TARGET}"
      COMMENT "Copy Engine Resources for ${TARGET}"
      COMMAND "${CMAKE_COMMAND}" -E copy "${FLUTTER_ENGINE_DIR}/libflutter_engine.dylib" "${CMAKE_CURRENT_BINARY_DIR}"
      COMMAND "${CMAKE_COMMAND}" -E copy "${FLUTTER_ENGINE_DIR}/icudtl.dat"              "${CMAKE_CURRENT_BINARY_DIR}"
      DEPENDS
        "${FLUTTER_ENGINE_DIR}/libflutter_engine.dylib"
        "${FLUTTER_ENGINE_DIR}/icudtl.dat"
    )
  else()
    add_custom_target("flutter_engine_resources_${TARGET}"
      COMMENT "Copy Engine Resources for ${TARGET}"
      COMMAND "${CMAKE_COMMAND}" -E copy "${FLUTTER_ENGINE_DIR}/flutter_engine.dll"     "${CMAKE_CURRENT_BINARY_DIR}"
      COMMAND "${CMAKE_COMMAND}" -E copy "${FLUTTER_ENGINE_DIR}/flutter_engine.dll.lib" "${CMAKE_CURRENT_BINARY_DIR}"
      COMMAND "${CMAKE_COMMAND}" -E copy "${FLUTTER_ENGINE_DIR}/icudtl.dat"             "${CMAKE_CURRENT_BINARY_DIR}"
      DEPENDS
        "${FLUTTER_ENGINE_DIR}/flutter_engine.dll"
        "${FLUTTER_ENGINE_DIR}/flutter_engine.dll.lib"
        "${FLUTTER_ENGINE_DIR}/icudtl.dat"
    )
  endif()

  add_dependencies(${TARGET} "flutter_engine_resources_${TARGET}")
endfunction()
