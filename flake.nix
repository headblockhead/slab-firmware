{
  description = "Firmware for Interchange keyboard modules";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
    libedbus-rp2040.url = "github:headblockhead/libedbus-rp2040";
  };

  outputs =
    {
      self,
      nixpkgs,
      libedbus-rp2040,
      ...
    }:
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

    in
    {
      packages = forEachSystem (
        system:
        let
          pkgs = pkgsForSystem system;
          edbusPkg = libedbus-rp2040.packages.${system}.default;
        in
        rec {
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
              edbusPkg
            ];
            cmakeFlags = [
              "-DCMAKE_C_COMPILER=${pkgs.gcc-arm-embedded}/bin/arm-none-eabi-gcc"
              "-DCMAKE_CXX_COMPILER=${pkgs.gcc-arm-embedded}/bin/arm-none-eabi-g++"
              "-Dedbus-rp2040_DIR=${edbusPkg}/lib/cmake/edbus-rp2040"
            ];
            env.PICO_SDK_PATH = "${pkgs.pico-sdk}/lib/pico-sdk";
          };
          default = interchange-firmware;
        }
      );
      devShells = forEachSystem (
        system:
        let
          pkgs = pkgsForSystem system;
        in
        {
          default = pkgs.mkShell {
            inputsFrom = [
              self.packages.${system}.interchange-firmware
            ];
            PICO_SDK_PATH = "${pkgs.pico-sdk}/lib/pico-sdk";
          };
        }
      );
    };
}
