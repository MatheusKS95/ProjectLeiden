## ⚠️ WARNING
I'll **not** accept pull requests, due to licensing issues and due to transient nature of this repo.

# Project Leiden
This is a WIP rendering project with some game development extras. Future platform for some games I'm planning to work with. This is a personal project, intended for personal use. Hopefully this also helps someone to learn something.

This project is not intended for use in production. It's not feature-complete, it's unstable and poorly optimized. The repo also don't have the required assets nor required dependencies. It won't work out-of-the-box. This is probably not the final repository where this project will sit. This is a temporary repository that will exist until I have a final name and most of the API is done. It's safe to say it's just a code dump.

The goal is to have a codebase I can reuse for some games I plan to develop. The whole project should care of all the windowing, asset loading, graphics, audio and physics. 

The name Project Leiden is a codename. Most of my projects use a fictional location as codename before getting a final name (because I'm bad at naming things). [Leiden](https://violet-evergarden.fandom.com/wiki/Leiden) is a major location in [Violet Evergarden](https://en.wikipedia.org/wiki/Violet_Evergarden)'s universe.

## ⚙️ Portability Status
- **Linux**: Tested on Debian-based distributions. Works fine.
- **GhostBSD**: The audio module had issues, but a workaround solved it. Might work on other *BSD systems, but untested.
- **Windows**: Works similarly to the Linux version when built with MinGW-W64. Does **not** build with MSVC due to some platform-specific code. Fixing this is currently low priority.
- **MacOS**: No access to Apple hardware. No SPIR-V support. Making it work is [troublesome](https://github.com/libsdl-org/SDL/blob/main/docs/README-macos.md). No plans for support.
- **iOS**: Same issues as MacOS. No plans to support.
- **Android**: Too much work to get it running. The graphics module might [not work at all](https://github.com/libsdl-org/SDL/issues/12652). Support is unlikely.

## 📦 About the submodules
- **audio**: Manages spatial sound and music using `miniaudio.h`. Still very basic with only a few test functions. May be re-implemented.
- **data**: Provides helper structures for future use. Currently includes a hashtable implementation.
- **filesystem**: Handles file access and includes an INI parser. Uses `miniphysfs` internally.
- **graphics**: The most developed module. Manages rendering via SDL GPU API (previously used OpenGL). Designed for general rendering but with a focus on NPR (non-photorealistic rendering—anime/cartoon styles) and retro aesthetics (e.g., PS1/PS2 era). No PBR or scene graph support. Simplicity is the goal.
- **input**: Manages input. Currently basic—supports keyboard and limited mouse input. Gamepad and steering wheel support are planned.
- **leiden**: The central glue module that aggregates the others into a single header. May be renamed in the future.
- **linmath**: A simple math library.
- **physics**: A planned module. Will likely use either JoltC or ODE for 3D physics. Not a current focus.

## 📝 Licence
Currently licensed under `GPL-3.0-only`. This may change when the project becomes more feature-complete. There’s a long road ahead before that happens.
