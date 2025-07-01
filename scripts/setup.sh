#!/bin/bash

# Blacksite Engine - Development Environment Setup Script
# This script sets up everything needed to develop on Blacksite Engine

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo ""
    echo -e "${PURPLE}=== $1 ===${NC}"
    echo ""
}

print_step() {
    echo -e "${CYAN}➤${NC} $1"
}

# Check if running on supported OS
check_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        OS="linux"
        print_success "Detected Linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
        print_success "Detected macOS"
    else
        print_error "Unsupported operating system: $OSTYPE"
        print_info "This script supports Linux and macOS only"
        exit 1
    fi
}

# Install system dependencies
install_deps() {
    print_header "Installing System Dependencies"

    if [[ "$OS" == "linux" ]]; then
        print_step "Updating package manager..."
        sudo apt update

        print_step "Installing build tools..."
        sudo apt install -y \
            build-essential \
            cmake \
            git \
            pkg-config \
            ninja-build

        print_step "Installing graphics libraries..."
        sudo apt install -y \
            libgl1-mesa-dev \
            libglu1-mesa-dev \
            libglfw3-dev \
            libglew-dev \
            libglm-dev \
            libassimp-dev

        print_step "Installing development tools..."
        sudo apt install -y \
            clang-format \
            gdb \
            valgrind

    elif [[ "$OS" == "macos" ]]; then
        if ! command -v brew &> /dev/null; then
            print_error "Homebrew not found. Please install Homebrew first:"
            print_info "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
            exit 1
        fi

        print_step "Installing build tools..."
        brew install cmake ninja git pkg-config

        print_step "Installing graphics libraries..."
        brew install glfw glew glm assimp

        print_step "Installing development tools..."
        brew install clang-format
    fi

    print_success "System dependencies installed"
}

# Setup git submodules
setup_submodules() {
    print_header "Setting Up Git Submodules"

    cd "$PROJECT_ROOT"

    print_step "Initializing submodules..."
    git submodule update --init --recursive

    print_step "Switching ImGui to docking branch..."
    cd third_party/imgui
    git checkout docking
    cd "$PROJECT_ROOT"

    print_success "Submodules configured"
}

# Make scripts executable
setup_scripts() {
    print_header "Setting Up Build Scripts"

    print_step "Making scripts executable..."
    chmod +x "$SCRIPT_DIR"/*.sh

    print_success "Scripts configured"
}

# Create development directories
setup_directories() {
    print_header "Creating Development Directories"

    print_step "Creating build directory..."
    mkdir -p "$PROJECT_ROOT/build"

    print_step "Creating logs directory..."
    mkdir -p "$PROJECT_ROOT/logs"

    print_success "Directories created"
}

# Initial build test
test_build() {
    print_header "Testing Initial Build"

    print_step "Running clean build..."
    "$SCRIPT_DIR/build.sh" --clean --debug

    if [[ -f "$PROJECT_ROOT/build/blacksite_test" ]]; then
        print_success "Build test passed!"
    else
        print_error "Build test failed - executable not found"
        exit 1
    fi
}

# Show completion message
show_completion() {
    print_header "Setup Complete!"

    echo -e "${GREEN}🎉 Blacksite Engine development environment is ready!${NC}"
    echo ""
    echo -e "${CYAN}Next steps:${NC}"
    echo "  1. Try a quick build and run:"
    echo "     ${YELLOW}./scripts/dev.sh quick${NC}"
    echo ""
    echo "  2. Check project info:"
    echo "     ${YELLOW}./scripts/dev.sh info${NC}"
    echo ""
    echo "  3. See all available commands:"
    echo "     ${YELLOW}./scripts/dev.sh --help${NC}"
    echo ""
    echo -e "${CYAN}Development workflow:${NC}"
    echo "  • Quick iteration: ${YELLOW}./scripts/dev.sh fast${NC}"
    echo "  • Clean build: ${YELLOW}./scripts/dev.sh quick${NC}"
    echo "  • Release build: ${YELLOW}./scripts/dev.sh release${NC}"
    echo "  • Clean everything: ${YELLOW}./scripts/dev.sh clean${NC}"
    echo ""
    echo -e "${CYAN}Project structure:${NC}"
    echo "  • Source code: ${YELLOW}src/${NC} and ${YELLOW}include/${NC}"
    echo "  • Build output: ${YELLOW}build/${NC}"
    echo "  • Scripts: ${YELLOW}scripts/${NC}"
    echo "  • Documentation: ${YELLOW}docs/${NC}"
    echo ""
    echo -e "${GREEN}Happy coding! 🚀${NC}"
}

# Main setup flow
main() {
    echo -e "${PURPLE}"
    cat << 'EOF'
    ____  __           __        _ __
   / __ )/ /___ ______/ /_______(_) /______
  / __  / / __ `/ ___/ //_/ ___/ / __/ ___/
 / /_/ / / /_/ / /__/ ,< (__  ) / /_/ __/
/_____/_/\__,_/\___/_/|_/____/_/\__/___/

        Engine Development Setup
EOF
    echo -e "${NC}"

    print_info "Setting up Blacksite Engine development environment..."
    print_info "Project: $PROJECT_ROOT"

    # Run setup steps
    check_os
    install_deps
    setup_submodules
    setup_scripts
    setup_directories
    test_build
    show_completion
}

# Handle command line arguments
case "${1:-}" in
    -h|--help|help)
        cat << EOF
Blacksite Engine Development Setup

Usage: $0 [OPTIONS]

This script will:
  1. Install required system dependencies
  2. Set up git submodules (ImGui, JoltPhysics)
  3. Configure build scripts
  4. Create necessary directories
  5. Test the build system

OPTIONS:
    -h, --help      Show this help message

REQUIREMENTS:
  • Linux: Ubuntu 20.04+ or similar
  • macOS: 10.15+ with Homebrew
  • Git
  • Internet connection

The script will ask for sudo permissions to install system packages.
EOF
        exit 0
        ;;
    "")
        main
        ;;
    *)
        print_error "Unknown option: $1"
        print_info "Use --help for usage information"
        exit 1
        ;;
esac
