
if(__flutter_engine)
  return()
endif()
set(__flutter_engine INCLUDED)


function(depend_on_flutter_engine TARGET)
  set(FLUTTER_ENGINE_DIR "C:/Users/chinm/VersionControlled/engine/src/out/host_debug")

  add_library("flutter_engine_${TARGET}" SHARED IMPORTED)

  set(FLUTTER_ENGINE_DLL     "${FLUTTER_ENGINE_DIR}/flutter_engine.dll")
  set(FLUTTER_ENGINE_LIB     "${FLUTTER_ENGINE_DIR}/flutter_engine.dll.lib")

  set_target_properties("flutter_engine_${TARGET}" PROPERTIES
      IMPORTED_LOCATION   "${FLUTTER_ENGINE_DLL}"
      IMPORTED_IMPLIB     "${FLUTTER_ENGINE_LIB}"
  )

  target_include_directories("flutter_engine_${TARGET}" INTERFACE
    "${FLUTTER_ENGINE_DIR}"
  )

  target_link_libraries(${TARGET} PRIVATE "flutter_engine_${TARGET}")
endfunction()

function(copy_flutter_engine_resources TARGET)
  set(FLUTTER_ENGINE_DIR "C:/Users/chinm/VersionControlled/engine/src/out/host_debug")

  add_custom_target("flutter_engine_resources_${TARGET}"
    COMMAND "${CMAKE_COMMAND}" -E copy "${FLUTTER_ENGINE_DIR}/flutter_engine.dll"     "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy "${FLUTTER_ENGINE_DIR}/flutter_engine.dll.lib" "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy "${FLUTTER_ENGINE_DIR}/icudtl.dat"             "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS
      "${FLUTTER_ENGINE_DIR}/flutter_engine.dll"
      "${FLUTTER_ENGINE_DIR}/flutter_engine.dll.lib"
      "${FLUTTER_ENGINE_DIR}/icudtl.dat"
  )

  add_dependencies(${TARGET} "flutter_engine_resources_${TARGET}")
endfunction()
