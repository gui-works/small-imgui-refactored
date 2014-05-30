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

#include <cstdio>
#include <cstring>
#include <string>
#include <cmath>

#include "imgui.h"

using namespace imgui;

void Imgui::resetGfxCmdQueue()
{
    renderQueue.clear();
}
bool Imgui::anyActive()
{
    return state.active != 0;
}
bool Imgui::isActive(uint32_t id)
{
    return state.active == id;
}
bool Imgui::isHot(uint32_t id)
{
    return state.hot == id;
}
bool Imgui::inRect(int x, int y, int w, int h, bool checkScroll)
{
    return (!checkScroll || state.insideCurrentScroll) &&
           state.mx >= x && state.mx <= x + w &&
           state.my >= y && state.my <= y + h;
}
void Imgui::clearInput()
{
    state.leftPressed = false;
    state.leftReleased = false;
    state.scroll = 0;
}
void Imgui::clearActive()
{
    state.active = 0;
    clearInput();
}
void Imgui::setActive(uint32_t id)
{
    state.active = id;
    state.wentActive = true;
}
void Imgui::setHot(uint32_t id)
{
    state.hotToBe = id;
}
bool Imgui::buttonLogic(uint32_t id, bool over)
{
    bool res = false;
    // process down
    if (!anyActive())
    {
        if (over)
        {
            setHot(id);
        }
        if (isHot(id) && state.leftPressed)
        {
            setActive(id);
        }
    }

    // if button is active, then react on left up
    if (isActive(id))
    {
        state.isActive = true;
        if (over)
        {
            setHot(id);
        }
        if (state.leftReleased)
        {
            if (isHot(id))
            {
                res = true;
            }
            clearActive();
        }
    }

    if (isHot(id))
    {
        state.isHot = true;
    }

    return res;
}
void Imgui::updateInput(int mx, int my, MouseButton mbut, int scroll)
{
    bool left = (mbut & MBUT_LEFT) != 0;

    state.mx = mx;
    state.my = my;
    state.leftPressed = !state.left && left;
    state.leftReleased = state.left && !left;
    state.left = left;

    state.scroll = scroll;
}

void Imgui::beginFrame(int mx, int my, MouseButton mbut, int scroll)
{
    updateInput(mx, my, mbut, scroll);

    state.hot = state.hotToBe;
    state.hotToBe = 0;

    state.wentActive = false;
    state.isActive = false;
    state.isHot = false;

    state.widgetX = 0;
    state.widgetY = 0;
    state.widgetW = 0;

    state.areaId = 1;
    state.widgetId = 1;

    resetGfxCmdQueue();
}

void Imgui::endFrame()
{
    clearInput();
}

static const int BUTTON_HEIGHT       = 16;
static const int SLIDER_HEIGHT       = 16;
static const int SLIDER_MARKER_WIDTH = 8;
static const int CHECK_SIZE          = 8;
static const int DEFAULT_SPACING     = 2;
static const int TEXT_HEIGHT         = 8;
static const int SCROLL_AREA_PADDING = 3;
static const int INDENT_SIZE         = 16;
static const int AREA_HEADER         = 20;

