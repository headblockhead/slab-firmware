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
          buildInputs = [
            libedbus-rp2040.packages.${pkgs.stdenv.hostPlatform.system}.default
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
          inputsFrom = [
            self.packages.${pkgs.stdenv.hostPlatform.system}.interchange-firmware
          ];
          PICO_SDK_PATH = "${pkgs.pico-sdk}/lib/pico-sdk";
        };
      });
    };
}
