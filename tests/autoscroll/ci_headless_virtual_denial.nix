# CI-only native Wayland observer and virtual-pointer source.
# Uses the repo's locked Nixpkgs; nothing is installed on the user host.
let
  flake = builtins.getFlake (toString ../..);
  pkgs = flake.inputs.nixpkgs.legacyPackages.x86_64-linux;
in pkgs.stdenv.mkDerivation {
  pname = "pa-ci-autoscroll-virtual-origin-negative";
  version = "1";
  dontUnpack = true;
  strictDeps = true;
  nativeBuildInputs = [ pkgs.pkg-config pkgs.wayland-scanner ];
  buildInputs = [ pkgs.wayland pkgs.wayland-protocols ];
  buildPhase = ''
    runHook preBuild
    wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.h
    wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-protocol.c
    wayland-scanner client-header ${../stage_director/protocols/wlr-virtual-pointer-unstable-v1.xml} wlr-virtual-pointer-unstable-v1-client-protocol.h
    wayland-scanner private-code ${../stage_director/protocols/wlr-virtual-pointer-unstable-v1.xml} wlr-virtual-pointer-protocol.c
    $CC -std=c11 -Wall -Wextra -Werror -pedantic -I. \
      ${./ci_headless_pointer_observer.c} xdg-shell-protocol.c \
      $(pkg-config --cflags --libs wayland-client) \
      -o pa-ci-autoscroll-observer
    $CC -std=c11 -Wall -Wextra -Werror -pedantic -I. \
      ${./ci_headless_virtual_middle.c} wlr-virtual-pointer-protocol.c \
      $(pkg-config --cflags --libs wayland-client) \
      -o pa-ci-virtual-middle
    runHook postBuild
  '';
  installPhase = ''
    runHook preInstall
    mkdir -p "$out/bin"
    cp pa-ci-autoscroll-observer pa-ci-virtual-middle "$out/bin/"
    runHook postInstall
  '';
}
