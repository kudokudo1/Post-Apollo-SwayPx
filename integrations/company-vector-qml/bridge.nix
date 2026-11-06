# Isolated Company-only QML client package, off by default. NOT installed
# to operator Fedora and never auto-loaded into Quickshell.
let
  flake = builtins.getFlake (toString ../..);
  pkgs = flake.inputs.nixpkgs.legacyPackages.x86_64-linux;
in pkgs.stdenv.mkDerivation {
  pname = "post-apollo-stage-director-company-vector-qml";
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
    cmake -S integrations/company-vector-qml -B build -G Ninja \
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
    export XDG_CACHE_HOME="$TMPDIR/post-apollo-company-qt-cache"
    mkdir -p "$XDG_CACHE_HOME"
    qt_qml_dir="$(find ${pkgs.qt6.qtdeclarative} -type f -path '*/qml/QtQml/qmldir' -print -quit)"
    test -n "$qt_qml_dir"
    export QML_IMPORT_PATH="$(dirname "$(dirname "$qt_qml_dir")")"
    QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure -VV
    runHook postCheck
  '';
  installPhase = ''
    runHook preInstall
    cmake --install build
    module="$out/lib/qt6/qml/PostApollo/CompanyVector"
    test -f "$module/qmldir"
    test -f "$module/libpostapollo_company_vectorplugin.so"
    test -f "$module/libpostapollo_company_vector.so"
    export QML_IMPORT_PATH="$out/lib/qt6/qml:$QML_IMPORT_PATH"
    QT_QPA_PLATFORM=offscreen build/company_vector_qml_disabled_test \
      "$out/lib/qt6/qml"
    echo "SD_COMPANY_M25_QML_PACKAGE: module=1 default_off=1 no_second_display=1 PASS"
    runHook postInstall
  '';
}
