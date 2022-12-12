#version 330 core

// Grab inputs
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;

// Declare outputs for frag shader
out vec3 worldPos;
out vec3 worldNorm;

uniform mat4 model;
uniform mat3 it_model;

// Uniforms for view and projection matrices
uniform mat4 m_view;
uniform mat4 m_proj;

void main() {
    // copy position and normal for shipping to frag shader
//    worldPos = vec3(pos);
//    worldNorm = vec3(norm);

    worldPos = vec3(model * vec4(pos, 1.0));
    worldNorm = vec3(vec4(it_model * norm,  0.0));

    // Set gl_Position to the object space position transformed to clip space
    gl_Position = m_proj * m_view * (vec4(worldPos, 1));
}
