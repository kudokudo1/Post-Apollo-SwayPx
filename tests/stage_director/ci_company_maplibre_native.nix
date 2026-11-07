# M30l: real Company MapLibre v3 QtLocation app built in disposable Nix CI.
# `source` is an exact-pinned temporary COPY, never the live Company repo.
{ source }:
let
  flake = builtins.getFlake (toString ../..);
  pkgs = flake.inputs.nixpkgs.legacyPackages.x86_64-linux;
  qt = pkgs.kdePackages;
in
pkgs.stdenv.mkDerivation {
  pname = "stage-director-company-m30l-native-maplibre-probe";
  version = "3.0.0-ci1";
  src = source;
  strictDeps = true;
  nativeBuildInputs = [ pkgs.cmake pkgs.ninja pkgs.pkg-config pkgs.qt6.wrapQtAppsHook ];
  buildInputs = [
    qt.qtbase qt.qtdeclarative qt.qtlocation qt.qtpositioning
    qt.qtwayland qt.maplibre-native-qt
  ];
  postPatch = ''
    substituteInPlace CMakeLists.txt \
      --replace-fail '''${CMAKE_CURRENT_SOURCE_DIR}/style-fixture.json' \
        "$out/share/post-apollo-company-m30l/style-fixture.json" \
      --replace-fail '''${CMAKE_CURRENT_SOURCE_DIR}/style-post-apollo-online.json' \
        "$out/share/post-apollo-company-m30l/style-post-apollo-online.json" \
      --replace-fail '''${CMAKE_CURRENT_SOURCE_DIR}/fixture-hubs.json' \
        "$out/share/post-apollo-company-m30l/fixture-hubs.json" \
      --replace-fail '''${CMAKE_CURRENT_SOURCE_DIR}/../../services/company/CompanyHubMapReadOnly.py' \
        "$out/share/post-apollo-company-m30l/NO_OWNER_LOCAL_DATA"
  '';
  cmakeFlags = [ "-DCMAKE_BUILD_TYPE=Release" ];
  installPhase = ''
    runHook preInstall
    install -Dm755 company-vector-probe "$out/bin/company-vector-probe"
    install -Dm644 "$src/style-fixture.json" \
      "$out/share/post-apollo-company-m30l/style-fixture.json"
    install -Dm644 "$src/style-post-apollo-online.json" \
      "$out/share/post-apollo-company-m30l/style-post-apollo-online.json"
    install -Dm644 "$src/fixture-hubs.json" \
      "$out/share/post-apollo-company-m30l/fixture-hubs.json"
    runHook postInstall
  '';
  meta.description = "CI-only exact Company MapLibre v3 QtLocation executable";
}
