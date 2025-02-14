#version 100

precision mediump float;

uniform vec4 u_color;
uniform sampler2D u_texture;

varying vec2 v_textureCoord;

void main() {
    gl_FragColor = texture2D(u_texture, v_textureCoord) * u_color;
}