# Mapster

Mapster is a game originally created by a 15-year-old for Arduino, now adapted for the Nintendo Switch, PC, and more using the C language upon turning 16.

## Installation

### Pre-built Release

Download the `mapster.nro` file from the releases section and copy it to `/switch/` on your modded Switch memory card.

### Compile It Yourself

1. Install [DevkitPro](https://devkitpro.org/). Make sure to select the Switch option during installation.
2. Open MSYS2 or any terminal emulator of your choice.
3. Navigate to the directory where you extracted the source code.
4. Run the following command: make

## Controls

- **Up/Down/Left/Right:** Move player character
- **A:** Interact with a map element
- **Plus (+):** Quit the game
- **Save/Load Tile:** Interact with this tile to save or load your progress to/from `sdmc:/switch/mapster.bin`

## Contribution

Contributions are greatly appreciated! Feel free to fork this repository and submit pull requests with your enhancements or bug fixes.

## Credits

- **Code:** AzizBgBoss
- **Sprites:** AzizBgBoss
- **Music:** Taken from `switch-examples/graphics/sdl2-demo`