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
  # Original CMake style-file paths were changed to a dedicated marker
  # in a disposable source copy. Avoid Nix/CMake interpolation collisions.
  postPatch = ''
    substituteInPlace CMakeLists.txt \
      --replace-fail 'M30L_NIX_OUTPUT_ROOT' "$out"
  '';
  cmakeFlags = [ "-DCMAKE_BUILD_TYPE=Release" ];
  # MapLibre v3 installs its QML module and QtLocation provider in package-
  # relative qml/ and plugins/, not in Qt's lib/qt-6/qml tree. The normal Qt
  # wrapper does not infer those custom locations from a linked library.
  # Pin them to the same Nix MapLibre derivation used to build this exact app.
  preFixup = ''
    test -f "${qt.maplibre-native-qt}/qml/MapLibre/qmldir" || {
      echo "M30L_MAPLIBRE_QML_MODULE_NOT_INSTALLED" >&2
      exit 1
    }
    test -d "${qt.maplibre-native-qt}/plugins/geoservices" || {
      echo "M30L_MAPLIBRE_GEOSERVICES_PLUGIN_NOT_INSTALLED" >&2
      exit 1
    }
    qtWrapperArgs+=(
      --prefix QML_IMPORT_PATH : "${qt.maplibre-native-qt}/qml"
      --prefix QT_PLUGIN_PATH : "${qt.maplibre-native-qt}/plugins"
    )
  '';
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
