#version 330 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out; 

uniform float lineWidth; 
uniform mat4 view;

in vec3 fragColor[]; 
in vec4 clipSpacePos[]; 

out vec3 gsColor;

void main()
{
    vec3 cameraDir = normalize(view[2].xyz);  
    vec2 lineDir = normalize(gl_in[1].gl_Position.xy - gl_in[0].gl_Position.xy); 
    vec2 perp = vec2(-lineDir.y, lineDir.x);
    perp *= lineWidth;

    vec4 offsetLeft = gl_in[0].gl_Position + vec4(perp, 0.0, 0.0);
    vec4 offsetRight = gl_in[1].gl_Position + vec4(perp, 0.0, 0.0);

    gsColor = fragColor[0];

    gl_Position = offsetLeft;
    EmitVertex();

    gl_Position = offsetRight;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position - vec4(perp, 0.0, 0.0);
    EmitVertex();

    gl_Position = gl_in[1].gl_Position - vec4(perp, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}
