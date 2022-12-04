# Projects 5 & 6: Lights, Camera & Action!

All project handouts can be found [here](https://cs1230.graphics/projects).

# Design Choices
- Reuse single vbo for all shapes for memory efficiency
- Enumerated light types and passed this information to the shader to enable handling different light types
- Assume on any setting change that the scene must be fully re-rendered
- Created separate classes for all shape types to facilitate reuse of Lab 8 code
- Created Shape struct containing RenderShapeData and vertex data. Useful to store a list of this for all objects
in the scene because when iterating through and rendering both are needed (material properties must be passed to shader)
- TO CHANGE DEFAULT SHADER VALUE, simply replace the initialization on line 54 of realtime.cpp with whatever value you need
- Implemented the inversion pixel-based filter, and the blur kernel-based filter

# Known Bugs
None

# Extra Credit
None
