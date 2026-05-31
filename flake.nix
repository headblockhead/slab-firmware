{
  description = "Tools for developing and building interchange-firmware";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";

  outputs =
    { nixpkgs, ... }:
    let
      forEachSystemWithPackages =
        do:
        (nixpkgs.lib.genAttrs [ "x86_64-linux" "aarch64-linux" ] (
          system:
          do (
            import nixpkgs {
              inherit system;
              overlays = [
                (final: prev: {
                  pico-sdk = prev.pico-sdk.override { withSubmodules = true; };
                })
              ];
            }
          )
        ));
    in
    {
      packages = forEachSystemWithPackages (pkgs: rec {
        interchange-firmware = pkgs.stdenv.mkDerivation {
          name = "interchange-firmware";
          src = ./.;

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
          env = {
            PICO_SDK_PATH = "${pkgs.pico-sdk}/lib/pico-sdk";
          };
          installPhase = ''
            runHook preInstall
            mkdir -p $out
            cp prototype/{*.bin,*.elf,*.uf2,*.elf.map,*.dis} $out
            runHook postInstall
          '';
        };
        default = interchange-firmware;
      });
      devShells = forEachSystemWithPackages (pkgs: {
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
