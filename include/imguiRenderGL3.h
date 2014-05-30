//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

// Source altered and distributed from https://github.com/AdrienHerubel/imgui

// Heavily modified Luca Deltodesco 2014 https://github.com/deltaluca/imgui

#ifndef IMGUI_RENDER_GL_H
#define IMGUI_RENDER_GL_H

#include <GL/glew.h>
#include <GL/gl.h>

#include "imgui.h"
#include "stb_truetype.h"

namespace imgui
{
    const unsigned TEMP_COORD_COUNT = 100;
    const int CIRCLE_VERTS = 8*4;
    struct RenderState
    {
        stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
        GLuint ftex = 0;
        GLuint whitetex = 0;
        GLuint vao = 0;
        GLuint vbos[3] = {0, 0, 0};
        GLuint program = 0;
        GLuint font_program = 0;
        GLuint programViewportLocation = 0;
        GLuint programTextureLocation = 0;
        GLuint font_programViewportLocation = 0;
        GLuint font_programTextureLocation = 0;

        float tempCoords[TEMP_COORD_COUNT*2];
        float tempNormals[TEMP_COORD_COUNT*2];
        float tempVertices[TEMP_COORD_COUNT * 12 + (TEMP_COORD_COUNT - 2) * 6];
        float tempTextureCoords[TEMP_COORD_COUNT * 12 + (TEMP_COORD_COUNT - 2) * 6];
        float tempColors[TEMP_COORD_COUNT * 24 + (TEMP_COORD_COUNT - 2) * 12];
        float circleVerts[CIRCLE_VERTS*2];
    };

    struct ImguiRenderGL3
    {
        bool init(const std::string& fontpath);
        void destroy();
        void draw(Imgui& imgui, int width, int height);

        ~ImguiRenderGL3()
        {
            destroy();
        }
        ImguiRenderGL3() {}
        ImguiRenderGL3(const ImguiRenderGL3&) = delete;
        ImguiRenderGL3(ImguiRenderGL3&&) noexcept;
        ImguiRenderGL3& operator=(ImguiRenderGL3&&) noexcept;

    private:
        bool initialized = false;
        RenderState state;

        void drawTexturedPolygon(const float* coords, unsigned numCoords, float r, unsigned int col, GLuint tex, float tx0, float ty0, float tx1, float ty1);
        void drawPolygon(const float* coords, unsigned numCoords, float r, uint32_t col);
        void drawRect(float x, float y, float w, float h, float fth, uint32_t col);
        void drawTexturedRect(float x, float y, float w, float h, uint32_t texture, uint32_t col, float tx0, float ty0, float tx1, float ty1);
        void drawRoundedRect(float x, float y, float w, float h, float r, float fth, uint32_t col);
        void drawLine(float x0, float y0, float x1, float y1, float r, float fth, uint32_t col);
        void getBakedQuad(stbtt_bakedchar *chardata, int pw, int ph, int char_index, float *xpos, float *ypos, stbtt_aligned_quad *q, float scale);
        void drawText(float x, float y, const std::string& text, int align, uint32_t col, float pointSize);
    };
}

#endif