bool Imgui::beginScrollArea(const std::string& name, int x, int y, int w, int h, int& scroll)
{
    int header = name.length() != 0 ? AREA_HEADER: SCROLL_AREA_PADDING + 2;

    state.areaId++;
    state.widgetId = 0;
    state.scrollId = (state.areaId << 16) | state.widgetId;

    state.widgetX      = x + SCROLL_AREA_PADDING;
    state.widgetY      = y + h - header + scroll;
    state.widgetW      = w - SCROLL_AREA_PADDING * 2;
    state.scrollTop    = y - header + h;
    state.scrollBottom = y + SCROLL_AREA_PADDING;
    state.scrollRight  = x + w - SCROLL_AREA_PADDING * 3;
    state.scrollVal    = &scroll;

    state.scrollAreaTop = state.widgetY;

    state.focusTop    = y - header;
    state.focusBottom = y - header + h;

    state.insideScrollArea = inRect(x, y, w, h, false);
    state.insideCurrentScroll = state.insideScrollArea;

    addGfxCmdRoundedRect((float)x, (float)y, (float)w, (float)h, 6, RGBA(50,50,50,192));

    if (name.length() > 0)
    {
        addGfxCmdText(x + header / 2,
                      y + h - header / 2 - TEXT_HEIGHT / 2,
                      ALIGN_LEFT,
                      name,
                      RGBA(255,255,255,128));
    }
    addGfxCmdScissor(x + SCROLL_AREA_PADDING,
                     y + SCROLL_AREA_PADDING,
                     w - SCROLL_AREA_PADDING * 2,
                     h - header - SCROLL_AREA_PADDING);

    return state.insideScrollArea;
}
void Imgui::endScrollArea()
{
    // Disable scissoring.
    addGfxCmdScissor(-1,-1,-1,-1);

    // Draw scroll bar
    int x = state.scrollRight+SCROLL_AREA_PADDING;
    int y = state.scrollBottom;
    int w = SCROLL_AREA_PADDING*2;
    int h = state.scrollTop - state.scrollBottom;

    int stop = state.scrollAreaTop;
    int sbot = state.widgetY;
    int sh = stop - sbot; // The scrollable area height.

    float barHeight = (float)h/(float)sh;

    if (barHeight < 1)
    {
        float barY = (float)(y - sbot)/(float)sh;
        barY = barY < 0 ? 0 : barY > 1 ? 1 : barY;

        // Handle scroll bar logic.
        uint32_t hid = state.scrollId;
        int hx = x;
        int hy = y + (int)(barY * h);
        int hw = w;
        int hh = (int)(barHeight * h);

        const int range = h - (hh-1);
        bool over = inRect(hx, hy, hw, hh);
        buttonLogic(hid, over);
        if (isActive(hid))
        {
            float u = (float)(hy - y) / (float)range;
            if (state.wentActive)
            {
                state.dragY = state.my;
                state.dragOrig = u;
            }
            if (state.dragY != state.my)
            {
                u = state.dragOrig + (state.my - state.dragY) / (float)range;
                u = u < 0 ? 0 : u > 1 ? 1 : u;
                *state.scrollVal = (int)((1 - u) * (sh - h));
            }
        }

        // BG
        addGfxCmdRoundedRect((float)x, (float)y, (float)w, (float)h, (float)w/2-1, RGBA(0,0,0,196));
        // Bar
        if (isActive(hid))
        {
            addGfxCmdRoundedRect((float)hx, (float)hy, (float)hw, (float)hh, (float)w/2-1, RGBA(255,196,0,196));
        }
        else
        {
            addGfxCmdRoundedRect((float)hx, (float)hy, (float)hw, (float)hh, (float)w/2-1, isHot(hid) ? RGBA(255,196,0,96) : RGBA(255,255,255,64));
        }

        // Handle mouse scrolling.
        if (state.insideScrollArea) // && !anyActive())
        {
            if (state.scroll)
            {
                *state.scrollVal += 20 * state.scroll;
                if (*state.scrollVal < 0)
                {
                    *state.scrollVal = 0;
                }
                if (*state.scrollVal > (sh - h))
                {
                    *state.scrollVal = (sh - h);
                }
            }
        }
        if (*state.scrollVal > (sh - h))
        {
            *state.scrollVal = (sh - h);
        }
    }
    else
    {
        *state.scrollVal = 0;
    }
    state.insideCurrentScroll = false;
}
bool Imgui::button(const std::string& text, bool enabled)
{
    state.widgetId++;
    uint32_t id = (state.areaId<<16) | state.widgetId;

    int x = state.widgetX;
    int y = state.widgetY - BUTTON_HEIGHT;
    int w = state.widgetW - SLIDER_MARKER_WIDTH;
    int h = BUTTON_HEIGHT;
    state.widgetY -= BUTTON_HEIGHT + DEFAULT_SPACING;

    bool over = enabled && inRect(x, y, w, h);
    bool res = buttonLogic(id, over);

    addGfxCmdRoundedRect((float)x, (float)y, (float)w, (float)h, (float)BUTTON_HEIGHT/2-1, RGBA(128,128,128, isActive(id)?196:96));
    if (enabled)
    {
        addGfxCmdText(x+w/2, y+BUTTON_HEIGHT/2-TEXT_HEIGHT/2, ALIGN_CENTER, text, isHot(id) ? RGBA(255,196,0,255) : RGBA(255,255,255,200));
    }
    else
    {
        addGfxCmdText(x+w/2, y+BUTTON_HEIGHT/2-TEXT_HEIGHT/2, ALIGN_CENTER, text, RGBA(128,128,128,200));
    }

    return res;
}
bool Imgui::item(const std::string& text, bool enabled)
{
    state.widgetId++;
    uint32_t id = (state.areaId<<16) | state.widgetId;

    int x = state.widgetX;
    int y = state.widgetY - BUTTON_HEIGHT;
    int w = state.widgetW;
    int h = BUTTON_HEIGHT;
    state.widgetY -= BUTTON_HEIGHT + DEFAULT_SPACING;

    bool over = enabled && inRect(x, y, w, h);
    bool res = buttonLogic(id, over);

    if (isHot(id))
    {
        addGfxCmdRoundedRect((float)x, (float)y, (float)w, (float)h, 2.0f, RGBA(255,196,0,isActive(id)?196:96));
    }

    if (enabled)
    {
        addGfxCmdText(x+BUTTON_HEIGHT/2, y+BUTTON_HEIGHT/2-TEXT_HEIGHT/2, ALIGN_LEFT, text, RGBA(255,255,255,200));
    }
    else
    {
        addGfxCmdText(x+BUTTON_HEIGHT/2, y+BUTTON_HEIGHT/2-TEXT_HEIGHT/2, ALIGN_LEFT, text, RGBA(128,128,128,200));
    }

    return res;
}
bool Imgui::check(const std::string& text, bool checked, bool enabled)
{
    state.widgetId++;
    uint32_t id = (state.areaId<<16) | state.widgetId;

    int x = state.widgetX;
    int y = state.widgetY - BUTTON_HEIGHT;
    int w = state.widgetW;
    int h = BUTTON_HEIGHT;
    state.widgetY -= BUTTON_HEIGHT + DEFAULT_SPACING;

    bool over = enabled && inRect(x, y, w, h);
    bool res = buttonLogic(id, over);

    const int cx = x+w-BUTTON_HEIGHT/2-CHECK_SIZE/2;
    const int cy = y+BUTTON_HEIGHT/2-CHECK_SIZE/2;
    addGfxCmdRoundedRect((float)cx-3, (float)cy-3, (float)CHECK_SIZE+6, (float)CHECK_SIZE+6, 4, RGBA(128,128,128, isActive(id)?196:96));
    if (checked)
    {
        if (enabled)
        {
            addGfxCmdRoundedRect((float)cx, (float)cy, (float)CHECK_SIZE, (float)CHECK_SIZE, (float)CHECK_SIZE/2-1, RGBA(255,255,255,isActive(id)?255:200));
        }
        else
        {
            addGfxCmdRoundedRect((float)cx, (float)cy, (float)CHECK_SIZE, (float)CHECK_SIZE, (float)CHECK_SIZE/2-1, RGBA(128,128,128,200));
        }
    }

    if (enabled)
    {
        addGfxCmdText(x, y+BUTTON_HEIGHT/2-TEXT_HEIGHT/2, ALIGN_LEFT, text, isHot(id) ? RGBA(255,196,0,255) : RGBA(255,255,255,200));
    }
    else
    {
        addGfxCmdText(x, y+BUTTON_HEIGHT/2-TEXT_HEIGHT/2, ALIGN_LEFT, text, RGBA(128,128,128,200));
    }

    return res;
}
bool Imgui::collapse(const std::string& text, const std::string& subtext, bool checked, bool enabled)
{
    state.widgetId++;
    uint32_t id = (state.areaId << 16) | state.widgetId;

    int x = state.widgetX;
    int y = state.widgetY - BUTTON_HEIGHT;
    int w = state.widgetW;
    int h = BUTTON_HEIGHT;
    state.widgetY -= BUTTON_HEIGHT; // + DEFAULT_SPACING;

    const int cx = x + BUTTON_HEIGHT / 2 - CHECK_SIZE / 2;
    const int cy = y + BUTTON_HEIGHT / 2 - CHECK_SIZE / 2;

    bool over = enabled && inRect(x, y, w, h);
    bool res = buttonLogic(id, over);

    if (checked)
    {
        addGfxCmdTriangle(cx, cy, CHECK_SIZE, CHECK_SIZE, 2, RGBA(255,255,255, isActive(id)?255:200));
    }
    else
    {
        addGfxCmdTriangle(cx, cy, CHECK_SIZE, CHECK_SIZE, 1, RGBA(255,255,255, isActive(id)?200:150));
    }

    if (enabled)
    {
        addGfxCmdText(x+BUTTON_HEIGHT, y+BUTTON_HEIGHT/2-TEXT_HEIGHT/2, ALIGN_LEFT, text, isHot(id) ? RGBA(255,196,0,255) : RGBA(255,255,255,200));
    }
    else
    {
        addGfxCmdText(x+BUTTON_HEIGHT, y+BUTTON_HEIGHT/2-TEXT_HEIGHT/2, ALIGN_LEFT, text, RGBA(128,128,128,200));
    }

    if (subtext.length() > 0)
    {
        if (checked)
        {
            addGfxCmdText(x+w, y+BUTTON_HEIGHT/2-TEXT_HEIGHT/2, ALIGN_RIGHT, subtext, RGBA(255,255,255,178));
        }
        else
        {
            addGfxCmdText(x+w, y+BUTTON_HEIGHT/2-TEXT_HEIGHT/2, ALIGN_RIGHT, subtext, RGBA(255,255,255,128));
        }
    }

    return res;
}
void Imgui::label(const std::string& text, TextAlign align, bool dontMove)
{
    int x = state.widgetX;
    int y = state.widgetY - BUTTON_HEIGHT;
    if (!dontMove)
    {
        state.widgetY -= BUTTON_HEIGHT;
    }
    if (align == ALIGN_CENTER)
    {
        x += state.widgetW / 2;
    }
    else if (align == ALIGN_RIGHT)
    {
        x += state.widgetW;
    }
    addGfxCmdText(x, y + BUTTON_HEIGHT / 2 - TEXT_HEIGHT / 2, align, text, RGBA(255,255,255,255));
}
void Imgui::value(const std::string& text, TextAlign align)
{
    int x = state.widgetX;
    const int y = state.widgetY - BUTTON_HEIGHT;
    const int w = state.widgetW;
    state.widgetY -= BUTTON_HEIGHT;

    if (align == ALIGN_CENTER)
    {
        x += w / 2;
    }
    else if (align == ALIGN_RIGHT)
    {
        x += w - SLIDER_HEIGHT / 2;
    }

    addGfxCmdText(x, y + BUTTON_HEIGHT / 2 - TEXT_HEIGHT / 2, align, text, RGBA(255,255,255,200));
}
void Imgui::labelledValue(const std::string& label, const std::string& value)
{
    this->label(label, ALIGN_LEFT, true);
    this->value(value);
}
bool Imgui::slider(const std::string& text, float& val, float vmin, float vmax, float vinc, bool enabled)
{
    state.widgetId++;
    uint32_t id = (state.areaId << 16) | state.widgetId;

    int x = state.widgetX;
    int y = state.widgetY - BUTTON_HEIGHT;
    int w = state.widgetW - SLIDER_MARKER_WIDTH;
    int h = SLIDER_HEIGHT;
    state.widgetY -= SLIDER_HEIGHT + DEFAULT_SPACING;

    addGfxCmdRoundedRect((float)x, (float)y, (float)w, (float)h, 4.0f, RGBA(255,255,255,32));

    const int range = w - SLIDER_MARKER_WIDTH;

    float u = (val - vmin) / (vmax-vmin);
    u = u < 0 ? 0 : u > 1 ? 1 : u;
    int m = (int)(u * range);

    bool over = enabled && inRect(x + m, y, SLIDER_MARKER_WIDTH, SLIDER_HEIGHT);
    buttonLogic(id, over);
    bool valChanged = false;

    if (isActive(id))
    {
        if (state.wentActive)
        {
            state.dragX = state.mx;
            state.dragOrig = u;
        }
        if (state.dragX != state.mx)
        {
            u = state.dragOrig + (float)(state.mx - state.dragX) / (float)range;
            u = u < 0 ? 0 : u > 1 ? 1 : u;
            float oldval = val;
            val = vmin + u * (vmax-vmin);
            val = floorf(val / vinc + 0.5f) * vinc; // Snap to vinc
            m = (int)(u * range);
            valChanged = val != oldval;
        }
    }

    if (isActive(id))
    {
        addGfxCmdRoundedRect((float)(x + m),
                             (float)y,
                             (float)SLIDER_MARKER_WIDTH,
                             (float)SLIDER_HEIGHT,
                             4.0f,
                             RGBA(255,255,255,255));
    }
    else
    {
        addGfxCmdRoundedRect((float)(x + m),
                             (float)y,
                             (float)SLIDER_MARKER_WIDTH,
                             (float)SLIDER_HEIGHT,
                             4.0f,
                             isHot(id) ? RGBA(255,196,0,128) : RGBA(255,255,255,64));
    }

    int digits = (int)(ceilf(log10f(vinc)));
    char fmt[16];
    snprintf(fmt, 16, "%%.%df", digits >= 0 ? 0 : -digits);
    char msg[128];
    snprintf(msg, 128, fmt, val);

    if (enabled)
    {
        addGfxCmdText(x,
                      y + SLIDER_HEIGHT / 2 - TEXT_HEIGHT / 2,
                      ALIGN_LEFT,
                      text,
                      isHot(id) ? RGBA(255,196,0,255) : RGBA(255,255,255,200));
        addGfxCmdText(x + w,
                      y + SLIDER_HEIGHT / 2 - TEXT_HEIGHT / 2,
                      ALIGN_RIGHT,
                      msg,
                      isHot(id) ? RGBA(255,196,0,255) : RGBA(255,255,255,200));
    }
    else
    {
        addGfxCmdText(x,
                      y + SLIDER_HEIGHT / 2 - TEXT_HEIGHT / 2,
                      ALIGN_LEFT,
                      text,
                      RGBA(128,128,128,200));
        addGfxCmdText(x + w,
                      y + SLIDER_HEIGHT / 2 - TEXT_HEIGHT / 2,
                      ALIGN_RIGHT,
                      msg,
                      RGBA(128,128,128,200));
    }

    return valChanged;
}
void Imgui::indent()
{
    state.widgetX += INDENT_SIZE;
    state.widgetW -= INDENT_SIZE;
}
void Imgui::unindent()
{
    state.widgetX -= INDENT_SIZE;
    state.widgetW += INDENT_SIZE;
}
void Imgui::separator()
{
    state.widgetY -= DEFAULT_SPACING * 3;
}
void Imgui::separatorLine()
{
    int x = state.widgetX;
    int y = state.widgetY - DEFAULT_SPACING * 2;
    int w = state.widgetW;
    int h = 1;
    state.widgetY -= DEFAULT_SPACING * 4;

    addGfxCmdRect((float)x, (float)y, (float)w, (float)h, RGBA(255,255,255,32));
}
void Imgui::drawText(int x, int y, TextAlign align, const std::string& text, uint32_t color, float pointSize)
{
    addGfxCmdText(x, y, align, text, color, pointSize);
}
void Imgui::drawLine(float x0, float y0, float x1, float y1, float r, uint32_t color)
{
    addGfxCmdLine(x0, y0, x1, y1, r, color);
}
void Imgui::drawRect(float x, float y, float w, float h, uint32_t color)
{
    addGfxCmdRect(x, y, w, h, color);
}
void Imgui::drawRoundedRect(float x, float y, float w, float h, float r, uint32_t color)
{
    addGfxCmdRoundedRect(x, y, w, h, r, color);
}
void Imgui::drawTexturedRect(float x, float y, float w, float h, uint32_t color, uint32_t texture, float tx0, float ty0, float tx1, float ty1)
{
    addGfxCmdTexturedRect(x, y, w, h, color, texture, tx0, ty0, tx1, ty1);
}

