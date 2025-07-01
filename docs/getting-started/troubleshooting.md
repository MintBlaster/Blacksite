# 🛠️ Troubleshooting Guide

Common issues and solutions for Blacksite Engine development and runtime problems.

---

## ⚙️ Build Issues

### CMake Configuration Fails

**Symptoms:**
- CMake cannot find required libraries
- "Could not find package" errors
- Configuration step fails

**Solutions:**
```sh
# Check dependencies are installed
./scripts/dev.sh deps

# Re-initialize submodules
git submodule update --init --recursive
cd third_party/imgui && git checkout docking && cd ../..

# Clean and reconfigure
./scripts/dev.sh clean
./scripts/build.sh
```

**Common Causes:**
- Missing system packages (GLFW, GLEW, GLM)
- Uninitialized git submodules
- Wrong ImGui branch (need docking branch)

### 🛠️ Compilation Errors

**Symptoms:**
- C++ compilation errors
- Missing header files
- Linker errors

**Solutions:**
```sh
# Check compiler version
gcc --version    # Need GCC 7+ or Clang 10+
clang++ --version

# Verbose build to see exact errors
./scripts/build.sh -v

# Check include paths
pkg-config --cflags glfw3 glew glm
```

**Common Causes:**
- Outdated compiler (need C++17 support)
- Missing development headers
- Incorrect library versions

### 🔗 Linking Errors

**Symptoms:**
- "undefined reference" errors
- Library not found errors
- Successful compilation but linking fails

**Solutions:**
```sh
# Check library installation
pkg-config --libs glfw3 glew
ldconfig -p | grep glfw

# Ubuntu: Install development packages
sudo apt install libglfw3-dev libglew-dev libglm-dev

# macOS: Install via Homebrew
brew install glfw glew glm
```

---

## 🚦 Runtime Issues

### 🖥️ Black Screen on Launch

**Symptoms:**
- Window opens but shows black screen
- No graphics rendering
- Application doesn't crash

**Diagnosis:**
```sh
# Check OpenGL support
glxinfo | grep "OpenGL version"
# Should show 3.3 or higher

# Check graphics drivers
lspci | grep VGA
# Identify your graphics card
```

**Solutions:**
```sh
# Ubuntu: Update graphics drivers
sudo apt update && sudo apt upgrade
sudo ubuntu-drivers autoinstall

# Install Mesa drivers (fallback)
sudo apt install mesa-utils

# Test OpenGL
glxgears
# Should show spinning gears
```

### 🪂 Physics Objects Not Falling

**Symptoms:**
- Objects spawn but don't move
- No gravity effect
- Static scene despite dynamic objects

**Common Causes & Solutions:**

**1. Ground Plane Not Static:**
```cpp
// WRONG - ground falls too
auto ground = engine.SpawnPlane({0, -1, 0}, {10, 0.2f, 10});

// CORRECT - ground stays put
auto ground = engine.SpawnPlane({0, -1, 0}, {10, 0.2f, 10});
engine.GetEntity(ground).MakeStatic();  // Essential!
```

**2. Objects Spawned Below Ground:**
```cpp
// WRONG - spawns inside ground
auto cube = engine.SpawnCube({0, -2, 0});

// CORRECT - spawns above ground
auto cube = engine.SpawnCube({0, 5, 0});
```

**3. Objects Accidentally Made Static:**
```cpp
// Check if object was made static
if (physics->IsBodyStatic(bodyID)) {
    physics->MakeBodyDynamic(bodyID);  // Re-enable physics
}
```

### 🖱️ Editor Not Showing

**Symptoms:**
- F1 key doesn't show panels
- No editor interface visible
- Console shows ImGui errors

**Solutions:**
```sh
# Ensure editor was built
./scripts/build.sh --target editor --run

# Check for ImGui initialization errors
# Look for messages like "Failed to initialize ImGui"

# Try toggling panels
# Press F1 multiple times

# Check ImGui submodule
cd third_party/imgui
git status
git checkout docking  # Must be on docking branch
```

### 🐢 Poor Performance / Low FPS

**Symptoms:**
- FPS below 30 with few objects
- Stuttering or jerky movement
- High CPU usage

**Diagnosis:**
- Check Performance panel in editor (F1)
  - Look for: Current FPS, number of dynamic bodies, physics simulation time

**Solutions:**

**1. Too Many Dynamic Objects:**
```cpp
// Make scenery static to reduce physics load
for (auto& decorativeObject : scenery) {
    engine.GetEntity(decorativeObject).MakeStatic();
}

// Limit dynamic objects to reasonable number
// Good: 10-50 dynamic objects
// Slow: 100+ dynamic objects
```

**2. Graphics Driver Issues:**
```sh
# Update graphics drivers
sudo ubuntu-drivers autoinstall

# Check if using integrated graphics
glxinfo | grep "OpenGL renderer"

# Force discrete GPU (if available)
DRI_PRIME=1 ./build/examples/BlacksiteEngine
```

**3. Debug Build Performance:**
```sh
# Use release build for performance testing
./scripts/dev.sh release
```

### 🎥 Camera Issues

**Symptoms:**
- Can't see anything in scene
- Camera looking at wrong direction
- Objects appear to be missing

**Solutions:**

**1. Camera Position Issues:**
```cpp
// Check camera is positioned away from origin
engine.SetCameraPosition({0, 0, 0});     // BAD - at origin
engine.SetCameraPosition({8, 6, 8});     // GOOD - diagonal view

// Ensure camera target is reasonable
engine.SetCameraTarget({0, 2, 0});       // Look at action area
```

