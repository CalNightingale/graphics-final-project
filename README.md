# CS1230 Final Project: Minecraft-Style Terrain Generation!

![An example of generated terrain](readmebanner.png?raw=true)

This project is based on my (Cal's) implementation of Projects 5 & 6 for Brown's CS 1230, using the OpenGL Realtime pipeline

## How To Run
- As soon as you've cloned this repository, the code is ready to run in the Qt Editor. Open it in Qt like with any CS1230 project, and remember to change the working directory in Qt to the repository directory. After you press run, the application will open.
- TO CHANGE DEFAULT SHADER VALUE, simply replace the initialization on line 57 of realtime.cpp with whatever value you need
- Now you can play with the sliders and toggles on the left hand interface.
- Toon shading can be turned on/off and scaled via slider
- Seed, number of biomes, max height, and time of day can all be changed in real time via slider
- Fun camera views can be toggled with the interface

## Design Choices/Features
- This project showcases terrain generation using Voronoi diagrams and Simplex noise, rendered with cubes in realtime with a Skybox and non-photorealistic "toon shading" and includes features to modify the seed, terrain height, number of Voronoi cells (or biomes), and ability to view the terrain in fun ways
- We use an open-source Voronoi implementation (linked below) to partition a grid of cubes into biomes. The cells are spaced apart with Lloyd's relaxation algorithm and Simplex noise (explained below) is used to add variety to the boundaries (which would be completely straight otherwise). This all happens in realtime.cpp.
- We use Simplex noise, which has a similar purpose to Perlin noise but produces less grid-like artifacts, to produce a temperature and precipitation map covering the entire grid. For each biome cell, the average temperature and precipitation is used to determine its biome type (for instance a desert is hot and dry, while a rainforest is warm and wet. A different height map is generated for each biome type with Simplex noise, and are masked and summed onto the Voronoi cells to produce an overall height map for the entire terrain. The code for this functionality lives in the "noise" folder.
- To render the terrain, we convert heights into y-stretched cube primitives; this is different from games like Minecraft in which blocks across the y-axis are separate entities and is done for the sake of efficiency. For further efficiency, any blocks in ocean regions are not rendered and rather are replaced with a single flat, stretched out cube.
- We create the skybox by defining six planes to create a fixed box around the camera, and populate each plane with a texture of clouds generated with Simplex noise and/or stars generated with Uniform noise. The color of the sky and the visibility of the clouds/stars is determined with Hermite interpolation across a time variable that ranges from 1 to 5000. The code for the skybox lives in the skybox.cpp file.
- We create a Toon shading effect by modifying the phong shader to use the ceiling and floor of the angle between the normal and position-to-light vector to quantize ranges of light values into discrete buckets. The code for this lives in the resources/phong.frag file.
- Finally, we introduce two new modes for viewing the terrain: a "marvel beauty mode" that locks the camera into a circular path around the terrain, and a "1st-person walk mode" that locks the camera to "walk" along the surface of the terrain as defined by the height map. The code for these features lives in realtime.cpp.

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
Simplex noise generation, noise map population (temperature, precipitation, height, biomes, etc.), rendering efficiency optimizations
### [Logan](https://github.com/lmb2lmb)
Skybox (background image + movement, cloud generation, sun + movement)
