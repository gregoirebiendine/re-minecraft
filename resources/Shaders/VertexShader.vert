#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 faces;
//layout (location = 1) in vec2 tex;

out vec2 texCoord;

uniform mat4 ViewMatrix;

void main()
{
    int index = 0;

    if (gl_VertexID < 16) {
        index = int(faces.x);
    } else if (gl_VertexID >= 16 && gl_VertexID < 20) {
        index = int(faces.y);
    } else if (gl_VertexID >= 20) {
        index = int(faces.z);
    }

    int tiles = 4;
    float tileSize = 1.0 / float(tiles);

    vec2 uv = vec2(
        float(index % tiles) * tileSize,
        1.0 - (float(index / tiles + 1) * tileSize)
    );

    vec2 test[4] = vec2[4](
            vec2(uv.x, uv.y),
            vec2(uv.x + (1.0f / tiles), uv.y),
            vec2(uv.x + (1.0f / tiles), uv.y + (1.0f / tiles)),
            vec2(uv.x, uv.y + (1.0f / tiles))
    );

    vec3 newPos = vec3(pos.x + (gl_InstanceID*2), pos.y, pos.z);
    gl_Position = ViewMatrix * vec4(newPos, 1.0);
    texCoord = test[gl_VertexID % 4];
}