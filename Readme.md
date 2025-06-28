A modular, physics-first game engine for C++ — built for real-time destruction, vehicles, and dynamic worlds.


# Blacksite Engine

**Blacksite** is a C++ game engine focused on real-time physics, destruction, and simulation. Designed for developers who want a "physics-first" experience — where every object is dynamic by default, destruction is built-in, and gameplay emerges from interaction.

## Features

- **Physics-First Architecture**  
  Bullet Physics is core, not an afterthought — simulate rigidbodies, collisions, and dynamic environments out of the box.

- **Modern Graphics**  
  OpenGL rendering with a clean pipeline, shader support, and GLM for math utilities.

- **Modular Design**  
  Clear separation of core systems: rendering, physics, input, asset loading.

- **Cross-Platform**  
  Works on Linux, Windows, and macOS with minimal setup.

- **Clean API**  
  Simple interfaces to spawn objects, apply forces, simulate vehicles, and more.

## Planned API Design

```cpp
#include <blacksite.h>

int main() {
    auto engine = Blacksite::Engine();
    auto scene = engine.CreateScene();
    
    auto car = scene.CreateVehicle("car.obj");
    car.SetPosition({0, 1, 0});
    
    auto ground = scene.CreatePlane();
    ground.SetMaterial(Material::ASPHALT);
    
    return engine.Run();
}
```

> **Note:** This is the planned API design. The engine is currently in early development.

## Current Status

**⚠️ Early Development**  
Core systems (graphics + physics) are being prototyped. Expect minimal functionality, no stable API yet.


## Why Physics-First?

Unlike traditional engines where physics is optional, **Blacksite** treats every object as physical by default. This enables:

- Rapid prototyping of destruction-heavy mechanics
- Realistic vehicle and environment simulation
- Cleaner APIs for building interactive worlds


## Contributing

The engine is in early development. If you're interested in contributing to a physics-first game engine, feel free to open an issue to discuss ideas or reach out about specific systems you'd like to work on.

Areas that will need attention:
- Core architecture design
- Graphics pipeline optimization
- Physics system integration
- Cross-platform compatibility
- Documentation and examples

## License

MIT License  
Use it. Break it. Build something wild with it.