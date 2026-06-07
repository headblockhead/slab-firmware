{
  description = "Firmware for Interchange keyboard modules";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";

  outputs =
    { nixpkgs, ... }:
    let
      pkgsForSystem =
        system:
        import nixpkgs {
          inherit system;
          overlays = [
            (final: prev: {
              pico-sdk = prev.pico-sdk.override { withSubmodules = true; };
            })
          ];
        };
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
      forEachSystem = nixpkgs.lib.genAttrs supportedSystems;
      forEachSystemWithPkgs = f: forEachSystem (system: f (pkgsForSystem system));
    in
    {
      packages = forEachSystemWithPkgs (pkgs: rec {
        interchange-firmware = pkgs.stdenv.mkDerivation {
          name = "interchange-firmware";
          src = pkgs.lib.cleanSource ./.;

          nativeBuildInputs = with pkgs; [
            cmake
            gcc-arm-embedded
            picotool
            python313
          ];
          cmakeFlags = [
            "-DCMAKE_C_COMPILER=${pkgs.gcc-arm-embedded}/bin/arm-none-eabi-gcc"
            "-DCMAKE_CXX_COMPILER=${pkgs.gcc-arm-embedded}/bin/arm-none-eabi-g++"
          ];
          env.PICO_SDK_PATH = "${pkgs.pico-sdk}/lib/pico-sdk";
        };
        default = interchange-firmware;
      });
      devShells = forEachSystemWithPkgs (pkgs: {
        default = pkgs.mkShell {
          packages = with pkgs; [
            cmake
            gcc-arm-embedded
            picotool
            python313
          ];
          PICO_SDK_PATH = "${pkgs.pico-sdk}/lib/pico-sdk";
        };
      });
    };
}
