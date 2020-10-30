import 'dart:ffi' as ffi;
import './gen/dart/pixel.dart' as pixel;

main(List<String> args) {
  pixel.AttachNativeBindings(ffi.DynamicLibrary.executable());

  var scene = pixel.SceneCreate();
  var application = pixel.ApplicationGetMain();

  if (pixel.ApplicationSetScene(application, scene) != pixel.CResult.kSuccess) {
    throw ("Could not set scene for main renderer.");
  }

  pixel.SceneCollect(scene);
}
