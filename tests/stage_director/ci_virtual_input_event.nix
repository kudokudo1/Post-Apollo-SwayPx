# CI-only Wayland virtual key/pointer event witness.
# Pinned by the repo's existing flake nixpkgs. No workstation installation.
let
  flake = builtins.getFlake (toString ../..);
  pkgs = flake.inputs.nixpkgs.legacyPackages.x86_64-linux;
in pkgs.stdenv.mkDerivation {
  pname = "stage-director-ci-virtual-input-event";
  version = "1";
  dontUnpack = true;
  strictDeps = true;
  nativeBuildInputs = [ pkgs.pkg-config pkgs.wayland-scanner ];
  buildInputs = [ pkgs.wayland pkgs.libxkbcommon ];
  buildPhase = ''
    runHook preBuild
    wayland-scanner client-header ${./protocols/virtual-keyboard-unstable-v1.xml} virtual-keyboard-unstable-v1-client-protocol.h
    wayland-scanner private-code ${./protocols/virtual-keyboard-unstable-v1.xml} virtual-keyboard-protocol.c
    wayland-scanner client-header ${./protocols/wlr-virtual-pointer-unstable-v1.xml} wlr-virtual-pointer-unstable-v1-client-protocol.h
    wayland-scanner private-code ${./protocols/wlr-virtual-pointer-unstable-v1.xml} virtual-pointer-protocol.c
    $CC -std=c11 -Wall -Wextra -Werror -pedantic -I. \
      ${./ci_virtual_input_event.c} virtual-keyboard-protocol.c virtual-pointer-protocol.c \
      $(pkg-config --cflags --libs wayland-client xkbcommon) \
      -o sd-ci-virtual-input-event
    runHook postBuild
  '';
  installPhase = ''
    runHook preInstall
    mkdir -p "$out/bin"
    cp sd-ci-virtual-input-event "$out/bin/sd-ci-virtual-input-event"
    runHook postInstall
  '';
}