void Imgui:: addGfxCmdScissor(int x, int y, int w, int h)
{
    gfxCmd cmd;
    cmd.type = GFXCMD_SCISSOR;
    cmd.flags = x < 0 ? 0 : 1;      // on/off flag.
    cmd.col = 0;
    cmd.rect.x = x;
    cmd.rect.y = y;
    cmd.rect.w = w;
    cmd.rect.h = h;
    renderQueue.push_back(cmd);
}
void Imgui:: addGfxCmdRect(float x, float y, float w, float h, uint32_t color)
{
    gfxCmd cmd;
    cmd.type = GFXCMD_RECT;
    cmd.flags = 0;
    cmd.col = color;
    cmd.rect.x = (x*8.0f);
    cmd.rect.y = (y*8.0f);
    cmd.rect.w = (w*8.0f);
    cmd.rect.h = (h*8.0f);
    cmd.rect.r = 0;
    renderQueue.push_back(cmd);
}

void Imgui:: addGfxCmdTexturedRect(float x, float y, float w, float h, uint32_t color, uint32_t texture, float tx0, float ty0, float tx1, float ty1)
{
    gfxCmd cmd;
    cmd.type = GFXCMD_TEXTURED_RECT;
    cmd.flags = 0;
    cmd.col = color;
    cmd.rect.x = (x*8.0f);
    cmd.rect.y = (y*8.0f);
    cmd.rect.w = (w*8.0f);
    cmd.rect.h = (h*8.0f);
    cmd.texturedRect.texture = texture;
    cmd.texturedRect.tx0 = tx0;
    cmd.texturedRect.ty0 = ty0;
    cmd.texturedRect.tx1 = tx1;
    cmd.texturedRect.ty1 = ty1;
    renderQueue.push_back(cmd);
}
void Imgui:: addGfxCmdLine(float x0, float y0, float x1, float y1, float r, uint32_t color)
{
    gfxCmd cmd;
    cmd.type = GFXCMD_LINE;
    cmd.flags = 0;
    cmd.col = color;
    cmd.line.x0 = (x0*8.0f);
    cmd.line.y0 = (y0*8.0f);
    cmd.line.x1 = (x1*8.0f);
    cmd.line.y1 = (y1*8.0f);
    cmd.line.r = (r*8.0f);
    renderQueue.push_back(cmd);
}
void Imgui:: addGfxCmdRoundedRect(float x, float y, float w, float h, float r, uint32_t color)
{
    gfxCmd cmd;
    cmd.type = GFXCMD_RECT;
    cmd.flags = 0;
    cmd.col = color;
    cmd.rect.x = (x*8.0f);
    cmd.rect.y = (y*8.0f);
    cmd.rect.w = (w*8.0f);
    cmd.rect.h = (h*8.0f);
    cmd.rect.r = (r*8.0f);
    renderQueue.push_back(cmd);
}
void Imgui:: addGfxCmdTriangle(int x, int y, int w, int h, int flags, uint32_t color)
{
    gfxCmd cmd;
    cmd.type = GFXCMD_TRIANGLE;
    cmd.flags = (char)flags;
    cmd.col = color;
    cmd.rect.x = (x*8.0f);
    cmd.rect.y = (y*8.0f);
    cmd.rect.w = (w*8.0f);
    cmd.rect.h = (h*8.0f);
    renderQueue.push_back(cmd);
}
void Imgui:: addGfxCmdText(int x, int y, TextAlign align, const std::string& text, uint32_t color, float pointSize)
{
    gfxCmd cmd;
    cmd.type = GFXCMD_TEXT;
    cmd.flags = 0;
    cmd.col = color;
    cmd.text.x = x;
    cmd.text.y = y;
    cmd.text.align = align;
    cmd.text.text = text;
    cmd.text.pointSize = (pointSize * 100);
    renderQueue.push_back(cmd);
}
