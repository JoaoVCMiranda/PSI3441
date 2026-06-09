#!/usr/bin/env bash
# =============================================================================
# KL25Z Development Environment — Mega Install Script
# Supports: Debian/Ubuntu and Arch Linux
# Installs: ARM toolchain, PyOCD, OpenOCD, VSCode + extensions, udev rules
# =============================================================================

set -euo pipefail

# ── Colors ────────────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

info()    { echo -e "${CYAN}[INFO]${NC}  $*"; }
ok()      { echo -e "${GREEN}[OK]${NC}    $*"; }
warn()    { echo -e "${YELLOW}[WARN]${NC}  $*"; }
die()     { echo -e "${RED}[ERROR]${NC} $*" >&2; exit 1; }
section() { echo -e "\n${BOLD}━━━ $* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"; }

# ── Detect distro ─────────────────────────────────────────────────────────────
detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        case "$ID" in
            debian|ubuntu|linuxmint|pop|kali|raspbian) DISTRO="debian" ;;
            arch|manjaro|endeavouros|garuda)            DISTRO="arch"   ;;
            *)
                case "$ID_LIKE" in
                    *debian*|*ubuntu*) DISTRO="debian" ;;
                    *arch*)            DISTRO="arch"   ;;
                    *) die "Unsupported distro: $ID. This script supports Debian/Ubuntu and Arch." ;;
                esac
                ;;
        esac
    else
        die "/etc/os-release not found. Cannot detect distro."
    fi
    ok "Detected distro family: ${BOLD}$DISTRO${NC}"
}

# ── Privilege check ───────────────────────────────────────────────────────────
check_sudo() {
    if [ "$EUID" -eq 0 ]; then
        SUDO=""
        warn "Running as root. Consider running as a regular user with sudo."
    elif command -v sudo &>/dev/null; then
        SUDO="sudo"
        info "sudo available."
    else
        die "Not root and sudo not found. Install sudo or run as root."
    fi
}

# ── Package manager wrappers ──────────────────────────────────────────────────
pkg_install() {
    case "$DISTRO" in
        debian) $SUDO apt-get install -y "$@" ;;
        arch)   $SUDO pacman -S --needed --noconfirm "$@" ;;
    esac
}

pkg_update() {
    case "$DISTRO" in
        debian) $SUDO apt-get update -qq ;;
        arch)   $SUDO pacman -Sy --noconfirm ;;
    esac
}

# ── 1. System update ──────────────────────────────────────────────────────────
install_system_update() {
    section "System Update"
    pkg_update
    ok "Package lists updated."
}

# ── 2. Base build tools ───────────────────────────────────────────────────────
install_base_tools() {
    section "Base Build Tools"
    case "$DISTRO" in
        debian)
            pkg_install \
                build-essential \
                cmake \
                ninja-build \
                git \
                curl \
                wget \
                unzip \
                ca-certificates \
                pkg-config \
                libusb-1.0-0-dev \
                libudev-dev
            ;;
        arch)
            pkg_install \
                base-devel \
                cmake \
                ninja \
                git \
                curl \
                wget \
                unzip \
                ca-certificates \
                pkgconf \
                libusb \
                systemd-libs
            ;;
    esac
    ok "Base tools installed."
}

# ── 3. ARM cross-compiler toolchain ──────────────────────────────────────────
install_arm_toolchain() {
    section "ARM Cortex-M Toolchain (arm-none-eabi)"
    case "$DISTRO" in
        debian)
            pkg_install \
                gcc-arm-none-eabi \
                binutils-arm-none-eabi \
                libnewlib-arm-none-eabi \
                libnewlib-dev \
                libstdc++-arm-none-eabi-newlib
            ;;
        arch)
            pkg_install \
                arm-none-eabi-gcc \
                arm-none-eabi-binutils \
                arm-none-eabi-newlib
            ;;
    esac

    # Verify
    if command -v arm-none-eabi-gcc &>/dev/null; then
        VER=$(arm-none-eabi-gcc --version | head -1)
        ok "arm-none-eabi-gcc: $VER"
    else
        die "arm-none-eabi-gcc not found after install."
    fi
}

# ── 4. OpenOCD ────────────────────────────────────────────────────────────────
install_openocd() {
    section "OpenOCD"
    case "$DISTRO" in
        debian) pkg_install openocd ;;
        arch)   pkg_install openocd ;;
    esac

    if command -v openocd &>/dev/null; then
        VER=$(openocd --version 2>&1 | head -1)
        ok "OpenOCD: $VER"
    else
        die "OpenOCD not found after install."
    fi
}

# ── 5. uv (Python tool runner, replaces pip) ──────────────────────────────────
install_uv() {
    section "uv (Python package manager)"
    if command -v uv &>/dev/null; then
        ok "uv already installed: $(uv --version)"
        return
    fi

    info "Installing uv via official installer..."
    curl -LsSf https://astral.sh/uv/install.sh | sh

    # Add to PATH for current session
    export PATH="$HOME/.local/bin:$PATH"

    if command -v uv &>/dev/null; then
        ok "uv installed: $(uv --version)"
    else
        die "uv installation failed or not in PATH. Check ~/.local/bin."
    fi
}

