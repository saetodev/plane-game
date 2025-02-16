#ifndef CORE_RENDERER_SHADER_H
#define CORE_RENDERER_SHADER_H

#include "Buffer.h"
#include "LinearMath.h"

namespace Renderer2D {

u32 CreateShader(const std::string& vertexSource, const std::string& fragmentSource, const List<Attribute, MAX_ATTRIBUTE_COUNT>& attributes);
void DestroyShader(u32 shader);

void UseShader(u32 shader);

void SetUniform(u32 shader, int value, const char* name);
void SetUniform(u32 shader, const Mat4& mat, const char* name);

void SetUniformIndex(u32 shader, int index, int value, const char* name);

}

#endif