{
  description = "Firmware for Interchange keyboard modules";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    pico-edbus.url = "git+https://seed.edwardh.dev/rad:z2AihL5EFL98E9nhuwguu6tj51yYN";
  };

  outputs =
    { self, nixpkgs, ... }@inputs:
    let
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
      forEachSystemWithPkgs =
        do:
        nixpkgs.lib.genAttrs supportedSystems (
          system:
          do {
            inherit system;
            pkgs = import nixpkgs {
              inherit system;
              overlays = [
                (final: prev: {
                  pico-sdk = prev.pico-sdk.override { withSubmodules = true; };
                  pico-edbus = inputs.pico-edbus.packages.${prev.stdenv.hostPlatform.system}.default;
                })
              ];
            };
          }
        );
    in
    {
      packages = forEachSystemWithPkgs (
        { pkgs, system }: rec {
          interchange-firmware = pkgs.stdenv.mkDerivation {
            name = "interchange-firmware";
            src = pkgs.lib.cleanSource ./.;

            nativeBuildInputs = with pkgs; [
              cmake
              gcc-arm-embedded
              picotool
              python313
            ];
            buildInputs = [
              pkgs.pico-edbus
            ];
            cmakeFlags = [
              "-DCMAKE_C_COMPILER=${pkgs.gcc-arm-embedded}/bin/arm-none-eabi-gcc"
              "-DCMAKE_CXX_COMPILER=${pkgs.gcc-arm-embedded}/bin/arm-none-eabi-g++"
            ];
            env = {
              PICO_SDK_PATH = "${pkgs.pico-sdk}/lib/pico-sdk";
              PICO_EDBUS_PATH = "${pkgs.pico-edbus}/lib/cmake/pico-edbus";
            };
          };
          default = interchange-firmware;
        }
      );
      devShells = forEachSystemWithPkgs (
        { pkgs, system }: {
          default = pkgs.mkShell {
            inputsFrom = [
              self.packages.${system}.interchange-firmware
            ];
            PICO_SDK_PATH = "${pkgs.pico-sdk}/lib/pico-sdk";
            PICO_EDBUS_PATH = "${pkgs.pico-edbus}/lib/cmake/pico-edbus";
          };
        }
      );
    };
}
