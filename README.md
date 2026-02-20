
# JoJoAPI

JoJoAPI is a modding API for Jojo's Bizarre Adventure All-Star Battle R.
It's in a very early stage of development, but it's already possible to create mods with it! (And i'll try to make the API as stable as possible, so you don't have to worry about your mods breaking with every update)

## Installation

1. Download the latest release of `JAPILauncher.exe` from the [releases page](https://github.com/Kapilarny/JAPI/releases)
2. Drag&drop the file into your game folder (where the `ASBR.exe` is located)
3. Run `JAPILauncher.exe` whenever you want to play the game with JAPI (you can also create a shortcut for it)
4. Enjoy!

## Documentation

The documentation is currently a work in progress, although you can ask me any questions on our [modding Discord](https://discord.jojomodding.com)

## TODO's
- [ ] Namespace library headers (e.g. `jconfig/config.h` instead of `config.h`)

## Donations

Huge thanks to these people for supporting this project:

- Hydra
- Damn.Broh
- Jake
- Moeru
- justcamtro

If you also want to support me, you can donate to me via [PayPal](https://paypal.me/kapilarny) or [Ko-fi](https://ko-fi.com/kapilarny)

## Dependencies

Thanks to these, JojoAPI is possible:

- [MinHook](https://github.com/TsudaKageyu/minhook) - A minimalistic x86/x64 API hooking library for Windows
- [toml++](https://github.com/marzer/tomlplusplus) - Header-only TOML config file parser and serializer for C++17
- [nlohmann/json](https://github.com/nlohmann/json) - Header-only JSON library
- [kiero](https://github.com/Rebzzel/kiero/) - Universal graphical hook for a D3D9-D3D12, OpenGL and Vulkan based games
- [Dear ImGui](https://github.com/ocornut/imgui) - Bloat-free graphical user interface library for C++
- A lot of coffee
