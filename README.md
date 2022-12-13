# CS1230 Final Project: Minecraft-Style Terrain Generation!

This project is based on my implementation of Projects 5 & 6 for this class, using the OpenGL Realtime pipeline

## Design Choices
- TO CHANGE DEFAULT SHADER VALUE, simply replace the initialization on line 57 of realtime.cpp with whatever value you need
- Ocean renders and vertical stacks of blocks render as singular blocks for efficiency
- Toon shading can be turned on/off and scaled via slider
- Seed, number of biomes, max height, and time of day can all be changed in real time via slider

## Known Bugs
None

## If We Had More Time
- Implement shadows
- Add procedural structures (trees, villages, etc.)
- Replace solid color blocks with textures

## Credits
### [Brian](https://github.com/Brian-han77)
Toon shading, view modes (player mode, presentation mode, etc.)
### [Cal](https://github.com/CalNightingale)
Voronoi implementation (using https://github.com/JCash/voronoi), Biome generation & Assignment
(based on [this article](https://towardsdatascience.com/replicating-minecraft-world-generation-in-python-1b491bc9b9a4))
### [Dustin](https://github.com/dustin-wu)
Simplex noise generation, noise map population (temperature, precipitation, height, clouds, etc.), day/night cycling
### [Logan](https://github.com/lmb2lmb)
Skybox (background image + movement, cloud generation, sun + movement)
