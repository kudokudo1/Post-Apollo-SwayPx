# CI-only actual Chrome compositor pixel crop diagnostic. This builds an
# independent opt-in wlroots probe. Normal SwayPX never links the probe,
# no physical workstation modification and no production WATCH authorization.
let
  flake = builtins.getFlake (toString ../..);
  system = "x86_64-linux";
in flake.packages.${system}.swayfx-unwrapped-git.overrideAttrs (old: {
  mesonFlags = (old.mesonFlags or []) ++ [
    "-Dsd-ci-news-scene-probe=true"
  ];
})
