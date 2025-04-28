## WARNING
I'll **not** accept pull requests, due to licensing issues and due to transient nature of this repo.

# Project Leiden
WIP game engine focused on driving stuff. Future platform for some games I'm planning to work with. This is a personal project, intended for personal use. Hopefully this also helps someone to learn something.

This project is not intended for use in production. It's not feature-complete, it's unstable (despite what the branch name says), and poorly optimized. The repo also don't have the required assets nor required dependencies. It won't work out-of-the-box. This is probably not the final repository where this project will sit. This is a temporary repository that will exist until I have a final name and most of the API is done. It's safe to say it's just a code dump.

The goal is to make a library with all the functionalities I need to make racing and car games. The library should care of all the windowing, asset loading, graphics, audio and physics, leaving the programmer only with an API similar to raylib and similar libraries, with a focus on 3D and vehicular physics. Right now, the library is... not actually a library, there's a main.c that exists for testing (and will be there until everything works as intended).

The name Project Leiden is a codename. Most of my projects use a fictional location as codename before getting a final name (because I'm bad at naming things). [Leiden](https://violet-evergarden.fandom.com/wiki/Leiden) is a major location in [Violet Evergarden](https://en.wikipedia.org/wiki/Violet_Evergarden)'s universe.

## It's not portable yet.
- **Linux**: tried only on Debian-based distros. Works fine on them.
- **GhostBSD**: audio module gave me some issues, but a little workaround fixed it. It *might* work on other *BSD systems but not sure.
- **Windows**: worked fine like Linux version when built with MinGW-W64. It doesn't build under MSVC due to some platform-specific things, but fixing it is not a priority right now.
- **MacOS**: no access to Apple hardware. No support for SPIR-V. Workaround is a lot of work. I don't plan to support it.
- **iOS**: same issues as MacOS. No plans to support.
- **Android**: too much work to get it working. Graphics module might explode. I would like to support but not right now.

## About the submodules
- **audio**: the troublesome module, it's intended to manage spatial sound and music. Uses miniaudio.h. It's still barebones with only a few test functions done. Might get re-implemented soon.
- **data**: helper structures intended for later use. As of now, only hashtable implemented.
- **demos**: initially intended to test stuff. It's now obsolete and will be removed.
- **filesystem**: manages filesystem access, also includes an INI parser. Uses miniphysfs behind the scenes.
- **graphics**: the most worked on module, deal with the rendering. Uses SDL GPU as the graphics API (used OpenGL in the past). The goal is to make generic graphics functionalities, but the plan is to focus on NPR (anime-style and cartoon-ish style) and retro-style graphics (PS1 and PS2 kind of graphics). It has no PBR support nor scene graphs/trees. The goal is simplicity.
- **input**: manages input. Still barebones, keyboard and some mouse works fine. The goal is to support gamepads and, if possible, steering wheels.
- **leiden**: the glue that joins every other module, so I can use a single header.
- **linmath**: math library.
- **physics** is a planned submodule that will use either JoltC or ODE to handle 3D physics.
- **vehicle** will be either a future separate submodule on top of physics or an extension to it that will handle car stuff.

## Licence
It's `GPL-3.0-only` for now. I might change it when it becomes close to feature complete. There's a long road until then.
