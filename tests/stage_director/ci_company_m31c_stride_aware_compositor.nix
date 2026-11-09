# Company M31c // disposable SceneFX GL_UNPACK_ALIGNMENT control build.
# No production SwayPX/SceneFX/flake.lock change. This only replaces one
# pinned SceneFX derivation in a separately compiled Company CI compositor.
# The original unmodified CI compositor remains the negative control.
let
  flake = builtins.getFlake (toString ../..);
  system = "x86_64-linux";
  pkgs = flake.inputs.nixpkgs.legacyPackages.${system};
  upstreamFx = flake.inputs.scenefx.packages.${system}.scenefx-git;
  originalSway = flake.packages.${system}.swayfx-unwrapped-git;

  # Override ONLY the SceneFX source COPY made by the Nix builder.
  # The immutable pinned upstream input and all production consumers
  # are left untouched. Strict patch script fails if upstream changes.
  isolatedFx = upstreamFx.overrideAttrs (old: {
    pname = "scenefx-m31c-private-gl-unpack-ci";
    nativeBuildInputs = (old.nativeBuildInputs or []) ++ [ pkgs.python3 ];
    postPatch = (old.postPatch or "") + ''
      python3 ${./ci_company_m31c_patch_scenefx.py} \
        render/fx_renderer/fx_texture.c
    '';
  });
  inputs = originalSway.buildInputs or [];
  matches = builtins.filter
    (drv: builtins.toString drv == builtins.toString upstreamFx)
    inputs;
in
assert builtins.length matches == 1;
originalSway.overrideAttrs (old: {
  pname = "swayfx-company-m31c-private-gl-unpack-ci";
  mesonFlags = (old.mesonFlags or []) ++ [ "-Dsd-ci-company-crop-probe=true" ];
  buildInputs = builtins.map
    (drv:
      if builtins.toString drv == builtins.toString upstreamFx
      then isolatedFx else drv)
    (old.buildInputs or []);
})