# ── 6. PyOCD (via uv) ─────────────────────────────────────────────────────────
install_pyocd() {
    section "PyOCD (via uv tool)"

    # Ensure uv is in PATH (may have been installed just now)
    export PATH="$HOME/.local/bin:$PATH"

    if ! command -v uv &>/dev/null; then
        die "uv not found. Cannot install PyOCD."
    fi

    info "Installing pyocd as a uv tool..."
    uv tool install pyocd

    # uv tools land in ~/.local/bin
    export PATH="$HOME/.local/bin:$PATH"

    if command -v pyocd &>/dev/null; then
        ok "PyOCD installed: $(pyocd --version)"
    else
        die "pyocd not found after install. Ensure ~/.local/bin is in your PATH."
    fi

    # Install KL25Z target pack
    section "PyOCD — KL25Z Target Pack"
    info "Installing NXP KL25Z CMSIS pack (requires internet)..."
    pyocd pack install MKL25Z128xxx4 && ok "KL25Z pack installed." \
        || warn "Pack install failed. Run manually: pyocd pack install MKL25Z128xxx4"
}

# ── 7. udev rules for OpenSDA / CMSIS-DAP ────────────────────────────────────
install_udev_rules() {
    section "udev Rules (OpenSDA / CMSIS-DAP)"

    UDEV_FILE="/etc/udev/rules.d/50-cmsis-dap.rules"

    $SUDO tee "$UDEV_FILE" > /dev/null <<'EOF'
# CMSIS-DAP / OpenSDA (Freescale FRDM-KL25Z and similar)
SUBSYSTEM=="usb", ATTR{idVendor}=="0d28", ATTR{idProduct}=="0204", MODE="0666", GROUP="plugdev", TAG+="uaccess"
# Freescale OpenSDA bootloader
SUBSYSTEM=="usb", ATTR{idVendor}=="15a2", ATTR{idProduct}=="0073", MODE="0666", GROUP="plugdev", TAG+="uaccess"
# Generic CMSIS-DAP
SUBSYSTEM=="usb", ATTRS{product}=="*CMSIS-DAP*", MODE="0666", GROUP="plugdev", TAG+="uaccess"
EOF

    $SUDO udevadm control --reload-rules
    $SUDO udevadm trigger
    ok "udev rules installed at $UDEV_FILE"

    # Add current user to plugdev (Debian) — Arch typically doesn't need this
    if [ "$DISTRO" = "debian" ]; then
        $SUDO usermod -aG plugdev "$USER" \
            && info "Added $USER to plugdev group (re-login required)." \
            || warn "Could not add $USER to plugdev. Do it manually: sudo usermod -aG plugdev $USER"
    fi
}


# ── Summary ───────────────────────────────────────────────────────────────────
print_summary() {
    section "Installation Summary"
    echo ""
    echo -e "  ${GREEN}✔${NC} ARM toolchain  : $(arm-none-eabi-gcc --version 2>/dev/null | head -1 || echo 'not found')"
    echo -e "  ${GREEN}✔${NC} OpenOCD        : $(openocd --version 2>&1 | head -1 || echo 'not found')"
    echo -e "  ${GREEN}✔${NC} uv             : $(uv --version 2>/dev/null || echo 'not found')"
    echo -e "  ${GREEN}✔${NC} PyOCD          : $(pyocd --version 2>/dev/null || echo 'not found')"
    echo -e "  ${GREEN}✔${NC} VSCode         : $(code --version 2>/dev/null | head -1 || echo 'not found')"
    echo ""
    echo -e "  ${YELLOW}Next steps:${NC}"
    echo -e "  1. Re-login (or run: newgrp plugdev) to apply udev/group changes"
    echo -e "  2. Plug in the KL25Z and run:  ${CYAN}pyocd list${NC}"
    echo -e "  3. Flash a binary:             ${CYAN}pyocd flash firmware.bin --target MKL25Z128xxx4${NC}"
    echo -e "  4. Open project in VSCode and use the Cortex-Debug launch config"
    echo ""
}

# ── Main ──────────────────────────────────────────────────────────────────────
main() {
    echo -e "${BOLD}"
    echo "  ╔══════════════════════════════════════════════╗"
    echo "  ║   KL25Z Dev Environment — Mega Installer    ║"
    echo "  ║   Debian / Arch                              ║"
    echo "  ╚══════════════════════════════════════════════╝"
    echo -e "${NC}"

    detect_distro
    check_sudo

    install_system_update
    install_base_tools
    install_arm_toolchain
    install_openocd
    install_uv
    install_pyocd
    install_udev_rules

    print_summary
}

main "$@"
