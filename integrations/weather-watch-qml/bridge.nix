# Isolated opt-in QML integration package. Does NOT install to the
# operator's machine or auto-load in Weather/Quickshell. Protocol XML is
# pulled from the SAME SwayPX source checkout.
let
  flake = builtins.getFlake (toString ../..);
  pkgs = flake.inputs.nixpkgs.legacyPackages.x86_64-linux;
in pkgs.stdenv.mkDerivation {
  pname = "post-apollo-stage-director-watch-qml";
  version = "0.1.0";
  src = ../..;
  strictDeps = true;
  dontWrapQtApps = true;
  nativeBuildInputs = [
    pkgs.cmake pkgs.ninja pkgs.pkg-config pkgs.wayland-scanner
  ];
  buildInputs = [
    pkgs.qt6.qtbase pkgs.qt6.qtdeclarative pkgs.qt6.qtwayland pkgs.wayland
  ];
  configurePhase = ''
    runHook preConfigure
    cmake -S integrations/weather-watch-qml -B build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$out"
    runHook postConfigure
  '';
  buildPhase = ''
    runHook preBuild
    cmake --build build --parallel 2
    runHook postBuild
  '';
  doCheck = true;
  checkPhase = ''
    runHook preCheck
    export XDG_CACHE_HOME="$TMPDIR/post-apollo-qt-cache"
    mkdir -p "$XDG_CACHE_HOME"
    # dontWrapQtApps is correct for this library. In the isolated test
    # process, explicitly expose Qt's standard QML import modules.
    qt_qml_dir="$(find ${pkgs.qt6.qtdeclarative} -type f -path '*/qml/QtQml/qmldir' -print -quit)"
    test -n "$qt_qml_dir"
    export QML_IMPORT_PATH="$(dirname "$(dirname "$qt_qml_dir")")"
    QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure -VV
    runHook postCheck
  '';
  installPhase = ''
    runHook preInstall
    cmake --install build
    module="$out/lib/qt6/qml/PostApollo/StageDirector"
    test -f "$module/qmldir"
    test -f "$module/libpostapollo_weather_watchplugin.so"
    test -f "$module/libpostapollo_weather_watch.so"
    # Validate the delivered package, not just the in-build-tree module.
    export QML_IMPORT_PATH="$out/lib/qt6/qml:$QML_IMPORT_PATH"
    QT_QPA_PLATFORM=offscreen build/watch_qml_offscreen_test "$out/lib/qt6/qml"
    echo "SD_WEATHER_QML_INSTALLED_IMPORT: packaged_plugin=1 backing_lib=1 default_off=1 PASS"
    runHook postInstall
  '';
}
