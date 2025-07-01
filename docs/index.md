# 🕹️ Blacksite Engine Documentation

Welcome to the Blacksite Engine documentation! Blacksite is a **physics-first C++ game engine** with a real-time editor and modern workflow.

## 📖 Quick Navigation

### 🚀 Getting Started
- [Installation Guide](./getting-started/installation.md) — Set up your development environment
- [Quick Start](./getting-started/quick-start.md) — Your first physics scene in 5 minutes
- [Troubleshooting](./getting-started/troubleshooting.md) — Common issues and solutions

### 📚 API Reference
- [API Overview](./api/overview.md) — Core concepts and design philosophy
- [Engine Core](./api/engine-core.md) — Main Engine class and lifecycle
- [Entity System](./api/entity-system.md) — Fluent API for object manipulation
- [Physics System](./api/physics.md) — Jolt Physics integration
- [Editor Interface](./api/editor.md) — Real-time physics editor

### 🎯 Guides & Tutorials
- [Physics-First Design](./guides/physics-first.md) — Understanding our core philosophy
- [Build System](./guides/build-scripts.md) — Development workflow and scripts
- [Code Examples](./guides/examples.md) — Complete working examples
- [Project Architecture](./guides/architecture.md) — How everything fits together

### 🔧 Reference
- [Build Targets](./reference/build-targets.md) — CMake targets and options
- [Dependencies](./reference/dependencies.md) — Required libraries and versions
- [Changelog](./reference/changelog.md) — Version history and breaking changes

## 🏗️ Project Status

- **Version:** 0.2.0-alpha
- **Status:** Experimental — suitable for learning and prototyping
- **Platform:** Linux/macOS (Windows not yet supported)

### ✅ What's Working
- Complete physics simulation with Jolt Physics
- Real-time editor with ImGui panels
- Static/dynamic body switching
- Force application and velocity control
- Modular project structure
- Automated build system

### 🚧 What's Missing
- Input handling (mouse/keyboard controls)
- Asset loading (textures, models)
- Audio system
- Advanced rendering features
- Cross-platform Windows support

## ⚡ Quick Start Commands

**First time setup:**
```sh
./scripts/setup.sh
```

**Run physics editor (most common):**
```sh
./scripts/dev.sh editor-fast
```

**Run engine test:**
```sh
./scripts/dev.sh quick
```

**Clean everything and rebuild:**
```sh
./scripts/dev.sh clean && ./scripts/dev.sh editor
```

## 🧭 Learning Path

1. **Start Here:** [Installation Guide](./getting-started/installation.md)
2. **Build Your First Scene:** [Quick Start](./getting-started/quick-start.md)
3. **Understand the Philosophy:** [Physics-First Design](./guides/physics-first.md)
4. **Explore the API:** [Engine Core](./api/engine-core.md)
5. **Use the Editor:** [Editor Interface](./api/editor.md)
6. **Build Complex Scenes:** [Code Examples](./guides/examples.md)

---

*This documentation covers Blacksite Engine v0.2.0-alpha. For the latest updates, see our [Changelog](./reference/changelog.md).*
