# SEAgregate

A 2D waste-segregation game built with **C++17** and **SFML 3**. Trash bags pile up in our waters, and your job is to sort them correctly before time runs out. Choose a level (**Lake**, **River**, or **Ocean**) and race the clock.

The game teaches the four waste categories used in segregation:

| Category | Bag asset | Bin asset |
| --- | --- | --- |
| Biodegradable | `bioBag.png` | `bio.png` |
| Non-biodegradable | `nonBag.png` | `non.png` |
| Recyclable | `recBag.png` | `rec.png` |
| Hazardous | `hazBag.png` | `haz.png` |

## Features

- Main menu with Play button, plus "Our Mission", "How to Play", and "Good to Know" sections
- Level selection screen with a back button
- Three levels with their own backgrounds: **Lake**, **River**, **Ocean**
- Mouse-controlled gameplay
- Countdown timer, with the remaining time shown on the win screen
- Game Over screen that shows your stats
- Game Won screen
- Fixed 1920x1080 game resolution that scales with the window (default window is 1280x720)

## Game Flow

```
Main Menu -> Level Selection -> Lake / River / Ocean -> Game Over or Game Won -> Level Selection
```

## Controls

| Input | Action |
| --- | --- |
| Left Mouse Button | Click buttons and interact with trash bags in-game |
| Mouse Movement | Tracked live every frame during gameplay |
| `Esc` | Close the game |

## Requirements

- Windows (the CMake script copies the SFML DLLs on Windows)
- A C++17 compiler (MSVC, MinGW, etc.)
- [CMake](https://cmake.org/) 3.24 or newer
- [SFML 3.x](https://www.sfml-dev.org/download.php) (SFML 2.x will not work)

## Setup and Building

1. **Install SFML 3** and extract it to `C:/SFML`.
   The `CMakeLists.txt` expects the CMake config files at:

   ```
   C:/SFML/lib/cmake/SFML
   ```

   If you installed SFML somewhere else, edit this line in `CMakeLists.txt`:

   ```cmake
   set(SFML_DIR "C:/SFML/lib/cmake/SFML")
   ```

2. **Clone the repository**

   ```bash
   git clone <your-repo-url>
   cd <your-repo-folder>
   ```

3. **Configure and build**

   ```bash
   cmake -S . -B build
   cmake --build build
   ```

4. **Run**

   The executable is named `SeagregateMenu`. Run it from the build folder (for example `build/Debug/SeagregateMenu.exe` with Visual Studio generators, or `build/SeagregateMenu.exe` with Ninja/MinGW).

### Automatic asset and DLL copying

After each build, CMake automatically:

- Copies every game image listed in `ALL_GAME_ASSETS` from the project folder into the build folder (only if changed)
- Copies the required SFML runtime DLLs next to the executable (Windows only)

So you don't need to copy files by hand. If you add a new image, add its filename to the `ALL_GAME_ASSETS` list in `CMakeLists.txt`.

> **Using CLion or Visual Studio?** Open the project folder as a CMake project and build normally. Just make sure the working directory is the build folder so the game can find its images.

## Project Structure

| File | Purpose |
| --- | --- |
| `main.cpp` | Window setup, event loop, and game state machine |
| `Menu.hpp` | Main menu screen |
| `SelectLevel.hpp` | Level selection screen |
| `LakeLevel.hpp` | Lake level |
| `RiverLevel.hpp` | River level |
| `OceanLevel.hpp` | Ocean level |
| `TrashBag.hpp` | Trash bag object used in the levels |
| `GameOverScreen.hpp` | Game over screen and stats |
| `GameWonScreen.hpp` | Victory screen |
| `CMakeLists.txt` | Build configuration and asset/DLL deployment |

### Game States

`main.cpp` runs a simple state machine:

`MainMenu`, `LevelSelection`, `LakeGameplay`, `RiverGameplay`, `OceanGameplay`, `GameOver`, `GameWon`

Each level's `update()` returns a status code:

| Return value | Meaning |
| --- | --- |
| `-1` | Player lost (go to the Game Over screen) |
| `2` | Player won (go to the Game Won screen) |
| anything else | Game continues |

### Assets

All images live in the project root and are copied to the build folder:

- **UI:** `Dashboard.png`, `SelectLevel.png`, `play_level.png`, `SEAgregate_Logo.png`, `Play_Button.png`, `Our_mission_sprite.png`, `How_to_play_sprite.png`, `Good_to_know_sprite.png`
- **Level backgrounds:** `lake.png`, `river.png`, `ocean.png`
- **End screens:** `GameOver.png`, `GameWon.png`
- **Trash bags:** `bioBag.png`, `nonBag.png`, `recBag.png`, `hazBag.png`
- **Bins:** `bio.png`, `non.png`, `rec.png`, `haz.png`

## Troubleshooting

- **`Could not find SFML`:** check that `SFML_DIR` in `CMakeLists.txt` points to the folder containing `SFMLConfig.cmake`, and that you downloaded SFML **3.x**.
- **Missing DLL error on launch:** rebuild so the post-build step copies the DLLs, or copy them from `C:/SFML/bin` next to the executable.
- **Blank or missing images:** make sure the image files exist in the project root and appear in `ALL_GAME_ASSETS`, then rebuild.
- **Compiler and SFML mismatch:** use the SFML build that matches your compiler (MSVC vs MinGW).

## Screenshots

Add screenshots here, for example:

```markdown
![Main Menu](screenshots/menu.png)
```

## Author

**wyne4rest-afk**
GitHub: [github.com/wyne4rest-afk](https://github.com/wyne4rest-afk)

## License

Add a license (for example MIT) and include a `LICENSE` file.