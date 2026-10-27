# CI-only override: the normal SwayPX package and pinned SceneFX input are
# never modified. Rebuild the dependency with diagnostic log hooks ONLY in
# the isolated headless runtime smoke. No production compositor uses this.
let
  flake = builtins.getFlake (toString ../..);
  system = "x86_64-linux";
  pkgs = flake.inputs.nixpkgs.legacyPackages.${system};
  ordinarySceneFX = flake.inputs.scenefx.packages.${system}.scenefx-git;
  tracedSceneFX = ordinarySceneFX.overrideAttrs (old: {
    nativeBuildInputs = (old.nativeBuildInputs or []) ++ [ pkgs.python3 ];
    postPatch = (old.postPatch or "") + ''
      ${pkgs.python3}/bin/python3 ${./ci_scenefx_render_trace.py}
    '';
  });
in
  flake.packages.${system}.swayfx-unwrapped-git.overrideAttrs (old: {
    # Project flake.nix places SceneFX last in the explicit buildInputs.
    # Fail if the project changes dependency ordering, rather than silently
    # replacing some unrelated dependency or losing the pinned SceneFX.
    buildInputs =
      assert builtins.elemAt old.buildInputs (builtins.length old.buildInputs - 1)
        == ordinarySceneFX;
      (pkgs.lib.init old.buildInputs) ++ [ tracedSceneFX ];
    mesonFlags = (old.mesonFlags or []) ++ [ "-Dsd-ci-news-scene-probe=true" ];
  })
