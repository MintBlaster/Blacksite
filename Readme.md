# Blacksite Engine

A modern graphics and physics engine built in C++ for game development.

## Features

- **Graphics**: OpenGL-based rendering with modern shader support
- **Physics**: Bullet Physics integration for realistic simulations  
- **Cross-platform**: Linux, Windows, macOS support
- **Modular**: Clean separation of graphics, physics, input, and core systems

## Dependencies

- OpenGL 3.3+
- GLFW 3.x
- GLEW
- GLM (OpenGL Mathematics)
- Bullet Physics
- Assimp (3D model loading)

## Building on Arch Linux

### Install Dependencies
```bash
sudo pacman -S base-devel cmake git
sudo pacman -S mesa vulkan-devel glfw-x11 glew glm bullet assimp
```

### Build
```bash
mkdir build && cd build
cmake ..
make
./blacksite_test
```

## Project Structure

```
blacksite/
├── include/blacksite/     # Public headers
│   ├── core/             # Engine core systems
│   ├── graphics/         # Rendering components
│   ├── physics/          # Physics integration
│   └── input/            # Input handling
├── src/                  # Implementation files
├── shaders/              # GLSL shaders
├── assets/               # Game assets
└── examples/             # Usage examples
```

## Status

🚧 **In Development** - Basic setup complete, implementing core systems

## Goals

- [ ] Core engine architecture
- [ ] Basic 3D rendering pipeline
- [ ] Physics world integration
- [ ] Input system
- [ ] Asset loading
- [ ] Scene management
- [ ] Audio system

## Contributing

This is a learning project, but contributions and suggestions are welcome!

## License

MIT License - See LICENSE file for details