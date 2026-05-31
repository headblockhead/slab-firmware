{
  description = "Tools for developing and building interchange-firmware";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";

  outputs =
    { nixpkgs, ... }:
    let
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;

      interchange-firmware = (
        pkgs:
        pkgs.stdenv.mkDerivation {
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
        }
      );
    in
    {
      packages = forAllSystems (
        system:
        let
          pkgs = import nixpkgs { inherit system; };
        in
        {
          interchange-firmware = interchange-firmware pkgs;
          default = interchange-firmware pkgs;
        }
      );
      devShells = forAllSystems (
        system:
        let
          pkgs = import nixpkgs { inherit system; };
        in
        {
          default = pkgs.mkShell {
            packages = with pkgs; [
              cmake
              #gcc-arm-embedded
              #picotool
              #python39
            ];
            PICO_SDK_PATH = "${pkgs.pico-sdk}/lib/pico-sdk";
          };
        }
      );
    };
}
