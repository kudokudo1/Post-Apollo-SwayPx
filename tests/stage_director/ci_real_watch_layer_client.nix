# CI-only actual layer-shell client plus production WATCH protocol on the
# SAME wl_display. Uses pinned Nixpkgs, never operator's local machine.
let
  flake = builtins.getFlake (toString ../..);
  pkgs = flake.inputs.nixpkgs.legacyPackages.x86_64-linux;
in pkgs.stdenv.mkDerivation {
  pname = "stage-director-ci-real-watch-layer";
  version = "1";
  dontUnpack = true;
  strictDeps = true;
  nativeBuildInputs = [ pkgs.pkg-config pkgs.wayland-scanner ];
  buildInputs = [ pkgs.wayland pkgs.wayland-protocols ];
  buildPhase = ''
    runHook preBuild
    wayland-scanner client-header ${../../protocols/wlr-layer-shell-unstable-v1.xml} wlr-layer-shell-unstable-v1-client-protocol.h
    wayland-scanner private-code ${../../protocols/wlr-layer-shell-unstable-v1.xml} layer-protocol.c
    wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-protocol.c
    wayland-scanner client-header ${../../protocols/post-apollo-weather-watch-v1.xml} post-apollo-weather-watch-v1-client-protocol.h
    wayland-scanner private-code ${../../protocols/post-apollo-weather-watch-v1.xml} watch-protocol.c
    $CC -std=c11 -Wall -Wextra -Werror -pedantic -I. \
      ${./ci_real_watch_layer_client.c} layer-protocol.c \
      xdg-shell-protocol.c watch-protocol.c \
      $(pkg-config --cflags --libs wayland-client) \
      -o sd-ci-real-watch-layer
    runHook postBuild
  '';
  installPhase = ''
    runHook preInstall
    mkdir -p "$out/bin"
    cp sd-ci-real-watch-layer "$out/bin/sd-ci-real-watch-layer"
    runHook postInstall
  '';
}
