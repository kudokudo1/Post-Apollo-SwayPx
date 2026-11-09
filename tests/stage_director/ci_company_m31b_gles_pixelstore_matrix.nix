# M31b standalone GLES3 test program built from the pinned Nixpkgs tree.
# No SceneFX, SwayPX, Company Qt or production package is modified.
let
  flake = builtins.getFlake (toString ../..);
  pkgs = flake.inputs.nixpkgs.legacyPackages.x86_64-linux;
in pkgs.stdenv.mkDerivation {
  pname = "company-m31b-ci-gl-pixelstore-matrix";
  version = "1.0.0";
  dontUnpack = true;
  strictDeps = true;
  nativeBuildInputs = [ pkgs.pkg-config ];
  buildInputs = [ pkgs.libGL pkgs.mesa ];
  buildPhase = ''
    runHook preBuild
    "$CC" -std=c11 -O2 -Wall -Wextra -Werror \
      ${./ci_company_m31b_gles_pixelstore_matrix.c} \
      -o company-m31b-gles-pixelstore-matrix \
      $(pkg-config --cflags --libs egl glesv2)
    runHook postBuild
  '';
  installPhase = ''
    runHook preInstall
    install -Dm755 company-m31b-gles-pixelstore-matrix \
      "$out/bin/company-m31b-gles-pixelstore-matrix"
    runHook postInstall
  '';
}
