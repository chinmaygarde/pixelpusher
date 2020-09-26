import 'dart:ffi' as ffi;
import './gen/dart/pixel.dart' as pixel;

main(List<String> args) {
  pixel.AttachNativeBindings(ffi.DynamicLibrary.executable());
  print("Pixel Runtime Initialized.");

  var scene = pixel.SceneCreate();
  pixel.SceneCollect(scene);
}
