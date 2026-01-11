#version 460 core

in vec2 texCoord;
in vec3 currentPos;
in vec3 currentNormal;

out vec4 FragColor;

uniform sampler2D tex0;
uniform vec3 CameraPosition;

void main()
{
    // ambient lighting
    float ambient = 0.8f;
    vec3 lightPos = vec3(0.5f, 512.f, 0.5f);

    // diffuse lighting
    vec3 normal = normalize(currentNormal);
    vec3 lightDirection = normalize(lightPos - currentPos);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    // specular lighting
    float specularLight = 0.50f;
    vec3 viewDirection = normalize(CameraPosition - currentPos);
    vec3 reflectionDirection = reflect(-lightDirection, currentNormal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 8);
    float specular = specAmount * specularLight;

//    FragColor = texture(tex0, texCoord) * vec4(1.0f, 1.0f, 1.0f, 1.0f) * (diffuse + ambient + specular);
    FragColor = texture(tex0, texCoord);
}