# 🚀 Blacksite Engine — Installation Guide

Welcome to the **Blacksite Engine** setup!  
This guide will help you get your development environment ready in minutes.

---

## 🖥️ System Requirements

**Supported Platforms**
- **Linux:** Ubuntu 20.04+ (recommended), other distros may work
- **macOS:** 10.15+ with Homebrew
- **Windows:** _Not yet supported_

**Hardware**
- **Graphics:** OpenGL 3.3+ compatible GPU
- **Memory:** 4GB RAM (8GB recommended)
- **Storage:** 2GB free space

---

## ⚡ Automatic Setup (Recommended)

The fastest way to get started is with our setup script:

```bash
git clone https://github.com/MintBlaster/Blacksite.git
cd Blacksite
./scripts/setup.sh
```

This script will:
- Install system dependencies
- Initialize git submodules (ImGui, Jolt Physics)
- Create build directories
- Test the initial build
- Make all scripts executable

---

## 🛠️ Manual Setup

If the automatic setup doesn't work, follow these steps:

### 1. Install System Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install -y \
  cmake build-essential git pkg-config \
  libgl1-mesa-dev libglfw3-dev libglew-dev libglm-dev libassimp-dev
```

**macOS (with Homebrew):**
```bash
brew install cmake git pkg-config glfw glew glm assimp
```

### 2. Initialize Submodules

```bash
git submodule update --init --recursive
# Switch ImGui to docking branch (required for editor)
cd third_party/imgui
git checkout docking
cd ../..
```

### 3. Create Build Directory

```bash
mkdir -p build
```

### 4. Test Build

**Engine test:**
```bash
./scripts/build.sh --target test --run
```
**Editor:**
```bash
./scripts/build.sh --target editor --run
```

---

## ✅ Verification

After setup, verify everything works:

### 1. Check Dependencies

```bash
./scripts/dev.sh deps
```

### 2. Build and Run Engine Test

```bash
./scripts/dev.sh quick
```
You should see:
- Window titled "Blacksite Engine"
- Colored cube falls and hits a gray plane
- Physics simulation at 60 FPS
- Press **ESC** to exit

### 3. Build and Run Editor

```bash
./scripts/dev.sh editor
```
You should see:
- Same physics scene as above
- Press **F1** to show editor panels
- Scene Hierarchy, Inspector, Console, and Performance panels
- Select and edit entities in real time

---

## 🧑‍💻 Development Environment

### Recommended IDE Setup

**Visual Studio Code**
```bash
code --install-extension ms-vscode.cpptools
code .
```

**CLion**
- Open the project root directory
- CLion should auto-detect `CMakeLists.txt`
- Set build directory to `build/`

### Git Configuration

Set up a pre-commit hook for code formatting:
```bash
echo "./scripts/dev.sh format" > .git/hooks/pre-commit
chmod +x .git/hooks/pre-commit
```

---

## 🩺 Troubleshooting

### Graphics Issues

**Black screen or OpenGL errors?**
```bash
glxinfo | grep "OpenGL version"
# Should show version 3.3 or higher
```
**Missing graphics drivers?**
```bash
sudo apt install mesa-utils
# Update GPU drivers via system settings
```

### Build Issues

**CMake not found?**
```bash
# Ubuntu
sudo apt install cmake
# macOS
brew install cmake
cmake --version  # Should be 3.16+
```

**Missing GLFW/GLEW?**
```bash
# Ubuntu
sudo apt install libglfw3-dev libglew-dev
# macOS
brew install glfw glew
pkg-config --modversion glfw3 glew
```

**Submodule issues?**
```bash
git submodule deinit --all
git submodule update --init --recursive
cd third_party/imgui
git checkout docking
git pull origin docking
```

### Permission Issues

**Scripts not executable?**
```bash
chmod +x scripts/*.sh
```

**Build directory permissions?**
```bash
sudo chown -R $USER:$USER build/
```

---

## ⏭️ Next Steps

1. **Read the Quick Start:** [Quick Start Guide](./quick-start.md)
2. **Understand the Build System:** [Build Scripts Guide](../guides/build-scripts.md)
3. **Explore the API:** [Engine Core Documentation](../api/engine-core.md)

---

## 🆘 Getting Help

If you encounter issues:
1. **Check logs:** `./scripts/dev.sh logs`
2. **Verify dependencies:** `./scripts/dev.sh deps`
3. **Clean rebuild:** `./scripts/dev.sh clean && ./scripts/dev.sh setup`
4. **Review troubleshooting:** [Troubleshooting Guide](./troubleshooting.md)

---

*Installation guide for Blacksite Engine v0.2.0-alpha*
