pixel_app(runtime
  pubspec.yaml
  lib/main.dart
)

epoxy_dart_resource(
  runtime
  ../../idl/dart.template.epoxy
  ../../source/machine/pixel.epoxy
  pixel.dart
)
