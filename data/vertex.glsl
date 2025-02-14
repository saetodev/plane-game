#version 100

attribute vec2 a_position;

uniform mat4 u_transform;
uniform mat4 u_projection;

void main() {
    gl_Position = u_projection * u_transform * vec4(a_position.xy, 0.0f, 1.0f);
}