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

#include <cmath>
#include <cstdio>

#include "imguiRenderGL3.h"

#ifndef PI
#define PI 3.14159265f
#endif

void imguifree(void* ptr, void* userptr);
void* imguimalloc(size_t size, void* userptr);

#define STBTT_malloc(x,y)    imguimalloc(x,y)
#define STBTT_free(x,y)      imguifree(x,y)
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

void imguifree(void* ptr, void* /*userptr*/)
{
    free(ptr);
}

void* imguimalloc(size_t size, void* /*userptr*/)
{
    return malloc(size);
}

using namespace imgui;

void ImguiRenderGL3::drawTexturedPolygon(const float* coords, unsigned numCoords, float r, uint32_t col, GLuint tex, float tx0, float ty0, float tx1, float ty1)
{
    if (numCoords > TEMP_COORD_COUNT) numCoords = TEMP_COORD_COUNT;

    for (unsigned i = 0, j = numCoords-1; i < numCoords; j=i++)
    {
        const float* v0 = &coords[j*2];
        const float* v1 = &coords[i*2];
        float dx = v1[0] - v0[0];
        float dy = v1[1] - v0[1];
        float d = sqrtf(dx*dx+dy*dy);
        if (d > 0)
        {
            d = 1.0f/d;
            dx *= d;
            dy *= d;
        }
        state.tempNormals[j*2+0] = dy;
        state.tempNormals[j*2+1] = -dx;
    }

    float colf[4] = { (float) (col&0xff) / 255.f, (float) ((col>>8)&0xff) / 255.f, (float) ((col>>16)&0xff) / 255.f, (float) ((col>>24)&0xff) / 255.f};
    float colTransf[4] = { (float) (col&0xff) / 255.f, (float) ((col>>8)&0xff) / 255.f, (float) ((col>>16)&0xff) / 255.f, 0};

    for (unsigned i = 0, j = numCoords-1; i < numCoords; j=i++)
    {
        float dlx0 = state.tempNormals[j*2+0];
        float dly0 = state.tempNormals[j*2+1];
        float dlx1 = state.tempNormals[i*2+0];
        float dly1 = state.tempNormals[i*2+1];
        float dmx = (dlx0 + dlx1) * 0.5f;
        float dmy = (dly0 + dly1) * 0.5f;
        float   dmr2 = dmx*dmx + dmy*dmy;
        if (dmr2 > 0.000001f)
        {
            float   scale = 1.0f / dmr2;
            if (scale > 10.0f) scale = 10.0f;
            dmx *= scale;
            dmy *= scale;
        }
        state.tempCoords[i*2+0] = coords[i*2+0]+dmx*r;
        state.tempCoords[i*2+1] = coords[i*2+1]+dmy*r;
    }

    int vSize = numCoords * 12 + (numCoords - 2) * 6;
    int uvSize = numCoords * 2 * 6 + (numCoords - 2) * 2 * 3;
    int cSize = numCoords * 4 * 6 + (numCoords - 2) * 4 * 3;
    float * v = state.tempVertices;
    float * uv = state.tempTextureCoords;
    memset(uv, 0, uvSize * sizeof(float));
    float * c = state.tempColors;
    memset(c, 1, cSize * sizeof(float));

    float * ptrV = v;
    float * ptrC = c;
    float * ptrUV = uv;
    for (unsigned i = 0, j = numCoords-1; i < numCoords; j=i++)
    {
        *ptrV = coords[i*2];
        *(ptrV+1) = coords[i*2 + 1];
        ptrV += 2;
        *ptrV = coords[j*2];
        *(ptrV+1) = coords[j*2 + 1];
        ptrV += 2;
        *ptrV = state.tempCoords[j*2];
        *(ptrV+1) = state.tempCoords[j*2 + 1];
        ptrV += 2;
        *ptrV = state.tempCoords[j*2];
        *(ptrV+1) = state.tempCoords[j*2 + 1];
        ptrV += 2;
        *ptrV = state.tempCoords[i*2];
        *(ptrV+1) = state.tempCoords[i*2 + 1];
        ptrV += 2;
        *ptrV = coords[i*2];
        *(ptrV+1) = coords[i*2 + 1];
        ptrV += 2;

        *ptrC = colf[0];
        *(ptrC+1) = colf[1];
        *(ptrC+2) = colf[2];
        *(ptrC+3) = colf[3];
        ptrC += 4;
        *ptrC = colf[0];
        *(ptrC+1) = colf[1];
        *(ptrC+2) = colf[2];
        *(ptrC+3) = colf[3];
        ptrC += 4;
        *ptrC = colTransf[0];
        *(ptrC+1) = colTransf[1];
        *(ptrC+2) = colTransf[2];
        *(ptrC+3) = colTransf[3];
        ptrC += 4;
        *ptrC = colTransf[0];
        *(ptrC+1) = colTransf[1];
        *(ptrC+2) = colTransf[2];
        *(ptrC+3) = colTransf[3];
        ptrC += 4;
        *ptrC = colTransf[0];
        *(ptrC+1) = colTransf[1];
        *(ptrC+2) = colTransf[2];
        *(ptrC+3) = colTransf[3];
        ptrC += 4;
        *ptrC = colf[0];
        *(ptrC+1) = colf[1];
        *(ptrC+2) = colf[2];
        *(ptrC+3) = colf[3];
        ptrC += 4;
    }

    for (unsigned i = 2; i < numCoords; ++i)
    {
        *ptrV = coords[0];
        *(ptrV+1) = coords[1];
        ptrV += 2;
        *ptrV = coords[(i-1)*2];
        *(ptrV+1) = coords[(i-1)*2+1];
        ptrV += 2;
        *ptrV = coords[i*2];
        *(ptrV+1) = coords[i*2 + 1];
        ptrV += 2;

        *ptrC = colf[0];
        *(ptrC+1) = colf[1];
        *(ptrC+2) = colf[2];
        *(ptrC+3) = colf[3];
        ptrC += 4;
        *ptrC = colf[0];
        *(ptrC+1) = colf[1];
        *(ptrC+2) = colf[2];
        *(ptrC+3) = colf[3];
        ptrC += 4;
        *ptrC = colf[0];
        *(ptrC+1) = colf[1];
        *(ptrC+2) = colf[2];
        *(ptrC+3) = colf[3];
        ptrC += 4;
    }

    float minX = 1e10;
    float minY = 1e10;
    float maxX = -1e10;
    float maxY = -1e10;
    for (float * ptrV2 = v; ptrV2 != ptrV; ptrV2 += 2)
    {
        minX = (minX < *ptrV2) ? minX : *ptrV2;
        maxX = (maxX > *ptrV2) ? maxX : *ptrV2;
        minY = (minY < *(ptrV2+1)) ? minY : *(ptrV2+1);
        maxY = (maxY > *(ptrV2+1)) ? maxY : *(ptrV2+1);
    }

    float scaleX = (tx1 - tx0) / (maxX - minX);
    float scaleY = (ty1 - ty0) / (maxY - minY);

    for (float * ptrUV = uv, * ptrV2 = v; ptrV2 != ptrV; ptrV2 += 2, ptrUV += 2)
    {
        *ptrUV       = ((*ptrV2) - minX) * scaleX + tx0;
        *(ptrUV + 1) = ((*(ptrV2 + 1)) - minY) * scaleY + ty0;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    if (GLEW_ARB_vertex_array_object)
    {
        glBindVertexArray(state.vao);
    }
    else
    {
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
    }
    glBindBuffer(GL_ARRAY_BUFFER, state.vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, vSize*sizeof(float), v, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, state.vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, uvSize*sizeof(float), uv, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, state.vbos[2]);
    glBufferData(GL_ARRAY_BUFFER, cSize*sizeof(float), c, GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, (numCoords * 2 + numCoords - 2)*3);
}

void ImguiRenderGL3::drawPolygon(const float* coords, unsigned numCoords, float r, uint32_t col)
{
    drawTexturedPolygon(coords, numCoords, r, col, state.whitetex, 0, 0, 1, 1);
}

void ImguiRenderGL3:: drawRect(float x, float y, float w, float h, float fth, uint32_t col)
{
    float verts[4*2] =
    {
        x+0.5f, y+0.5f,
        x+w-0.5f, y+0.5f,
        x+w-0.5f, y+h-0.5f,
        x+0.5f, y+h-0.5f,
    };
    drawPolygon(verts, 4, fth, col);
}

void ImguiRenderGL3:: drawTexturedRect(float x, float y, float w, float h, uint32_t texture, uint32_t col, float tx0, float ty0, float tx1, float ty1)
{
    float verts[4*2] =
    {
        x+0.5f, y+0.5f,
        x+w-0.5f, y+0.5f,
        x+w-0.5f, y+h-0.5f,
        x+0.5f, y+h-0.5f,
    };
    drawTexturedPolygon(verts, 4, 1.0, col, texture, tx0, ty0, tx1, ty1);
}

void ImguiRenderGL3:: drawRoundedRect(float x, float y, float w, float h, float r, float fth, uint32_t col)
{
    const unsigned n = CIRCLE_VERTS/4;
    float verts[(n+1)*4*2];
    const float* cverts = state.circleVerts;
    float* v = verts;

    for (unsigned i = 0; i <= n; ++i)
    {
        *v++ = x+w-r + cverts[i*2]*r;
        *v++ = y+h-r + cverts[i*2+1]*r;
    }

    for (unsigned i = n; i <= n*2; ++i)
    {
        *v++ = x+r + cverts[i*2]*r;
        *v++ = y+h-r + cverts[i*2+1]*r;
    }

    for (unsigned i = n*2; i <= n*3; ++i)
    {
        *v++ = x+r + cverts[i*2]*r;
        *v++ = y+r + cverts[i*2+1]*r;
    }

    for (unsigned i = n*3; i < n*4; ++i)
    {
        *v++ = x+w-r + cverts[i*2]*r;
        *v++ = y+r + cverts[i*2+1]*r;
    }
    *v++ = x+w-r + cverts[0]*r;
    *v++ = y+r + cverts[1]*r;

    drawPolygon(verts, (n+1)*4, fth, col);
}

void ImguiRenderGL3:: drawLine(float x0, float y0, float x1, float y1, float r, float fth, uint32_t col)
{
    float dx = x1-x0;
    float dy = y1-y0;
    float d = sqrtf(dx*dx+dy*dy);
    if (d > 0.0001f)
    {
        d = 1.0f/d;
        dx *= d;
        dy *= d;
    }
    float nx = dy;
    float ny = -dx;
    float verts[4*2];
    r -= fth;
    r *= 0.5f;
    if (r < 0.01f) r = 0.01f;
    dx *= r;
    dy *= r;
    nx *= r;
    ny *= r;

    verts[0] = x0-dx-nx;
    verts[1] = y0-dy-ny;

    verts[2] = x0-dx+nx;
    verts[3] = y0-dy+ny;

    verts[4] = x1+dx+nx;
    verts[5] = y1+dy+ny;

    verts[6] = x1+dx-nx;
    verts[7] = y1+dy-ny;

    drawPolygon(verts, 4, fth, col);
}

bool ImguiRenderGL3::init(const std::string& fontpath)
{
    initialized = true;

    for (int i = 0; i < CIRCLE_VERTS; ++i)
    {
        float a = (float)i/(float)CIRCLE_VERTS * PI*2;
        state.circleVerts[i*2+0] = cosf(a);
        state.circleVerts[i*2+1] = sinf(a);
    }

    // Load font.
    FILE* fp = fopen(fontpath.c_str(), "rb");
    if (!fp) return false;
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char* ttfBuffer = (unsigned char*)malloc(size);
    if (!ttfBuffer)
    {
        fclose(fp);
        return false;
    }

    fread(ttfBuffer, 1, size, fp);
    fclose(fp);
    fp = 0;

    unsigned char* bmap = (unsigned char*)malloc(512*512);
    if (!bmap)
    {
        free(ttfBuffer);
        return false;
    }

    stbtt_BakeFontBitmap(ttfBuffer,0, 15.0f, bmap,512,512, 32,96, (stbtt_bakedchar*)state.cdata);

    // can free ttf_buffer at this point
    glGenTextures(1, &state.ftex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, state.ftex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY, 512,512, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, bmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // can free ttf_buffer at this point
    unsigned char white_alpha = 255;
    glGenTextures(1, &state.whitetex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, state.whitetex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY, 1, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &white_alpha);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // needed imgui to work with GL 2.1... no VAO :'(
    if (GLEW_ARB_vertex_array_object)
    {
        glGenVertexArrays(1, &state.vao);
    }
    glGenBuffers(3, state.vbos);

    if (GLEW_ARB_vertex_array_object)
    {
        glBindVertexArray(state.vao);
    }
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, state.vbos[0]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, state.vbos[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, state.vbos[2]);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*4, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STREAM_DRAW);
    state.program = glCreateProgram();
    state.font_program = glCreateProgram();

    const char * vs =
    "#version 120\n"
    "uniform vec2 Viewport;\n"
    "in vec2 VertexPosition;\n"
    "in vec2 VertexTexCoord;\n"
    "in vec4 VertexColor;\n"
    "out vec2 texCoord;\n"
    "out vec4 vertexColor;\n"
    "void main(void)\n"
    "{\n"
    "    vertexColor = VertexColor;\n"
    "    texCoord = VertexTexCoord;\n"
    "    gl_Position = vec4(VertexPosition * 2.0 / Viewport - 1.0, 0.f, 1.0);\n"
    "}\n";
    GLuint vso = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vso, 1, (const char **)  &vs, NULL);
    glCompileShader(vso);

    GLint isCompiled = 0;
    glGetShaderiv(vso, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(vso, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        char* errorLog = new char[maxLength];
        glGetShaderInfoLog(vso, maxLength, &maxLength, errorLog);
        printf("%s\n", errorLog);

        //Provide the infolog in whatever manor you deem best.
        //Exit with failure.
        glDeleteShader(vso); //Don't leak the vso.
    }

    glAttachShader(state.program, vso);
    glAttachShader(state.font_program, vso);

    const char * fs2 =
    "#version 120\n"
    "in vec2 texCoord;\n"
    "in vec4 vertexColor;\n"
    "uniform sampler2D Texture;\n"
    "out vec4  Color;\n"
    "void main(void)\n"
    "{\n"
    "    Color = vertexColor * texture(Texture, texCoord).bgra;\n"
    "}\n";
    GLuint fso2 = glCreateShader(GL_FRAGMENT_SHADER);

    const char * fs =
    "#version 120\n"
    "in vec2 texCoord;\n"
    "in vec4 vertexColor;\n"
    "uniform sampler2D Texture;\n"
    "out vec4  Color;\n"
    "void main(void)\n"
    "{\n"
    "    Color = vertexColor * vec4(1, 1, 1, texture(Texture, texCoord).a);\n"
    "}\n";
    GLuint fso = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fso, 1, (const char **) &fs, NULL);
    glCompileShader(fso);
    glAttachShader(state.program, fso2);

    glGetShaderiv(fso, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(fso, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        char* errorLog = new char[maxLength];
        glGetShaderInfoLog(fso, maxLength, &maxLength, errorLog);
        printf("%s\n", errorLog);

        //Provide the infolog in whatever manor you deem best.
        //Exit with failure.
        glDeleteShader(fso); //Don't leak the vso.
    }

    glShaderSource(fso2, 1, (const char **) &fs2, NULL);
    glCompileShader(fso2);
    glAttachShader(state.font_program, fso);

    glGetShaderiv(fso2, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(fso2, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        char* errorLog = new char[maxLength];
        glGetShaderInfoLog(fso2, maxLength, &maxLength, errorLog);
        printf("%s\n", errorLog);

        //Provide the infolog in whatever manor you deem best.
        //Exit with failure.
        glDeleteShader(fso2); //Don't leak the vso.
    }

    glBindAttribLocation(state.program,  0,  "VertexPosition");
    glBindAttribLocation(state.program,  1,  "VertexTexCoord");
    glBindAttribLocation(state.program,  2,  "VertexColor");
    glBindAttribLocation(state.font_program,  0,  "VertexPosition");
    glBindAttribLocation(state.font_program,  1,  "VertexTexCoord");
    glBindAttribLocation(state.font_program,  2,  "VertexColor");

    glLinkProgram(state.program);
    GLint isLinked = 0;
    glGetProgramiv(state.program, GL_LINK_STATUS, (int *)&isLinked);
    if(isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(state.program, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        char* infoLog = new char[maxLength];
        glGetProgramInfoLog(state.program, maxLength, &maxLength, infoLog);
        printf("%s\n", infoLog);
    }

    glLinkProgram(state.font_program);
    glGetProgramiv(state.font_program, GL_LINK_STATUS, (int *)&isLinked);
    if(isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(state.font_program, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        char* infoLog = new char[maxLength];
        glGetProgramInfoLog(state.font_program, maxLength, &maxLength, infoLog);
        printf("%s\n", infoLog);
    }

    glDeleteShader(vso);
    glDeleteShader(fso);
    glDeleteShader(fso2);

    state.programViewportLocation = glGetUniformLocation(state.program, "Viewport");
    state.programTextureLocation = glGetUniformLocation(state.program, "Texture");

    state.font_programViewportLocation = glGetUniformLocation(state.font_program, "Viewport");
    state.font_programTextureLocation = glGetUniformLocation(state.font_program, "Texture");

    free(ttfBuffer);
    free(bmap);

    return true;
}

ImguiRenderGL3::ImguiRenderGL3(ImguiRenderGL3&& in) noexcept
{
    state = std::move(in.state);
    initialized = in.initialized;
    in.state = RenderState();
    in.initialized = false;
}
ImguiRenderGL3& ImguiRenderGL3::operator=(ImguiRenderGL3&& in) noexcept
{
    state = std::move(in.state);
    initialized = in.initialized;
    in.state = RenderState();
    in.initialized = false;
    return *this;
}

void ImguiRenderGL3::destroy()
{
    if (!initialized)
    {
        return;
    }
    initialized = false;

    if (state.ftex)
    {
        glDeleteTextures(1, &state.ftex);
        state.ftex = 0;
    }

    if (state.vao)
    {
        glDeleteVertexArrays(1, &state.vao);
        state.vao = 0;
    }
    if (state.vbos[0])
    {
        glDeleteBuffers(3, state.vbos);
    }

    if (state.program)
    {
        glDeleteProgram(state.program);
        state.program = 0;
    }

    if (state.font_program)
    {
        glDeleteProgram(state.font_program);
        state.font_program = 0;
    }
}

void ImguiRenderGL3:: getBakedQuad(stbtt_bakedchar *chardata, int pw, int ph, int char_index, float *xpos, float *ypos, stbtt_aligned_quad *q)
{
    stbtt_bakedchar *b = chardata + char_index;
    int round_x = STBTT_ifloor(*xpos + b->xoff);
    int round_y = STBTT_ifloor(*ypos - b->yoff);

    q->x0 = (float)round_x;
    q->y0 = (float)round_y;
    q->x1 = (float)round_x + b->x1 - b->x0;
    q->y1 = (float)round_y - b->y1 + b->y0;

    q->s0 = b->x0 / (float)pw;
    q->t0 = b->y0 / (float)pw;
    q->s1 = b->x1 / (float)ph;
    q->t1 = b->y1 / (float)ph;

    *xpos += b->xadvance;
}

static const float tabStops[4] = {150, 210, 270, 330};
static float getTextLength(stbtt_bakedchar *chardata, const char* text)
{
    float xpos = 0;
    float len = 0;
    while (*text)
    {
        int c = (unsigned char)*text;
        if (c == '\t')
        {
            for (int i = 0; i < 4; ++i)
            {
                if (xpos < tabStops[i])
                {
                    xpos = tabStops[i];
                    break;
                }
            }
        }
        else if (c >= 32 && c < 128)
        {
            stbtt_bakedchar *b = chardata + c-32;
            int round_x = STBTT_ifloor((xpos + b->xoff) + 0.5);
            len = round_x + b->x1 - b->x0 + 0.5f;
            xpos += b->xadvance;
        }
        ++text;
    }
    return len;
}

void ImguiRenderGL3:: drawText(float x, float y, const std::string& textin, int align, uint32_t col)
{
    if (!state.ftex) return;
    if (textin.length() == 0) return;

    const char* text = textin.c_str();

    glUseProgram(state.font_program);
    glActiveTexture(GL_TEXTURE0);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    if (align == ALIGN_CENTER)
        x -= getTextLength(state.cdata, text)/2;
    else if (align == ALIGN_RIGHT)
        x -= getTextLength(state.cdata, text);

    float r = (float) (col&0xff) / 255.f;
    float g = (float) ((col>>8)&0xff) / 255.f;
    float b = (float) ((col>>16)&0xff) / 255.f;
    float a = (float) ((col>>24)&0xff) / 255.f;

    // assume orthographic projection with units = screen pixels, origin at top left
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, state.ftex);

    const float ox = x;

    while (*text)
    {
        int c = (unsigned char)*text;
        if (c == '\t')
        {
            for (int i = 0; i < 4; ++i)
            {
                if (x < tabStops[i]+ox)
                {
                    x = tabStops[i]+ox;
                    break;
                }
            }
        }
        else if (c >= 32 && c < 128)
        {
            stbtt_aligned_quad q;
            getBakedQuad(state.cdata, 512,512, c-32, &x,&y,&q);

            float v[12] = {
                    q.x0, q.y0,
                    q.x1, q.y1,
                    q.x1, q.y0,
                    q.x0, q.y0,
                    q.x0, q.y1,
                    q.x1, q.y1,
                      };
            float uv[12] = {
                    q.s0, q.t0,
                    q.s1, q.t1,
                    q.s1, q.t0,
                    q.s0, q.t0,
                    q.s0, q.t1,
                    q.s1, q.t1,
                      };
            float c[24] = {
                    r, g, b, a,
                    r, g, b, a,
                    r, g, b, a,
                    r, g, b, a,
                    r, g, b, a,
                    r, g, b, a,
                      };


            if (GLEW_ARB_vertex_array_object)
            {
                glBindVertexArray(state.vao);
            }
            else
            {
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glEnableVertexAttribArray(2);
            }

            glBindBuffer(GL_ARRAY_BUFFER, state.vbos[0]);
            glBufferData(GL_ARRAY_BUFFER, 12*sizeof(float), v, GL_STREAM_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, state.vbos[1]);
            glBufferData(GL_ARRAY_BUFFER, 12*sizeof(float), uv, GL_STREAM_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, state.vbos[2]);
            glBufferData(GL_ARRAY_BUFFER, 24*sizeof(float), c, GL_STREAM_DRAW);
            glDrawArrays(GL_TRIANGLES, 0, 6);

        }
        ++text;
    }

    //glEnd();
    //glDisable(GL_TEXTURE_2D);

    glUseProgram(state.program);
    glActiveTexture(GL_TEXTURE0);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}


void ImguiRenderGL3::draw(Imgui& imgui, int width, int height)
{
    auto q = imgui.renderQueue;
    int nq = q.size();

    const float s = 1.0f/8.0f;

    glViewport(0, 0, width, height);
    glUseProgram(state.program);
    glActiveTexture(GL_TEXTURE0);
    glUniform2f(state.programViewportLocation, (float) width, (float) height);
    glUniform1i(state.programTextureLocation, 0);
    glUseProgram(state.font_program);
    glUniform2f(state.font_programViewportLocation, (float) width, (float) height);
    glUniform1i(state.font_programTextureLocation, 0);
    glUseProgram(state.program);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, state.vbos[0]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, state.vbos[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, state.vbos[2]);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*4, (void*)0);

    glDisable(GL_SCISSOR_TEST);
    for (int i = 0; i < nq; ++i)
    {
        const gfxCmd& cmd = q[i];
        if (cmd.type == GFXCMD_RECT)
        {
            if (cmd.rect.r == 0)
            {
                drawRect((float)cmd.rect.x*s+0.5f, (float)cmd.rect.y*s+0.5f,
                         (float)cmd.rect.w*s-1, (float)cmd.rect.h*s-1,
                         1.0f, cmd.col);
            }
            else
            {
                drawRoundedRect((float)cmd.rect.x*s+0.5f, (float)cmd.rect.y*s+0.5f,
                                (float)cmd.rect.w*s-1, (float)cmd.rect.h*s-1,
                                (float)cmd.rect.r*s, 1.0f, cmd.col);
            }
        }
        else if (cmd.type == GFXCMD_TEXTURED_RECT)
        {
            drawTexturedRect((float)cmd.rect.x*s+0.5f, (float)cmd.rect.y*s+0.5f,
                     (float)cmd.rect.w*s-1, (float)cmd.rect.h*s-1,
                     cmd.texturedRect.texture, cmd.col,
                     cmd.texturedRect.tx0, cmd.texturedRect.ty0,
                     cmd.texturedRect.tx1, cmd.texturedRect.ty1);
        }
        else if (cmd.type == GFXCMD_LINE)
        {
            drawLine(cmd.line.x0*s, cmd.line.y0*s, cmd.line.x1*s, cmd.line.y1*s, cmd.line.r*s, 1.0f, cmd.col);
        }
        else if (cmd.type == GFXCMD_TRIANGLE)
        {
            if (cmd.flags == 1)
            {
                const float verts[3*2] =
                {
                    (float)cmd.rect.x*s+0.5f, (float)cmd.rect.y*s+0.5f,
                    (float)cmd.rect.x*s+0.5f+(float)cmd.rect.w*s-1, (float)cmd.rect.y*s+0.5f+(float)cmd.rect.h*s/2-0.5f,
                    (float)cmd.rect.x*s+0.5f, (float)cmd.rect.y*s+0.5f+(float)cmd.rect.h*s-1,
                };
                drawPolygon(verts, 3, 1.0f, cmd.col);
            }
            if (cmd.flags == 2)
            {
                const float verts[3*2] =
                {
                    (float)cmd.rect.x*s+0.5f, (float)cmd.rect.y*s+0.5f+(float)cmd.rect.h*s-1,
                    (float)cmd.rect.x*s+0.5f+(float)cmd.rect.w*s/2-0.5f, (float)cmd.rect.y*s+0.5f,
                    (float)cmd.rect.x*s+0.5f+(float)cmd.rect.w*s-1, (float)cmd.rect.y*s+0.5f+(float)cmd.rect.h*s-1,
                };
                drawPolygon(verts, 3, 1.0f, cmd.col);
            }
        }
        else if (cmd.type == GFXCMD_TEXT)
        {
            drawText(cmd.text.x, cmd.text.y, cmd.text.text, cmd.text.align, cmd.col);
        }
        else if (cmd.type == GFXCMD_SCISSOR)
        {
            if (cmd.flags)
            {
                glEnable(GL_SCISSOR_TEST);
                glScissor(cmd.rect.x, cmd.rect.y, cmd.rect.w, cmd.rect.h);
            }
            else
            {
                glDisable(GL_SCISSOR_TEST);
            }
        }
    }
    glDisable(GL_SCISSOR_TEST);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glUseProgram(0);
}
