#!/bin/bash

# Blacksite Engine Development Utility Script
# Usage: ./scripts/dev.sh [command] [options]

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
BUILD_DIR="$PROJECT_ROOT/build"

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
    echo -e "${PURPLE}=== $1 ===${NC}"
}

# Function to show usage
show_usage() {
    cat << EOF
Blacksite Engine Development Utility

Usage: $0 [COMMAND] [OPTIONS]

COMMANDS:
    quick           Quick build and run (clean + debug + run)
    fast            Fast build and run (no clean, debug + run)
    release         Release build and run
    clean           Clean all build artifacts
    test            Run tests (if any)
    format          Format code using clang-format
    lint            Run static analysis
    deps            Check and install dependencies
    info            Show project information
    logs            Show recent log files
    commit          Quick git commit helper
    setup           Setup development environment

OPTIONS:
    -v, --verbose   Verbose output
    -h, --help      Show this help

EXAMPLES:
    $0 quick        # Clean build and run
    $0 fast         # Quick build and run
    $0 release      # Release build and run
    $0 clean        # Clean everything
    $0 info         # Show project info
EOF
}

# Quick build and run (most common)
cmd_quick() {
    print_header "Quick Development Build"
    "$SCRIPT_DIR/build.sh" --clean --debug --run --verbose
}

# Fast build and run (no clean)
cmd_fast() {
    print_header "Fast Development Build"
    "$SCRIPT_DIR/build.sh" --debug --run
}

# Release build and run
cmd_release() {
    print_header "Release Build"
    "$SCRIPT_DIR/build.sh" --clean --release --run
}

# Clean everything
cmd_clean() {
    print_header "Cleaning Project"

    # Remove build directory
    if [[ -d "$BUILD_DIR" ]]; then
        print_info "Removing build directory..."
        rm -rf "$BUILD_DIR"
        print_success "Build directory removed"
    fi

    # Remove logs
    if [[ -d "$PROJECT_ROOT/logs" ]]; then
        print_info "Removing log files..."
        rm -rf "$PROJECT_ROOT/logs"
        print_success "Log files removed"
    fi

    # Remove temporary files
    find "$PROJECT_ROOT" -name "*.tmp" -delete 2>/dev/null || true
    find "$PROJECT_ROOT" -name "core" -delete 2>/dev/null || true

    # Remove ImGui ini file
    if [[ -f "$PROJECT_ROOT/imgui.ini" ]]; then
        rm "$PROJECT_ROOT/imgui.ini"
        print_info "Removed imgui.ini"
    fi

    print_success "Project cleaned"
}

# Format code
cmd_format() {
    print_header "Formatting Code"

    if ! command -v clang-format &> /dev/null; then
        print_warning "clang-format not found, skipping code formatting"
        return
    fi

    print_info "Formatting C++ files..."
    find "$PROJECT_ROOT/src" "$PROJECT_ROOT/include" -name "*.cpp" -o -name "*.h" | xargs clang-format -i
    print_success "Code formatting completed"
}

# Show project information
cmd_info() {
    print_header "Project Information"

    echo -e "${CYAN}Project:${NC} Blacksite Engine"
    echo -e "${CYAN}Location:${NC} $PROJECT_ROOT"
    echo -e "${CYAN}Build Dir:${NC} $BUILD_DIR"

    if [[ -f "$PROJECT_ROOT/CMakeLists.txt" ]]; then
        VERSION=$(grep "project.*VERSION" "$PROJECT_ROOT/CMakeLists.txt" | head -1)
        echo -e "${CYAN}CMake:${NC} $VERSION"
    fi

    echo ""
    echo -e "${CYAN}Git Status:${NC}"
    if git -C "$PROJECT_ROOT" status --porcelain | head -5; then
        echo "  (showing first 5 changes)"
    else
        echo "  Working directory clean"
    fi

    echo ""
    echo -e "${CYAN}Recent Commits:${NC}"
    git -C "$PROJECT_ROOT" log --oneline -5 || echo "  No git repository"

    echo ""
    echo -e "${CYAN}Build Status:${NC}"
    if [[ -f "$BUILD_DIR/blacksite_test" ]]; then
        ls -lh "$BUILD_DIR/blacksite_test"
    else
        echo "  Not built"
    fi

    echo ""
    echo -e "${CYAN}Dependencies:${NC}"
    echo "  - OpenGL: $(pkg-config --modversion gl 2>/dev/null || echo 'Unknown')"
    echo "  - GLFW: $(pkg-config --modversion glfw3 2>/dev/null || echo 'Unknown')"
    echo "  - GLEW: $(pkg-config --modversion glew 2>/dev/null || echo 'Unknown')"
    echo "  - GLM: $(pkg-config --modversion glm 2>/dev/null || echo 'Unknown')"
}

