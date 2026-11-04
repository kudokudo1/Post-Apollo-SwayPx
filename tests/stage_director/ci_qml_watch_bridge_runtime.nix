# Explicitly isolated Qt GUI + actual installed opt-in QML WATCH module.
# Builds the mapped Weather layer fixture with an alternate QML-only peer.
# No browser spawn, production pairing, screen changes or local deployment.
let
  flake = builtins.getFlake (toString ../..);
  pkgs = flake.inputs.nixpkgs.legacyPackages.x86_64-linux;
  qmlBridge = import ../../integrations/weather-watch-qml/bridge.nix;
in pkgs.stdenv.mkDerivation {
  pname = "stage-director-ci-live-qt-qml-watch-bridge";
  version = "1";
  dontUnpack = true;
  strictDeps = true;
  nativeBuildInputs = [
    pkgs.pkg-config pkgs.wayland-scanner pkgs.qt6.wrapQtAppsHook
  ];
  buildInputs = [
    pkgs.qt6.qtbase pkgs.qt6.qtdeclarative pkgs.qt6.qtwayland
    pkgs.wayland pkgs.wayland-protocols qmlBridge
  ];
  buildPhase = ''
    runHook preBuild
    wayland-scanner client-header ${../../protocols/wlr-layer-shell-unstable-v1.xml} wlr-layer-shell-unstable-v1-client-protocol.h
    # C scanner's "namespace" parameter is a C++ keyword. The C ABI and
    # canonical XML remain unchanged; only this fixture header is renamed.
    sed -i 's/namespace/namespace_name/g' wlr-layer-shell-unstable-v1-client-protocol.h
    grep -Fq 'const char *namespace_name)' wlr-layer-shell-unstable-v1-client-protocol.h
    wayland-scanner private-code ${../../protocols/wlr-layer-shell-unstable-v1.xml} layer-protocol.c
    wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-protocol.c
    wayland-scanner client-header ${../../protocols/post-apollo-weather-watch-v1.xml} post-apollo-weather-watch-v1-client-protocol.h
    wayland-scanner private-code ${../../protocols/post-apollo-weather-watch-v1.xml} watch-protocol.c
    for source in layer-protocol.c xdg-shell-protocol.c watch-protocol.c; do
      $CC -std=c11 -Wall -Wextra -Werror -pedantic -I. \
        $(pkg-config --cflags wayland-client) -c "$source" -o "''${source%.c}.o"
    done
    $CXX -std=c++17 -Wall -Wextra -Werror -pedantic -DSD_CI_QML_WATCH_BRIDGE=1 \
      -I. $(pkg-config --cflags Qt6Gui Qt6Qml wayland-client) \
      ${./ci_qt_native_watch_bridge.cpp} \
      layer-protocol.o xdg-shell-protocol.o watch-protocol.o \
      $(pkg-config --libs Qt6Gui Qt6Qml wayland-client) \
      -o sd-ci-qt-qml-watch-bridge
    runHook postBuild
  '';
  installPhase = ''
    runHook preInstall
    mkdir -p "$out/bin"
    cp sd-ci-qt-qml-watch-bridge "$out/bin/sd-ci-qt-qml-watch-bridge"
    runHook postInstall
  '';
  passthru.qmlBridge = qmlBridge;
}
