# CI-only native XDG popup witness. Pinned Wayland/xdg-shell, not installed.
let
  flake = builtins.getFlake (toString ../..);
  pkgs = flake.inputs.nixpkgs.legacyPackages.x86_64-linux;
in pkgs.stdenv.mkDerivation {
  pname = "stage-director-ci-xdg-popup-witness";
  version = "1";
  dontUnpack = true;
  strictDeps = true;
  nativeBuildInputs = [ pkgs.pkg-config pkgs.wayland-scanner ];
  buildInputs = [ pkgs.wayland pkgs.wayland-protocols ];
  buildPhase = ''
    runHook preBuild
    wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.h
    wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-protocol.c
    $CC -std=c11 -Wall -Wextra -Werror -pedantic -I. \
      ${./ci_xdg_popup_witness.c} xdg-shell-protocol.c \
      $(pkg-config --cflags --libs wayland-client) \
      -o sd-ci-xdg-popup-witness
    runHook postBuild
  '';
  installPhase = ''
    runHook preInstall
    mkdir -p "$out/bin"
    cp sd-ci-xdg-popup-witness "$out/bin/sd-ci-xdg-popup-witness"
    runHook postInstall
  '';
}
