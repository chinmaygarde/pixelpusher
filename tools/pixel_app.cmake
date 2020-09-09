
if(__pixel_app)
  return()
endif()
set(__pixel_app INCLUDED)

function(pixel_app TARGET)

  add_custom_target(pixel_app_${TARGET} ALL)

  add_custom_target(pixel_app_snapshot_${TARGET}
    COMMENT "Snapshotting Dart for ${TARGET}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${ARGN} .packages
    COMMAND ${FLUTTER_ENGINE_DIR}/dart
              --disable-dart-dev
              ${FLUTTER_ENGINE_DIR}/frontend_server.dart.snapshot
              --sdk-root ${FLUTTER_ENGINE_DIR}/flutter_patched_sdk/
              --target=flutter
              --packages .packages
              -Ddart.vm.profile=false
              -Ddart.vm.product=false
              --enable-asserts
              --output-dill ${CMAKE_CURRENT_BINARY_DIR}/kernel_blob.bin
              package:${TARGET}/main.dart
  )

  add_dependencies(pixel_app_${TARGET} pixel_app_snapshot_${TARGET})

  add_custom_target(pixel_app_pub_${TARGET}
    COMMENT "Running 'pub get' ${TARGET}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${ARGN}
    COMMAND ${FLUTTER_ENGINE_DIR}/dart pub get
  )

  add_dependencies(pixel_app_snapshot_${TARGET} pixel_app_pub_${TARGET})

endfunction()
