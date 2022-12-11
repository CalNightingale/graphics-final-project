#version 330 core
out vec4 FragColor;

in vec3 texCoords;

uniform samplerCube skybox;
uniform samplerCube starTexture;
uniform vec4 skyColor;
uniform vec3 lightPos;
uniform float time;


uniform float sunPhi;
uniform float sunTheta;

void main()
{

    bool xAgnostic = texCoords.x == -1 || texCoords.x == 1;
    bool yAgnostic = texCoords.y == -1 || texCoords.y == 1;
    bool zAgnostic = texCoords.z == -1 || texCoords.z == 1;

    // this is legacy doesnt do anything, just here in case go back to it or something
    int cloudSize = 25;

    float u,v;

    if(xAgnostic){
        u = texCoords.y;
        v = texCoords.z;
    }
    if(yAgnostic){
        u = texCoords.x;
        v = texCoords.z;
    }
    if(zAgnostic){
        u = texCoords.x;
        v = texCoords.y;
    }


    int uPix = int(((u + 1) / 2.f) * cloudSize);
    int vPix = int(((v + 1) / 2.f) * cloudSize);



    float sunProx = dot(normalize(texCoords), normalize(lightPos));






//    if(texCoords.y == -1 ){
//        FragColor = vec4(1,0,0,1);

//    }

//    if(texCoords.y == 1 ){
//        FragColor = vec4(1,0,0,1);
//    }

//    if(texCoords.x == 1 ){
//        FragColor = vec4(0,1,0,1);
//    }


//    if(texCoords.x == -1 ){
//        FragColor = vec4(0,1,0,1);
//    }


//    if(texCoords.z == 1 ){
//        FragColor = vec4(0,0,1,1);
//    }


//    if(texCoords.z == -1 ){
//        FragColor = vec4(0,0,1,1);
//    }


//    if((uPix %2 == 1  && vPix %2 == 1) ||(uPix %2 == 0  && vPix %2 == 0) ){
//        FragColor = vec4(1,1,1,1);
//    } else {

//        FragColor = skyColor;
//    }

    if(sunProx > .9999 && texture(starTexture, texCoords) != vec4(0,0,0,0)){
        FragColor = vec4(1,.5,0,1);
    } else {
        FragColor = skyColor;
    }


//    FragColor = skyColor;
    if(time < 1000){
        FragColor += (1 - smoothstep(0, 1000, time)) * texture(starTexture, texCoords);
          FragColor += (( smoothstep(0, 1000, time) + 0.5) / 1.5f) * texture(skybox, texCoords);
    }
    else if(time > 4000){
        FragColor += smoothstep(4000, 5000, time) * texture(starTexture, texCoords);
          FragColor += ((1 -  ( smoothstep(4000, 5000, time)) + 0.5) / 1.5f) *texture(skybox, texCoords);
    }  else {
        FragColor += texture(skybox, texCoords);

    }



}
