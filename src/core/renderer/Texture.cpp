#include "Texture.h"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture2D Renderer2D::CreateTexture(u32 width, u32 height, void* pixels) {
    u32 id = 0;
    
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    return {
        .renderID = id,
        .width    = width,
        .height   = height,
    };
}

Texture2D Renderer2D::LoadTexture(const char* filename) {
    //stbi_set_flip_vertically_on_load(true);

    int w, h, comp;
    u8* data = stbi_load(filename, &w, &h, &comp, 4);

    Texture2D texture = CreateTexture(w, h, data);
    stbi_image_free(data);

    return texture;
}