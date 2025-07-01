# Blacksite Engine - Build Scripts

This directory contains build and development scripts for the Blacksite Engine project.

## Quick Start

For new developers, run the setup script first:
```bash
./scripts/setup.sh
```

For daily development, use these commands:
```bash
# Quick development cycle (most common)
./scripts/dev.sh quick      # Clean build + run

# Fast iteration (no clean)
./scripts/dev.sh fast       # Build + run

# Release build
./scripts/dev.sh release    # Clean release build + run
```

## Available Scripts

### 🚀 `setup.sh`
**One-time setup for new developers**
- Installs system dependencies (OpenGL, GLFW, CMake, etc.)
- Configures git submodules (ImGui docking branch, JoltPhysics)
- Creates build directories
- Tests initial build
- Makes other scripts executable

### 🔨 `build.sh`
**Main build script with full control**
```bash
./scripts/build.sh [OPTIONS]

OPTIONS:
    -c, --clean         Clean build directory before building
    -r, --run           Run the engine after building
    -d, --debug         Build in debug mode (default)
    -R, --release       Build in release mode
    -v, --verbose       Verbose output
    -j, --jobs N        Number of parallel jobs
    -h, --help          Show help

EXAMPLES:
    ./scripts/build.sh                 # Basic debug build
    ./scripts/build.sh -c -r           # Clean build and run
    ./scripts/build.sh -R -r           # Release build and run
    ./scripts/build.sh -c -d -v -j8    # Clean debug build, verbose, 8 jobs
```

### ⚡ `dev.sh`
**Development utility for common tasks**
```bash
./scripts/dev.sh [COMMAND]

COMMANDS:
    quick           Clean debug build + run (most common)
    fast            Debug build + run (no clean)
    release         Clean release build + run
    clean           Clean all build artifacts and logs
    format          Format code using clang-format
    info            Show project information and status
    logs            Show recent log files
    commit          Quick git commit helper
    setup           Setup development environment
    deps            Check dependencies
```

## Development Workflow

### Daily Development
```bash
# Start of day - get latest changes
git pull
./scripts/dev.sh deps        # Check if dependencies changed

# Fast iteration cycle
./scripts/dev.sh fast        # Quick build and test

# When things get weird
./scripts/dev.sh quick       # Clean build
```

### Before Committing
```bash
./scripts/dev.sh format      # Format code
./scripts/dev.sh release     # Test release build
./scripts/dev.sh commit      # Interactive commit helper
```

### Troubleshooting
```bash
./scripts/dev.sh clean       # Nuclear option - clean everything
./scripts/dev.sh setup       # Re-setup environment
./scripts/dev.sh info        # Check project status
```

## Project Structure

The scripts work with this project layout:
```
blacksite/
├── build/              # Build output (created by scripts)
├── docs/               # Documentation
├── include/            # Header files
├── logs/               # Runtime logs (created by engine)
├── scripts/            # Build and development scripts
├── src/                # Source code
├── third_party/        # Git submodules (ImGui, JoltPhysics)
├── CMakeLists.txt      # CMake configuration
└── README.md           # Project README
```

## Dependencies

### System Requirements
- **Linux**: Ubuntu 20.04+ or equivalent
- **macOS**: 10.15+ with Homebrew
- **Windows**: Not yet supported

### Required Packages
- `cmake` (3.16+)
- `build-essential` / Xcode Command Line Tools
- `git`
- `pkg-config`

### Graphics Libraries
- `libgl1-mesa-dev` / OpenGL
- `libglfw3-dev` / GLFW
- `libglew-dev` / GLEW
- `libglm-dev` / GLM
- `libassimp-dev` / Assimp

### Development Tools (Optional)
- `clang-format` - Code formatting
- `gdb` / `lldb` - Debugging
- `valgrind` - Memory checking (Linux)

## Build Targets

The main build target is `blacksite_test` which contains:
- Complete Blacksite Engine
- Editor with docking UI
- Scene hierarchy, entity inspector
- Console with integrated logger
- Performance monitoring
- Physics simulation

## Environment Variables

The scripts respect these environment variables:
- `CC` / `CXX` - C/C++ compilers
- `CMAKE_GENERATOR` - CMake generator (default: Unix Makefiles)
- `CMAKE_BUILD_PARALLEL_LEVEL` - Parallel build jobs

## Troubleshooting

### Build Fails
1. Check dependencies: `./scripts/dev.sh deps`
2. Clean build: `./scripts/dev.sh clean`
3. Re-setup: `./scripts/dev.sh setup`

### Missing Submodules
```bash
git submodule update --init --recursive
cd third_party/imgui && git checkout docking
```

### Permission Issues
```bash
chmod +x scripts/*.sh
```

### Graphics Issues
Ensure you have proper OpenGL drivers installed:
```bash
# Linux
sudo apt install mesa-utils
glxinfo | grep "OpenGL version"

# Check GLFW/GLEW
pkg-config --modversion glfw3 glew
```

## Script Maintenance

### Adding New Scripts
1. Create script in `scripts/` directory
2. Make executable: `chmod +x scripts/new-script.sh`
3. Follow existing patterns for colored output and error handling
4. Update this README

### Modifying Existing Scripts
- Test on both Linux and macOS if possible
- Maintain backward compatibility with existing workflows
- Use colored output functions for consistency
- Handle errors gracefully with `set -e`

## Getting Help

- Script help: `./scripts/[script].sh --help`
- Project info: `./scripts/dev.sh info`
- Check logs: `./scripts/dev.sh logs`
- Dependencies: `./scripts/dev.sh deps`

For engine-specific help, see the main project README.
