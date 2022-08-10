# OpenField

OpenField (Soon to be FreeField, Maybe) is modular drill design software made for all types of bands.

**Don't expect much of this project as it is in early development.**

Currently supports Windows - with macOS and Linux support planned. Setup scripts support Visual Studio 2022 by default.

## Requirements for building
- [Visual Studio 2022](https://visualstudio.com) (not strictly required, however included setup scripts only support this)
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows) (preferably a recent version)

## Getting Started
Once you've cloned, run `scripts/Setup.bat` to generate Visual Studio 2022 solution/project files.

### 3rd party libaries
- [Walnut](https://github.com/TheCherno/Walnut) (project forked from)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [GLFW](https://github.com/glfw/glfw)
- [stb_image](https://github.com/nothings/stb)
- [GLM](https://github.com/g-truc/glm) (included for convenience)

### Additional
- Walnut uses the [Roboto](https://fonts.google.com/specimen/Roboto) font ([Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0))
