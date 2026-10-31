# CI-only Qt Wayland native display probe, NOT a Quickshell plugin.
# Builds a real Qt6 QGuiApplication that owns the mapped Weather layer and
# native WATCH client on exactly the same borrowed QPA wl_display.
let
  flake = builtins.getFlake (toString ../..);
  pkgs = flake.inputs.nixpkgs.legacyPackages.x86_64-linux;
in pkgs.stdenv.mkDerivation {
  pname = "stage-director-ci-qt-native-watch-bridge";
  version = "1";
  dontUnpack = true;
  strictDeps = true;
  nativeBuildInputs = [
    pkgs.pkg-config
    pkgs.wayland-scanner
    pkgs.qt6.wrapQtAppsHook
  ];
  buildInputs = [
    pkgs.qt6.qtbase
    pkgs.qt6.qtwayland
    pkgs.wayland
    pkgs.wayland-protocols
  ];
  buildPhase = ''
    runHook preBuild
    wayland-scanner client-header ${../../protocols/wlr-layer-shell-unstable-v1.xml} wlr-layer-shell-unstable-v1-client-protocol.h
    wayland-scanner private-code ${../../protocols/wlr-layer-shell-unstable-v1.xml} layer-protocol.c
    wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-protocol.c
    wayland-scanner client-header ${../../protocols/post-apollo-weather-watch-v1.xml} post-apollo-weather-watch-v1-client-protocol.h
    wayland-scanner private-code ${../../protocols/post-apollo-weather-watch-v1.xml} watch-protocol.c
    for source in layer-protocol.c xdg-shell-protocol.c watch-protocol.c; do
      $CC -std=c11 -Wall -Wextra -Werror -pedantic -I. \
        $(pkg-config --cflags wayland-client) -c "$source" -o "''${source%.c}.o"
    done
    $CXX -std=c++17 -Wall -Wextra -Werror -pedantic -I. \
      $(pkg-config --cflags Qt6Gui wayland-client) \
      ${./ci_qt_native_watch_bridge.cpp} \
      layer-protocol.o xdg-shell-protocol.o watch-protocol.o \
      $(pkg-config --libs Qt6Gui wayland-client) \
      -o sd-ci-qt-native-watch-bridge
    runHook postBuild
  '';
  installPhase = ''
    runHook preInstall
    mkdir -p "$out/bin"
    cp sd-ci-qt-native-watch-bridge "$out/bin/sd-ci-qt-native-watch-bridge"
    runHook postInstall
  '';
}
