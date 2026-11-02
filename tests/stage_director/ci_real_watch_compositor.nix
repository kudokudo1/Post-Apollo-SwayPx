# Isolated CI-only SwayPX override; no ordinary production changes.
let
  flake = builtins.getFlake (toString ../..);
in flake.packages.x86_64-linux.swayfx-unwrapped-git.overrideAttrs (old: {
  mesonFlags = (old.mesonFlags or []) ++ [
    "-Dsd-ci-real-weather-watch-layer=true"
  ];
})
