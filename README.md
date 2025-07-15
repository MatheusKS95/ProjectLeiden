## ⚠️ WARNING
I'll **not** accept pull requests, due to licensing issues and due to the experimental, learning nature of this repo.

# Project Leiden
This is a WIP rendering project with some game development extras. I created this for learning purposes and for testing stuff.

This project is not intended for use in production. It's not feature-complete, it's unstable and poorly optimized. The repo also don't have the required assets nor required dependencies. It won't work out-of-the-box. This is probably not the final repository where this project will sit. It's safe to say it's just a code dump.

The name Project Leiden is a codename. Most of my projects use a fictional location as codename before getting a final name (because I'm bad at naming things). [Leiden](https://violet-evergarden.fandom.com/wiki/Leiden) is a major location in [Violet Evergarden](https://en.wikipedia.org/wiki/Violet_Evergarden)'s universe.

## ⚙️ Portability Status
- **Linux**: Tested on Linux Mint. Works fine.
- **GhostBSD**: Worked previously, unsure now. New testing is required.
- **Windows**: Works fine with both MinGW-W64 and MSVC, but runs as Vulkan-only.
- **MacOS**: No access to Apple hardware. No SPIR-V support. Making it work is [troublesome](https://github.com/libsdl-org/SDL/blob/main/docs/README-macos.md). No plans for support.
- **iOS**: Same issues as MacOS. No plans to support.
- **Android**: As of today, it [doesn't work at all, due to poor Vulkan support on most Android devices](https://github.com/libsdl-org/SDL/issues/12652#issuecomment-2755770682). Support is currently unfeasible. Also, no plans for OpenGL ES graphics.
- **Consoles**: Most are proprietary platforms incompatible with the GPL and/or the current GPU API. Some homebrew ones are possible, but require all rendering code to be reworked from scratch. Steam Deck is the closest thing to a video game console that can run this project as is.

## 📦 About the dependencies
- **SDL3**: Used for most middleware, windowing and GPU API (currently Vulkan-only). [Link](https://github.com/libsdl-org/SDL).
- **SDL3_image**: Used for loading images (textures used for testing are in PNG and QOI format). [Link](https://github.com/libsdl-org/SDL_image).

## 📺 Rendering
Currently, only a basic cel-shaded pipeline is being worked on. It is based on how classic cel shading works, relying on textures to achieve effects such as lights and shadows. I plan to work on a 5th-gen pipeline as well (PS1/N64/Saturn). No plans for more advanced rendering - it only has one texture per mesh (no normals, no specular, no PBR materials).

## 📝 Licence
Currently licensed under `GPL-3.0-only`. This may change when the project becomes more feature-complete. There’s a long road ahead before that happens.
