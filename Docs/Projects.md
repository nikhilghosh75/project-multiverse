Project Multiverse has a lot of projects contained within it.

# General
The following projects are general to the game or editors.

## Utils
Utils is a project that contains any general purpose code that should be shared between the game and its tools. It‘s intended mostly for math functionality (i.e. Rects, Matrices, Randomness) but can have other uses too, such as a Binary File Stream. No game-specific logic should go in this project.

## ApplicationCore
ApplicationCore contains the core functionality for opening a Vulkan application. This is used by the game, but can also be used by different tools that need visual interfaces. Not all tools will need these (i.e. pure command line build and converter tools likely won't use them)

## Rendering
Rendering contains the various utilities for rendering things within the application. This includes the rendering of images and fonts

# Game
The game project contains all of the game logic.