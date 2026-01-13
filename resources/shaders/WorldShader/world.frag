#version 460 core

in vec3 currentPos;
in vec3 currentNormal;
in vec2 currentUvs;
flat in uint currentTexIndex;

out vec4 FragColor;

uniform sampler2DArray Textures;
//uniform vec3 CameraPosition;

void main()
{
    // ambient lighting
    float ambient = 0.4f;
    vec3 lightPos = vec3(64.f, 256.f, 64.f);

    // diffuse lighting
    vec3 normal = normalize(currentNormal);
    vec3 lightDirection = normalize(lightPos - currentPos);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    FragColor = texture(Textures, vec3(currentUvs, currentTexIndex)) * vec4(1.f, 1.f, 1.f, 1.f) * (diffuse + ambient);
}