#version 100

attribute vec2 a_position;
attribute vec2 a_textureCoord;
attribute vec4 a_color;
attribute float a_textureID;

uniform mat4 u_projection;

varying vec2 v_textureCoord;
varying vec4 v_color;
varying float v_textureID;

void main() {
    v_textureCoord = a_textureCoord;
    v_color        = a_color;
    v_textureID    = a_textureID;

    gl_Position = u_projection * vec4(a_position, 0.0, 1.0);
}