# Disposable CI-only sender: actual V1/V2/V3 engine -> VIRTUAL pointer.
# This cannot prove a physical/libinput event or activate SwayPX autoscroll.
let
  flake = builtins.getFlake (toString ../..);
  pkgs = flake.inputs.nixpkgs.legacyPackages.x86_64-linux;
in pkgs.stdenv.mkDerivation {
  pname = "pa-ci-autoscroll-engine-virtual-sender";
  version = "1";
  dontUnpack = true;
  strictDeps = true;
  nativeBuildInputs = [ pkgs.pkg-config pkgs.wayland-scanner ];
  buildInputs = [ pkgs.wayland ];
  buildPhase = ''
    runHook preBuild
    wayland-scanner client-header ${../stage_director/protocols/wlr-virtual-pointer-unstable-v1.xml} wlr-virtual-pointer-unstable-v1-client-protocol.h
    wayland-scanner private-code ${../stage_director/protocols/wlr-virtual-pointer-unstable-v1.xml} wlr-virtual-pointer-protocol.c
    $CC -std=c11 -Wall -Wextra -Werror -pedantic \
      -I. -I${../..}/include \
      ${./ci_engine_virtual_sender.c} \
      ${../../sway/input/post_apollo_autoscroll.c} \
      ${../../sway/input/post_apollo_autoscroll_adapter.c} \
      ${../../sway/input/post_apollo_autoscroll_axis_dispatch.c} \
      wlr-virtual-pointer-protocol.c \
      $(pkg-config --cflags --libs wayland-client) -lm \
      -o pa-ci-autoscroll-engine-virtual
    runHook postBuild
  '';
  installPhase = ''
    runHook preInstall
    mkdir -p "$out/bin"
    cp pa-ci-autoscroll-engine-virtual "$out/bin/"
    runHook postInstall
  '';
}
