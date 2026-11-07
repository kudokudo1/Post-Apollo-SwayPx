# Build a real, isolated QtWayland QPA XDG client with SwayPX's pinned Nixpkgs.
# This does NOT use the operator-local MapLibre ELF and has no scene authority.
let
  flake = builtins.getFlake (toString ../..);
  pkgs = flake.inputs.nixpkgs.legacyPackages.x86_64-linux;
in pkgs.stdenv.mkDerivation {
  pname = "stage-director-company-m30d-real-qt-qpa-probe";
  version = "1";
  dontUnpack = true;
  strictDeps = true;
  nativeBuildInputs = [ pkgs.pkg-config pkgs.qt6.wrapQtAppsHook ];
  buildInputs = [ pkgs.qt6.qtbase pkgs.qt6.qtwayland pkgs.wayland ];
  buildPhase = ''
    runHook preBuild
    $CXX -std=c++17 -O2 -Wall -Wextra -Werror -pedantic \
      ${./ci_company_qt_wayland_probe.cpp} \
      -o sd-company-qt-wayland-probe \
      $(pkg-config --cflags --libs Qt6Widgets)
    runHook postBuild
  '';
  installPhase = ''
    runHook preInstall
    mkdir -p "$out/bin"
    cp sd-company-qt-wayland-probe "$out/bin/"
    runHook postInstall
  '';
}
