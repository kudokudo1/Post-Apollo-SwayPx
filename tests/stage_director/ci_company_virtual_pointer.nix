# Isolated real wlroots virtual pointer sender, pinned to this SwayPX flake.
# NOT installed and NOT invoked outside disposable GitHub CI.
let
  flake = builtins.getFlake (toString ../..);
  pkgs = flake.inputs.nixpkgs.legacyPackages.x86_64-linux;
in pkgs.stdenv.mkDerivation {
  pname = "sd-company-m30h-virtual-pointer";
  version = "1";
  dontUnpack = true;
  strictDeps = true;
  nativeBuildInputs = [ pkgs.pkg-config pkgs.wayland-scanner ];
  buildInputs = [ pkgs.wayland pkgs.wlr-protocols ];
  buildPhase = ''
    runHook preBuild
    xml="$(find ${pkgs.wlr-protocols}/share -name wlr-virtual-pointer-unstable-v1.xml -type f -print -quit)"
    test -n "$xml"
    wayland-scanner client-header "$xml" wlr-virtual-pointer-unstable-v1-client-protocol.h
    wayland-scanner private-code "$xml" wlr-virtual-pointer-unstable-v1-protocol.c
    $CC -std=c11 -O2 -Wall -Wextra -Werror -pedantic -I. \
      ${./ci_company_virtual_pointer.c} \
      wlr-virtual-pointer-unstable-v1-protocol.c \
      -o sd-company-m30h-pointer \
      $(pkg-config --cflags --libs wayland-client)
    runHook postBuild
  '';
  installPhase = ''
    runHook preInstall
    mkdir -p "$out/bin"
    cp sd-company-m30h-pointer "$out/bin/"
    runHook postInstall
  '';
}
