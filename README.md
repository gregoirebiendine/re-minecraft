# Farfield

Personal reproduction of Minecraft with a custom C++/OpenGL engine. \
Farfield aims to completely modernize Minecraft architecture, while learning major OOP patterns.

## Targeted Features (Roadmap)

- Infinite procedural terrain generation with seed
- Multi-threaded architecture
- Supports for modern rendering solutions such as MSAA, AO, ...
- Entity Component System (ECS) architecture
- Prefab/schematic system for structures
- Fully integrated modloader with shader supports
- _Multiplayer supports (currently not planned)_

## Dependencies

- **CMake** 3.30+
- **C++26** compatible compiler (GCC/MinGW)
- **just** command runner
- Internal libs such as GLFW3 are bundled in the lib/ folder

## Building

Farfield uses [just](https://github.com/casey/just) as its task runner. Install it first, then use the commands below.

### Windows

**Prerequisites:**
- MinGW/GCC with C++26 support
- CMake 3.30+
- [just](https://github.com/casey/just)
- [watchexec](https://github.com/watchexec/watchexec) (for hot reload)

### Linux

**Prerequisites:**
- GCC with C++26 support
- CMake 3.30+
- [just](https://github.com/casey/just)
- [entr](https://github.com/eradman/entr) (for hot reload)

### Commands

```bash
# Build the project
just build

# Build and run
just run

# Hot reload (rebuild on file changes)
just debug

# Clean build artifacts
just clean
```

### Command Aliases

| Alias | Command |
|-------|---------|
| `just b` | `just build` |
| `just r` | `just run` |
| `just d` | `just debug` |
| `just c` | `just clean` |

## Authors

[@Melio](https://github.com/gregoirebiendine) \
[@Tech0ne](https://github.com/Tech0ne)

## License

Minecraft is a trademark of Mojang Studios / Microsoft. All game concepts and original assets belong to Mojang. \
Textures are provided by the [Faithful](https://faithfulpack.net/) team under their own license. \
Engine code is authored by the contributors listed above. All rights reserved.
