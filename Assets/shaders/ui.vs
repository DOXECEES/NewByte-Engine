#version 330 core
layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec2 aTexCoord;

//out vec2 TexCoord;

uniform mat4 projection;
uniform vec2 position;

void main() {
    gl_Position = projection * vec4(aPos.xy + position, 0.0, 1.0);
    //TexCoord = aTexCoord;
}
