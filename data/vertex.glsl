#version 100

attribute vec2 a_position;
attribute vec2 a_textureCoord;

uniform mat4 u_transform;
uniform mat4 u_projection;

varying vec2 v_textureCoord;

void main() {
    v_textureCoord = a_textureCoord;
    gl_Position = u_projection * u_transform * vec4(a_position, 0.0, 1.0);
}