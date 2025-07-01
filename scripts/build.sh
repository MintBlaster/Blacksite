#!/bin/bash

# Blacksite Engine Build and Run Script
# Usage: ./scripts/build.sh [options]

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"

# Default options
CLEAN=false
RUN=false
DEBUG=false
RELEASE=false
VERBOSE=false
JOBS=$(nproc)

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

# Function to show usage
show_usage() {
    cat << EOF
Blacksite Engine Build Script

Usage: $0 [OPTIONS]

OPTIONS:
    -c, --clean         Clean build directory before building
    -r, --run           Run the engine after building
    -d, --debug         Build in debug mode (default)
    -R, --release       Build in release mode
    -v, --verbose       Verbose output
    -j, --jobs N        Number of parallel jobs (default: $JOBS)
    -h, --help          Show this help message

EXAMPLES:
    $0                  # Basic build
    $0 -c -r            # Clean build and run
    $0 -R -r            # Release build and run
    $0 -c -d -v -j8     # Clean debug build with verbose output using 8 jobs

BUILD TARGETS:
    The script builds the main 'blacksite_test' executable by default.
EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -r|--run)
            RUN=true
            shift
            ;;
        -d|--debug)
            DEBUG=true
            RELEASE=false
            shift
            ;;
        -R|--release)
            RELEASE=true
            DEBUG=false
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Determine build type
if [[ "$RELEASE" == true ]]; then
    BUILD_TYPE="Release"
else
    BUILD_TYPE="Debug"
fi

print_info "Blacksite Engine Build Script"
print_info "Project: $PROJECT_ROOT"
print_info "Build Type: $BUILD_TYPE"
print_info "Jobs: $JOBS"

# Clean build directory if requested
if [[ "$CLEAN" == true ]]; then
    print_warning "Cleaning build directory..."
    if [[ -d "$BUILD_DIR" ]]; then
        rm -rf "$BUILD_DIR"
        print_success "Build directory cleaned"
    else
        print_info "Build directory doesn't exist, skipping clean"
    fi
fi

# Create build directory
if [[ ! -d "$BUILD_DIR" ]]; then
    print_info "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure with CMake
print_info "Configuring with CMake..."
CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"

if [[ "$VERBOSE" == true ]]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_VERBOSE_MAKEFILE=ON"
fi

cmake $CMAKE_ARGS .. || {
    print_error "CMake configuration failed!"
    exit 1
}

print_success "CMake configuration completed"

# Build the project
print_info "Building Blacksite Engine..."

if [[ "$VERBOSE" == true ]]; then
    make -j"$JOBS" VERBOSE=1 || {
        print_error "Build failed!"
        exit 1
    }
else
    make -j"$JOBS" || {
        print_error "Build failed!"
        exit 1
    }
fi

print_success "Build completed successfully!"

# Show build artifacts
print_info "Build artifacts:"
if [[ -f "$BUILD_DIR/blacksite_test" ]]; then
    ls -lh "$BUILD_DIR/blacksite_test"
else
    print_warning "Main executable not found!"
fi

# Run the engine if requested
if [[ "$RUN" == true ]]; then
    print_info "Running Blacksite Engine..."

    if [[ -f "$BUILD_DIR/blacksite_test" ]]; then
        echo ""
        print_info "=== ENGINE OUTPUT ==="
        echo ""

        # Run the engine
        "$BUILD_DIR/blacksite_test" || {
            print_error "Engine execution failed!"
            exit 1
        }

        echo ""
        print_success "Engine execution completed"
    else
        print_error "Executable not found: $BUILD_DIR/blacksite_test"
        exit 1
    fi
fi

print_success "Script completed successfully!"