# Show recent logs
cmd_logs() {
    print_header "Recent Log Files"

    if [[ -d "$PROJECT_ROOT/logs" ]]; then
        echo -e "${CYAN}Log Directory:${NC} $PROJECT_ROOT/logs"
        ls -la "$PROJECT_ROOT/logs/"

        # Show most recent log
        RECENT_LOG=$(ls -t "$PROJECT_ROOT/logs/"*.log 2>/dev/null | head -1)
        if [[ -n "$RECENT_LOG" ]]; then
            echo ""
            echo -e "${CYAN}Most Recent Log:${NC} $(basename "$RECENT_LOG")"
            echo "Last 10 lines:"
            tail -10 "$RECENT_LOG"
        fi
    else
        print_info "No log directory found"
    fi
}

# Git commit helper
cmd_commit() {
    print_header "Quick Commit Helper"

    # Check if there are changes
    if ! git -C "$PROJECT_ROOT" diff --quiet; then
        echo -e "${CYAN}Changed files:${NC}"
        git -C "$PROJECT_ROOT" status --porcelain

        echo ""
        read -p "Enter commit message: " commit_msg

        if [[ -n "$commit_msg" ]]; then
            git -C "$PROJECT_ROOT" add .
            git -C "$PROJECT_ROOT" commit -m "$commit_msg"
            print_success "Committed changes"

            read -p "Push to remote? (y/N): " push_answer
            if [[ "$push_answer" =~ ^[Yy]$ ]]; then
                git -C "$PROJECT_ROOT" push
                print_success "Pushed to remote"
            fi
        else
            print_warning "No commit message provided, skipping"
        fi
    else
        print_info "No changes to commit"
    fi
}

# Setup development environment
cmd_setup() {
    print_header "Development Environment Setup"

    print_info "Checking required dependencies..."

    # Check for required tools
    MISSING_DEPS=()

    if ! command -v cmake &> /dev/null; then
        MISSING_DEPS+=("cmake")
    fi

    if ! command -v make &> /dev/null; then
        MISSING_DEPS+=("build-essential")
    fi

    if ! command -v git &> /dev/null; then
        MISSING_DEPS+=("git")
    fi

    if ! pkg-config --exists glfw3; then
        MISSING_DEPS+=("libglfw3-dev")
    fi

    if ! pkg-config --exists glew; then
        MISSING_DEPS+=("libglew-dev")
    fi

    if [[ ${#MISSING_DEPS[@]} -gt 0 ]]; then
        print_warning "Missing dependencies: ${MISSING_DEPS[*]}"
        echo "Install with:"
        echo "  sudo apt update"
        echo "  sudo apt install ${MISSING_DEPS[*]}"
    else
        print_success "All dependencies are available"
    fi

    # Setup git submodules
    print_info "Initializing git submodules..."
    git -C "$PROJECT_ROOT" submodule update --init --recursive

    # Create build directory
    mkdir -p "$BUILD_DIR"

    print_success "Development environment setup complete"
}

# Check dependencies
cmd_deps() {
    print_header "Dependency Check"

    echo -e "${CYAN}System Dependencies:${NC}"

    check_cmd() {
        if command -v "$1" &> /dev/null; then
            echo -e "  ✅ $1"
        else
            echo -e "  ❌ $1 (missing)"
        fi
    }

    check_pkg() {
        if pkg-config --exists "$1" 2>/dev/null; then
            VERSION=$(pkg-config --modversion "$1" 2>/dev/null)
            echo -e "  ✅ $1 ($VERSION)"
        else
            echo -e "  ❌ $1 (missing)"
        fi
    }

    check_cmd cmake
    check_cmd make
    check_cmd git
    check_cmd clang-format

    echo ""
    echo -e "${CYAN}Library Dependencies:${NC}"
    check_pkg glfw3
    check_pkg glew
    check_pkg glm

    echo ""
    echo -e "${CYAN}Git Submodules:${NC}"
    if [[ -d "$PROJECT_ROOT/third_party/imgui" ]]; then
        echo -e "  ✅ ImGui"
    else
        echo -e "  ❌ ImGui (run 'git submodule update --init')"
    fi

    if [[ -d "$PROJECT_ROOT/third_party/JoltPhysics" ]]; then
        echo -e "  ✅ JoltPhysics"
    else
        echo -e "  ❌ JoltPhysics"
    fi
}

# Main command dispatcher
case "${1:-}" in
    quick)
        cmd_quick
        ;;
    fast)
        cmd_fast
        ;;
    release)
        cmd_release
        ;;
    clean)
        cmd_clean
        ;;
    format)
        cmd_format
        ;;
    info)
        cmd_info
        ;;
    logs)
        cmd_logs
        ;;
    commit)
        cmd_commit
        ;;
    setup)
        cmd_setup
        ;;
    deps)
        cmd_deps
        ;;
    -h|--help|help)
        show_usage
        ;;
    "")
        print_error "No command specified"
        echo ""
        show_usage
        exit 1
        ;;
    *)
        print_error "Unknown command: $1"
        echo ""
        show_usage
        exit 1
        ;;
esac