**2. Objects Outside Camera View:**
```cpp
// Check object positions
auto pos = entity.GetPosition();
std::cout << "Object at: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;

// Reset camera to known good position
engine.SetCameraPosition({10, 8, 10});
engine.SetCameraTarget({0, 3, 0});
```

**3. Camera Inside Objects:**
```cpp
// Ensure camera isn't inside large objects
// Move camera further from scene center
engine.SetCameraPosition({15, 10, 15});
```

---

## 🖥️ System-Specific Issues

### 🐧 Linux Issues

**Missing Libraries:**
```sh
# Ubuntu/Debian
sudo apt install -y \
    cmake build-essential git pkg-config \
    libgl1-mesa-dev libglfw3-dev libglew-dev \
    libglm-dev libassimp-dev

# Fedora/RHEL
sudo dnf install cmake gcc-c++ git pkgconfig \
    mesa-libGL-devel glfw-devel glew-devel \
    glm-devel assimp-devel
```

**Graphics Driver Issues:**
```sh
# Check current driver
lspci -k | grep -A 2 -E "(VGA|3D)"

# NVIDIA proprietary drivers
sudo ubuntu-drivers autoinstall

# AMD open source drivers
sudo apt install mesa-vulkan-drivers
```

### 🍏 macOS Issues

**Homebrew Dependencies:**
```sh
# Install Homebrew if not present
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake git pkg-config glfw glew glm assimp

# Update if issues persist
brew update && brew upgrade
```

**Xcode Command Line Tools:**
```sh
# Install if missing
xcode-select --install

# Verify installation
xcode-select -p
# Should show: /Applications/Xcode.app/Contents/Developer
```

**OpenGL Compatibility:**
```sh
# macOS limits OpenGL to 4.1
# Engine requires 3.3+ so should work
# Check with:
system_profiler SPDisplaysDataType | grep OpenGL
```

---

## 🧑‍💻 Development Issues

### 🔗 Git Submodule Problems

**Symptoms:**
- Missing third_party directories
- ImGui or Jolt Physics not found
- Empty submodule directories

**Solutions:**
```sh
# Re-initialize all submodules
git submodule deinit --all
git submodule update --init --recursive

# Ensure ImGui is on correct branch
cd third_party/imgui
git checkout docking
git pull origin docking
cd ../..

# Verify submodules
git submodule status
# Should show commit hashes, not empty
```

### 📝 Build Script Issues

**Permission Denied:**
```sh
# Make scripts executable
chmod +x scripts/*.sh

# Check script permissions
ls -la scripts/
```

**Script Not Found:**
```sh
# Ensure you're in project root
pwd
ls scripts/
# Should show setup.sh, build.sh, dev.sh

# Run from correct directory
cd /path/to/BlacksiteEngine
./scripts/dev.sh quick
```

### 🖼️ Editor Panel Issues

**Panels Not Docking:**
- Drag panel tabs to dock them
- Use View menu to show/hide panels
- Reset layout by closing and reopening editor

**Inspector Not Updating:**
- Ensure entity is selected in Scene Hierarchy
- Click entity name to select it
- Check Console panel for errors

**Console Not Showing Logs:**
- Check log level filters
- Ensure auto-scroll is enabled
- Look for initialization errors

---

## 🐞 Debugging Techniques

### 📝 Enable Debug Logging

**In Code:**
```cpp
#define BS_DEBUG_ENABLED
#include "blacksite/Engine.h"
// More verbose logging will appear in console
```

**Check Log Output:**
```sh
# View recent logs
./scripts/dev.sh logs

# Monitor logs in real-time
tail -f logs/engine.log
```

### 📈 Performance Debugging

- Use Editor Performance Panel to monitor FPS and frame times
- Check entity and physics body counts
- Look for performance spikes

**System Monitoring:**
```sh
# Check CPU usage
top -p $(pgrep BlacksiteEngine)

# Check GPU usage (NVIDIA)
nvidia-smi

# Check memory usage
ps aux | grep BlacksiteEngine
```

### ⚙️ Physics Debugging

**Visual Inspection:**
- Use editor to select objects and check properties
- Monitor velocities in Inspector panel
- Check static vs dynamic state

**Console Commands:**
```sh
# In editor console
physics info          # Show physics statistics
reset physics         # Reset all physics bodies
spawn cube            # Test object creation
```

---

## 💡 Getting Help

### 📝 Information Gathering

**System Information:**
```sh
# Project status
./scripts/dev.sh info

# Dependency check
./scripts/dev.sh deps

# Recent logs
./scripts/dev.sh logs
```

**Engine Information:**
- Check Performance panel for entity counts
- Use Console panel to view real-time logs
- Monitor FPS and timing information

### 📨 Reporting Issues

When reporting problems, include:

1. **System Information:**
   - Operating system and version
   - Graphics card and driver version
   - Compiler version

2. **Build Information:**
   - Output of `./scripts/dev.sh info`
   - Build target (test/editor)
   - Debug or release build

3. **Error Details:**
   - Exact error messages
   - Steps to reproduce
   - Expected vs actual behavior

4. **Logs:**
   - Console output
   - Engine log files
   - Any crash dumps

### 📚 Resources

- **Main Documentation:** [API Overview](../api/overview.md)
- **Build System:** [Build Scripts Guide](../guides/build-scripts.md)
- **Physics Concepts:** [Physics-First Design](../guides/physics-first.md)
- **Examples:** [Code Examples](../guides/examples.md)

---

*Troubleshooting Guide for Blacksite Engine v0.2.0-alpha*
