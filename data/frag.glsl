#version 100

precision mediump float;

uniform sampler2D u_textures[8];

varying vec2 v_textureCoord;
varying vec4 v_color;
varying float v_textureID;

void main() {
    vec4 o_color = v_color;

    if (int(v_textureID) == 0) {
        o_color = v_color * texture2D(u_textures[0], v_textureCoord);
    }
    else if (int(v_textureID) == 1) {
        o_color = v_color * texture2D(u_textures[1], v_textureCoord);
    }
    else if (int(v_textureID) == 2) {
        o_color = v_color * texture2D(u_textures[2], v_textureCoord);
    }
    else if (int(v_textureID) == 3) {
        o_color = v_color * texture2D(u_textures[3], v_textureCoord);
    }
    else if (int(v_textureID) == 4) {
        o_color = v_color * texture2D(u_textures[4], v_textureCoord);
    }
    else if (int(v_textureID) == 5) {
        o_color = v_color * texture2D(u_textures[5], v_textureCoord);
    }
    else if (int(v_textureID) == 6) {
        o_color = v_color * texture2D(u_textures[6], v_textureCoord);
    }
    else if (int(v_textureID) == 7) {
        o_color = v_color * texture2D(u_textures[7], v_textureCoord);
    }

    gl_FragColor = o_color;
}