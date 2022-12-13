#version 330 core

// Input position and normal from vertex shader
in vec3 worldPos;
in vec3 worldNorm;

// Output color vector
out vec4 fragColor;

// Global uniforms
uniform float ka;
uniform float kd;
uniform float ks;
uniform vec3 cameraPos;

// Material property uniforms
uniform vec4 amb;
uniform vec4 diff;
uniform vec4 spec;
uniform float shininess;

uniform int toonCount;
uniform float toonIncrements;
bool toonOn;

// Lights
uniform vec4 lightIntensities[8];
uniform vec3 lightDirs[8]; // directions TO lights FROM point
uniform vec3 lightPositions[8];
uniform vec3 lightFunctions[8];
uniform float lightAngles[8];
uniform float lightPenumbras[8];
// LIGHT TYPE ENUMERATION: 1 = directional, 2 = point, 3 = spot, 4 = area
uniform int lightTypes[8];

// helper method to compute reflection
vec3 computeReflect(vec3 u, vec3 v) {
    return normalize(2.f * u * dot(v, u) - v);
}

float computeAttenuation(float dist, vec3 func) {
    return min(1.0, 1.0 / (func[0] + func[1]*dist + func[2]*dist*dist));
}

void main() {
    //fragColor = vec4(abs(worldNorm), 1); // UNCOMMENT TO TEST WITH NORMALS AS COLORS

    // add ambient term
    fragColor = ka * amb;
    vec3 dirToCam = cameraPos - worldPos;

    // loop over lights
    for (int i = 0; i < 8; i++) {
        float att = 1.0;
        float falloff = 1.0;
        vec3 dirToLight;
        if (lightTypes[i] == 1) {
            // directional
            dirToLight = -lightDirs[i];
        } else if (lightTypes[i] == 2) {
            // point
            dirToLight = lightPositions[i] - worldPos;
            att = computeAttenuation(distance(worldPos, lightPositions[i]), lightFunctions[i]);
        } else if (lightTypes[i] == 3) {
            // spot
            dirToLight = lightPositions[i] - worldPos;
            att = computeAttenuation(distance(worldPos, lightPositions[i]), lightFunctions[i]);
            float x = acos(dot(lightDirs[i], -dirToLight) / (length(lightDirs[i]) * length(dirToLight)));
            float outer = lightAngles[i];
            float inner = outer - lightPenumbras[i];
            if (x <= inner) {
                falloff = 1;
            } else if (x >= inner && x <= outer) {
                float y = (x - inner) / (outer - inner);
                float fallFactor = -2*y*y*y + 3*y*y;
                falloff = 1 - fallFactor;
            } else {
                falloff = 0;
            }
        } else {
            continue;
        }

        // diffuse term
        float dotProd = max(0.0, min(dot(normalize(worldNorm), normalize(dirToLight)), 1.0));

        if (toonOn == true){


            if (dotProd > 0){
                dotProd = ceil(dotProd * toonCount) * toonIncrements;
            }

            if (dotProd > 1){
                dotProd = 1;
            }
            if (dotProd < 0){
                dotProd = 0;

            }
            fragColor += att * falloff * lightIntensities[i] * kd * diff * dotProd;

        }
        if (toonOn == false){
            if (dotProd > 0){
                dotProd = ceil(dotProd * toonCount) * toonIncrements;
            }

            if (dotProd > 1){
                dotProd = 1;
            }
            if (dotProd < 0){
                dotProd = 0;

            }
        fragColor += att * falloff * lightIntensities[i] * kd * diff * dotProd;

        // specular term (for things facing camera that are shiny)
        if (dot(worldNorm, dirToCam) >= 0 && shininess > 0) {
            vec3 Ri = computeReflect(worldNorm, dirToLight);
            float specProd = max(0, dot(Ri, normalize(dirToCam)));
            fragColor += floor(lightIntensities[i] * ks * spec * pow(specProd, shininess) * toonCount) * toonIncrements;
        }
        }
    }
}
