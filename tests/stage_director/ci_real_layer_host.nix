# CI-only Wayland client compiled with pinned source protocol XML and Nixpkgs.
# NO installation on the operator's desktop; no compositor/browser launch.
let
  flake = builtins.getFlake (toString ../..);
  pkgs = flake.inputs.nixpkgs.legacyPackages.x86_64-linux;
in pkgs.stdenv.mkDerivation {
  pname = "stage-director-ci-real-layer-host";
  version = "1";
  dontUnpack = true;
  strictDeps = true;
  nativeBuildInputs = [ pkgs.pkg-config pkgs.wayland-scanner ];
  buildInputs = [ pkgs.wayland pkgs.wayland-protocols ];
  buildPhase = ''
    runHook preBuild
    wayland-scanner client-header ${../../protocols/wlr-layer-shell-unstable-v1.xml} wlr-layer-shell-unstable-v1-client-protocol.h
    wayland-scanner private-code ${../../protocols/wlr-layer-shell-unstable-v1.xml} layer-protocol.c
    # Generated layer-shell interface metadata references xdg_popup even when
    # no popup is created; supply the pinned official XDG protocol metadata.
    wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-protocol.c
    $CC -std=c11 -Wall -Wextra -Werror -pedantic -I. \
      ${./ci_real_layer_host.c} layer-protocol.c xdg-shell-protocol.c \
      $(pkg-config --cflags --libs wayland-client) \
      -o sd-ci-real-layer-host
    runHook postBuild
  '';
  installPhase = ''
    runHook preInstall
    mkdir -p "$out/bin"
    cp sd-ci-real-layer-host "$out/bin/sd-ci-real-layer-host"
    runHook postInstall
  '';
}