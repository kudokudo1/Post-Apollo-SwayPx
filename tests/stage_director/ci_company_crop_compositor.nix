# M30g: separate Qt-source crop compositor. Normal SwayPX is unchanged.
# No selected Company host or production render authority.
let
  flake = builtins.getFlake (toString ../..);
  system = "x86_64-linux";
in flake.packages.${system}.swayfx-unwrapped-git.overrideAttrs (old: {
  mesonFlags = (old.mesonFlags or []) ++ [
    "-Dsd-ci-company-crop-probe=true"
  ];
})
