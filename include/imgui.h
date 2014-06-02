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

#ifndef IMGUI_H
#define IMGUI_H

#include <stdint.h>
#include <string>
#include <vector>

namespace imgui
{
    enum MouseButton : uint8_t
    {
        MBUT_LEFT  = 0x01,
        MBUT_RIGHT = 0x02
    };

    enum TextAlign : uint8_t
    {
        ALIGN_LEFT,
        ALIGN_CENTER,
        ALIGN_RIGHT
    };

    inline uint32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff)
    {
        return r | (g << 8) | (b << 16) | (a << 24);
    }

    enum gfxCmdType : uint8_t
    {
        GFXCMD_RECT,
        GFXCMD_TRIANGLE,
        GFXCMD_LINE,
        GFXCMD_TEXT,
        GFXCMD_SCISSOR,
        GFXCMD_TEXTURED_RECT
    };

    struct gfxRect
    {
        float x, y, w, h, r;
    };

    struct gfxTexturedRect
    {
        float x, y, w, h;
        unsigned int texture;
        float tx0, ty0, tx1, ty1;
    };

    struct gfxText
    {
        float x, y, pointSize;
        TextAlign align;
        std::string text;
    };

    struct gfxLine
    {
        float x0, y0, x1, y1, r;
    };

    struct gfxCmd
    {
        char type;
        char flags;
        char pad[2];
        unsigned int col;
        gfxLine line;
        gfxRect rect;
        gfxText text;
        gfxTexturedRect texturedRect;
    };

    struct GuiState
    {
        GuiState() {}

        bool left                = false;
        bool leftPressed         = false;
        bool leftReleased        = false;
        int mx                   = -1;
        int my                   = -1;
        int scroll               = 0;
        uint32_t active          = 0;
        uint32_t hot             = 0;
        uint32_t hotToBe         = 0;
        bool isHot               = false;
        bool isActive            = false;
        bool wentActive          = false;
        int dragX                = 0;
        int dragY                = 0;
        float dragOrig           = 0.f;
        float widgetX              = 0;
        float widgetY              = 0;
        float widgetW              = 0;
        bool insideCurrentScroll = false;
        uint32_t areaId          = 0;
        uint32_t widgetId        = 0;
        int scrollTop            = 0;
        int scrollBottom         = 0;
        int scrollRight          = 0;
        int scrollAreaTop        = 0;
        int* scrollVal           = nullptr;
        int focusTop             = 0;
        int focusBottom          = 0;
        uint32_t scrollId        = 0;
        bool insideScrollArea    = false;
        float x0, y0, x1, y1;
    };

    struct Imgui
    {
        void beginFrame(int mouseX, int mouseY, MouseButton mbut, int scroll);
        void endFrame();

        bool beginScrollArea(const std::string& name, int x, int y, int w, int h, int& scroll);
        void endScrollArea();

        void indent(float scale = 1.f);
        void unindent(float scale = 1.f);
        void separator(float scale = 1.f);
        void separatorLine();

        void renderPort(float x, float y, float w);

        bool button  (const std::string& name, bool enabled = true);
        bool item    (const std::string& name, bool enabled = true);
        bool check   (const std::string& name, bool checked, bool enabled = true);
        bool collapse(const std::string& name, const std::string& subText, bool checked, bool enabled = true);
        void label   (const std::string& name, TextAlign align = ALIGN_LEFT, bool dontMove = false, float scale = 1.f);
        void value   (const std::string& name, TextAlign align = ALIGN_RIGHT, float scale = 1.f);
        bool slider  (const std::string& name, float& value, float vmin, float vmax, float vinc, bool enabled = true, float scale = 1.f);

        void labelledValue(const std::string& name, const std::string& value, float scale = 1.f);

        void drawText(int x, int y, TextAlign align, const std::string& text, uint32_t color, float pointSize = 8.f);
        void drawLine(float x0, float y0, float x1, float y1, float r, uint32_t color);
        void drawRoundedRect(float x, float y, float w, float h, float r, uint32_t color);
        void drawRect(float x, float y, float w, float h, uint32_t color);
        void drawTexturedRect(float x, float y, float w, float h, uint32_t color, unsigned int texture, float tx0, float ty0, float tx1, float ty1);

        std::vector<gfxCmd> renderQueue;

        GuiState state;
        bool anyActive();
        bool isActive(uint32_t id);
        bool isHot(uint32_t id);
        bool inRect(int x, int y, int w, int h, bool checkScroll = true);
        void clearInput();
        void clearActive();
        void setActive(uint32_t id);
        void setHot(uint32_t id);
        bool buttonLogic(uint32_t id, bool over);
        void updateInput(int mx, int my, MouseButton mbut, int scroll);

        void resetGfxCmdQueue();
        void addGfxCmdScissor(int x, int y, int w, int h);
        void addGfxCmdRect(float x, float y, float w, float h, uint32_t color);
        void addGfxCmdTexturedRect(float x, float y, float w, float h, uint32_t color, uint32_t texture, float tx0, float ty0, float tx1, float ty1);
        void addGfxCmdLine(float x0, float y0, float x1, float y1, float r, uint32_t color);
        void addGfxCmdRoundedRect(float x, float y, float w, float h, float r, uint32_t color);
        void addGfxCmdTriangle(int x, int y, int w, int h, int flags, uint32_t color);
        void addGfxCmdText(int x, int y, TextAlign align, const std::string& text, uint32_t color, float pointSize = 8.f);
    };
}

#endif
