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
    QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
    runHook postCheck
  '';
  installPhase = ''
    runHook preInstall
    cmake --install build
    test -f "$out/lib/qt6/qml/PostApollo/StageDirector/qmldir"
    runHook postInstall
  '';
}
