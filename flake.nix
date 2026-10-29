{
  description = "pico-3d-mouse RP2040 firmware and host TDD development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";

    # Pico SDK as a source-only input with submodules (tinyusb etc.)
    # fetched alongside it — the nixpkgs `pico-sdk` package omits
    # submodules, which breaks USB-CDC / TinyUSB-backed features.
    pico-sdk-src = {
      type = "git";
      url = "https://github.com/raspberrypi/pico-sdk.git";
      ref = "refs/tags/2.2.0";
      submodules = true;
      flake = false;
    };
  };

  outputs = { self, nixpkgs, flake-utils, pico-sdk-src }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = pkgs.mkShell {
          name = "pico-3d-mouse";

          packages = with pkgs; [
            # ── Host toolchain & off-target TDD ──────────────────
            gcc
            gtest
            clang-tools
            python3

            # ── RP2040 Cross-Toolchain ───────────────────────────
            # Cortex-M0+ (RP2040) — arm-none-eabi-{gcc,g++,gdb,objdump,...}
            gcc-arm-embedded

            # ── Flashing / debugging ─────────────────────────────
            picotool
            openocd-rp2040
            usbutils

            # ── Serial console for USB-CDC ───────────────────────
            picocom

            # ── Build system ─────────────────────────────────────
            cmake
            ninja
            pkg-config
            git
          ];

          shellHook = ''
            export PICO_SDK_PATH=${pico-sdk-src}
            export PICO_BOARD=pico

            echo "────────────────────────────────────────────────"
            echo " pico-3d-mouse (RP2040 + Host TDD) dev shell"
            echo "────────────────────────────────────────────────"
            echo "  host g++          : $(g++ --version 2>/dev/null | head -1)"
            echo "  arm-none-eabi-gcc : $(arm-none-eabi-gcc --version 2>/dev/null | head -1)"
            echo "  picotool          : $(picotool version 2>&1 | head -1)"
            echo "  cmake             : $(cmake --version 2>/dev/null | head -1)"
            echo "  PICO_SDK_PATH     : $PICO_SDK_PATH"
            echo "  PICO_BOARD        : $PICO_BOARD"
            echo "────────────────────────────────────────────────"
          '';
        };
      });
}
