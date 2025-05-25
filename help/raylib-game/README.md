# raylib-game

## Overview
This project is a simple 3D game built using the Raylib library. It features player movement, block manipulation, and networking capabilities for multiplayer gameplay.

## Project Structure
```
raylib-game
├── src
│   ├── main.c          # Entry point of the application
│   ├── game            # Contains game-related functionalities
│   │   ├── player.c    # Player movement and state management
│   │   ├── player.h    # Player-related declarations
│   │   ├── editor.c    # Editor functionalities for block manipulation
│   │   ├── editor.h    # Editor-related declarations
│   │   ├── block.c     # Block creation and collision detection
│   │   └── block.h     # Block-related declarations
│   ├── network         # Networking functionalities
│   │   ├── network.c   # Hosting and joining games
│   │   └── network.h   # Networking-related declarations
│   └── utils           # Utility functions and headers
│       ├── config.h    # Configuration constants
│       ├── debug.c     # Debugging functions
│       └── debug.h     # Debugging-related declarations
├── Makefile            # Build instructions
├── LICENSE             # Licensing information
└── README.md           # Project documentation
```

## Setup Instructions
1. Clone the repository:
   ```
   git clone <repository-url>
   cd raylib-game
   ```

2. Install Raylib:
   Follow the installation instructions for Raylib based on your operating system.

3. Build the project:
   ```
   make
   ```

4. Run the game:
   ```
   ./raylib-game
   ```

## Usage
- Use the keyboard to control the player and interact with the game world.
- Enter editor mode to place and manipulate blocks.

## Contributing
Contributions are welcome! Please submit a pull request or open an issue for any suggestions or improvements.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.