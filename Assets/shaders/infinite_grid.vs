/*
    Copyright 2024 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#version 330

out vec3 worldPos;

uniform mat4 uViewProjection = mat4(1.0);
uniform float uGridSize = 100.0;
uniform vec3 uCameraWorldPosition;

const vec3 position[4] = vec3[4](
    vec3(-1.0, 0.0, -1.0),      // bottom left
    vec3( 1.0, 0.0, -1.0),      // bottom right
    vec3( 1.0, 0.0,  1.0),      // top right
    vec3(-1.0, 0.0,  1.0)       // top left
);

const int indices[6] = int[6](0, 2, 1, 2, 0, 3);


void main()
{
    int index = indices[gl_VertexID];
    vec3 vPos3 = position[index] * uGridSize;

    vPos3.x += uCameraWorldPosition.x;
    vPos3.z += uCameraWorldPosition.z;

    vec4 vPos4 = vec4(vPos3, 1.0);

    gl_Position = uViewProjection * vPos4;

    worldPos = vPos3;
}