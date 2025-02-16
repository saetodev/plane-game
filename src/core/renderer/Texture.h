#ifndef CORE_RENDERER_TEXTURE_H
#define CORE_RENDERER_TEXTURE_H

#include "Basic.h"

struct Texture2D {
    u32 renderID;
    u32 width;
    u32 height;
};

namespace Renderer2D {

Texture2D CreateTexture(u32 width, u32 height, void* pixels);
Texture2D LoadTexture(const char* filename);

}

#endif