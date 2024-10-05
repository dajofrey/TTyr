// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Graphics.h"

#include "../Vulkan/Render.h"
#include "../OpenGL/Render.h"

#include "../Common/Log.h"
#include "../Common/Macros.h"
#include "../Common/Config.h"

#include "nh-gfx/Base/Viewport.h"
#include "nh-gfx/Fonts/HarfBuzz.h"

#include "nh-core/System/Memory.h"
#include "nh-core/Util/Math.h"
#include "nh-core/Util/Array.h"
#include "nh-core/Util/List.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// HELPER ==========================================================================================

static TTYR_TERMINAL_RESULT ttyr_terminal_getInternalMonospaceFonts(
    nh_core_List *Fonts_p)
{
TTYR_TERMINAL_BEGIN()
 
    nh_gfx_FontStyle Style;
    nh_gfx_parseFontStyle(&Style, "normal");

    nh_gfx_FontFamily Family = nh_gfx_initFontFamily(NULL);
    Family.generic_p[NH_GFX_GENERIC_FONT_FAMILY_MONOSPACE] = true;

    *Fonts_p = nh_gfx_getFonts(Family, Style, true);
    if (Fonts_p->size <= 0) {TTYR_TERMINAL_END(TTYR_TERMINAL_ERROR_BAD_STATE)}

    nh_gfx_freeFontStyle(&Style);

TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

static ttyr_terminal_GraphicsAction ttyr_terminal_initGraphicsAction()
{
TTYR_TERMINAL_BEGIN()

    ttyr_terminal_GraphicsAction Action;
    Action.init = true;

TTYR_TERMINAL_END(Action)
}

static nh_Color ttyr_terminal_getGradientColor(
    ttyr_terminal_Config *Config_p, ttyr_terminal_GraphicsGradient *Gradient_p)
{
TTYR_TERMINAL_BEGIN()

    if (Config_p->accents == 1) {
        TTYR_TERMINAL_END(Config_p->Accents_p[0])
    }

    nh_Color Color1 = Config_p->Accents_p[Gradient_p->index];
    nh_Color Color2 = Gradient_p->index == Config_p->accents-1 ? Config_p->Accents_p[0] : Config_p->Accents_p[Gradient_p->index+1];

    nh_Color Result;
    Result.r = Color1.r + Gradient_p->ratio * (Color2.r - Color1.r);
    Result.g = Color1.g + Gradient_p->ratio * (Color2.g - Color1.g);
    Result.b = Color1.b + Gradient_p->ratio * (Color2.b - Color1.b);
    Result.a = 1.0f;

    Gradient_p->ratio += 0.01f;

TTYR_TERMINAL_END(Result)
}

// INIT/FREE =======================================================================================

static TTYR_TERMINAL_RESULT ttyr_terminal_initGraphicsData(
    ttyr_terminal_GraphicsData *Data_p)
{
TTYR_TERMINAL_BEGIN()

    Data_p->Foreground.Action = ttyr_terminal_initGraphicsAction();
    Data_p->Foreground.Vertices = nh_core_initArray(sizeof(float), 1024);
    Data_p->Foreground.Indices  = nh_core_initArray(sizeof(uint32_t), 1024);
    Data_p->Foreground.Vertices2 = nh_core_initArray(sizeof(float), 1024);
    Data_p->Foreground.Indices2  = nh_core_initArray(sizeof(uint32_t), 1024);
    Data_p->Foreground.Ranges = nh_core_initArray(sizeof(ttyr_terminal_AttributeRange), 32);
    Data_p->Foreground.Ranges2 = nh_core_initArray(sizeof(ttyr_terminal_AttributeRange), 32);

    ttyr_terminal_initOpenGLForeground(&Data_p->Foreground.OpenGL);

    Data_p->Background.Action = ttyr_terminal_initGraphicsAction();
    Data_p->Background.Vertices = nh_core_initArray(sizeof(float), 1024);
    Data_p->Background.Indices  = nh_core_initArray(sizeof(uint32_t), 1024);
    Data_p->Background.Ranges = nh_core_initArray(sizeof(ttyr_terminal_AttributeRange), 32);

    ttyr_terminal_initOpenGLBackground(&Data_p->Background.OpenGL);

    Data_p->Boxes.Action = ttyr_terminal_initGraphicsAction();
    ttyr_terminal_initOpenGLBoxes(&Data_p->Boxes.OpenGL);
 
TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

static TTYR_TERMINAL_RESULT ttyr_terminal_initGraphicsState(
    ttyr_terminal_GraphicsState *State_p)
{
TTYR_TERMINAL_BEGIN()

    ttyr_terminal_Config Config = ttyr_terminal_getConfig();

    TTYR_TERMINAL_CHECK(ttyr_terminal_getInternalMonospaceFonts(&State_p->Fonts))

    State_p->Map = nh_core_createHashMap();
    State_p->Glyphs = nh_core_initList(128);
    State_p->Codepoints = nh_core_initList(128);

    State_p->Gradient.Color = ttyr_terminal_getGradientColor(&Config, &State_p->Gradient);
    State_p->Gradient.interval = 0.1;
    State_p->Gradient.index = 0;
    State_p->Gradient.ratio = 0.0f;
    State_p->Gradient.LastChange = nh_core_getSystemTime();

    State_p->Blink.LastBlink = nh_core_getSystemTime();
    State_p->Blink.on = false;
 
TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

TTYR_TERMINAL_RESULT ttyr_terminal_initGraphics(
    ttyr_terminal_Graphics *Graphics_p)
{
TTYR_TERMINAL_BEGIN()

    memset(Graphics_p, 0, sizeof(ttyr_terminal_Graphics));

    TTYR_TERMINAL_CHECK(ttyr_terminal_initGraphicsState(&Graphics_p->State))
    TTYR_TERMINAL_CHECK(ttyr_terminal_initGraphicsData(&Graphics_p->Data1))
    TTYR_TERMINAL_CHECK(ttyr_terminal_initGraphicsData(&Graphics_p->Data2))
 
TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

static TTYR_TERMINAL_RESULT ttyr_terminal_freeGraphicsData(
    ttyr_terminal_GraphicsData *Data_p)
{
TTYR_TERMINAL_BEGIN()

    nh_core_freeArray(&Data_p->Foreground.Vertices);
    nh_core_freeArray(&Data_p->Foreground.Indices);
    nh_core_freeArray(&Data_p->Foreground.Ranges);
    nh_core_freeArray(&Data_p->Foreground.Vertices2);
    nh_core_freeArray(&Data_p->Foreground.Indices2);
    nh_core_freeArray(&Data_p->Foreground.Ranges2);

    ttyr_terminal_freeOpenGLForeground(&Data_p->Foreground.OpenGL);

    nh_core_freeArray(&Data_p->Background.Vertices);
    nh_core_freeArray(&Data_p->Background.Indices);
    nh_core_freeArray(&Data_p->Background.Ranges);
 
    ttyr_terminal_freeOpenGLBackground(&Data_p->Background.OpenGL);
    ttyr_terminal_freeOpenGLBoxes(&Data_p->Boxes.OpenGL);

TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

TTYR_TERMINAL_RESULT ttyr_terminal_freeGraphics(
    ttyr_terminal_Graphics *Graphics_p)
{
TTYR_TERMINAL_BEGIN()

    TTYR_TERMINAL_CHECK(ttyr_terminal_freeGraphicsData(&Graphics_p->Data1))
    TTYR_TERMINAL_CHECK(ttyr_terminal_freeGraphicsData(&Graphics_p->Data2))

    nh_core_freeList(&Graphics_p->State.Fonts, false);
    nh_core_freeList(&Graphics_p->State.Glyphs, true);
    nh_core_freeList(&Graphics_p->State.Codepoints, true);

    nh_core_freeHashMap(Graphics_p->State.Map);

TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

// RANGES ==========================================================================================

static int ttyr_terminal_getCurrentAttributeRangeForLineGraphics(
    ttyr_terminal_Grid *Grid_p, ttyr_tty_Glyph *Current_p, int *col_p, int *row_p)
{
TTYR_TERMINAL_BEGIN()

    int total = 0;

    for (int row = *row_p; row < Grid_p->rows; ++row) {
        for (int col = *col_p; col < Grid_p->cols; ++col) {
            ttyr_tty_Glyph Glyph = ((ttyr_terminal_Tile*)((nh_core_List*)Grid_p->Rows.pp[row])->pp[col])->Glyph;
            if (!(Glyph.mark & TTYR_TTY_MARK_LINE_GRAPHICS)) {
                continue;
            }
            if (ttyr_terminal_compareForegroundAttributes(Current_p, &Glyph)) {
                *col_p = col;
                *row_p = row;
                *Current_p = Glyph;
                TTYR_TERMINAL_END(total)
            }
            ++total;
        }
        *col_p = 0;
    }

    *col_p = Grid_p->cols;
    *row_p = Grid_p->rows;

TTYR_TERMINAL_END(total)
}

static int ttyr_terminal_getCurrentAttributeRange(
    ttyr_terminal_Grid *Grid_p, ttyr_tty_Glyph *Current_p, int *col_p, int *row_p, bool foreground)
{
TTYR_TERMINAL_BEGIN()

    int total = 0;

    for (int row = *row_p; row < Grid_p->rows; ++row) {
        for (int col = *col_p; col < Grid_p->cols; ++col) {
            ttyr_tty_Glyph Glyph = ((ttyr_terminal_Tile*)((nh_core_List*)Grid_p->Rows.pp[row])->pp[col])->Glyph;
            if (foreground && (!Glyph.codepoint || Glyph.codepoint == ' ' || Glyph.mark & TTYR_TTY_MARK_LINE_GRAPHICS)) {
                continue;
            }
            if (!foreground && (!Glyph.Background.custom && !Glyph.Attributes.reverse && !Glyph.Attributes.blink)) {
                continue;
            }
            if ((foreground && ttyr_terminal_compareForegroundAttributes(Current_p, &Glyph))
            || (!foreground && ttyr_terminal_compareBackgroundAttributes(Current_p, &Glyph))) {
                *col_p = col;
                *row_p = row;
                *Current_p = Glyph;
                TTYR_TERMINAL_END(total)
            }
            ++total;
        }
        *col_p = 0;
    }

    *col_p = Grid_p->cols;
    *row_p = Grid_p->rows;

TTYR_TERMINAL_END(total)
}

static TTYR_TERMINAL_RESULT ttyr_terminal_computeRangeForLineGraphics(
    ttyr_terminal_GraphicsData *Data_p, ttyr_terminal_Grid *Grid_p)
{
TTYR_TERMINAL_BEGIN()

    nh_core_freeArray(&Data_p->Foreground.Ranges2);
    Data_p->Foreground.Ranges2 = nh_core_initArray(sizeof(ttyr_terminal_AttributeRange), 32);

    if (Grid_p->rows <= 0 || Grid_p->Rows.size == 0) {
        TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
    }

    int total = 0;
    int row = 0;
    int col = 0;

    ttyr_tty_Glyph Glyph = ((ttyr_terminal_Tile*)((nh_core_List*)Grid_p->Rows.pp[0])->pp[0])->Glyph;
    bool begin = true;

    while (true)
    {
        ttyr_tty_Glyph NextGlyph = Glyph;
        total = ttyr_terminal_getCurrentAttributeRangeForLineGraphics(Grid_p, &NextGlyph, &col, &row) * 12;
        if (!total && !begin) {break;}
        begin = false;

        ttyr_terminal_AttributeRange *Range_p = nh_core_incrementArray(&Data_p->Foreground.Ranges2);

        Range_p->Glyph = Glyph;
        Range_p->indices = total;

        Glyph = NextGlyph;
    }

TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

static TTYR_TERMINAL_RESULT ttyr_terminal_computeRange(
    ttyr_terminal_GraphicsData *Data_p, ttyr_terminal_Grid *Grid_p, bool foreground)
{
TTYR_TERMINAL_BEGIN()

    if (foreground) {
        nh_core_freeArray(&Data_p->Foreground.Ranges);
        Data_p->Foreground.Ranges = nh_core_initArray(sizeof(ttyr_terminal_AttributeRange), 32);
    } else {
        nh_core_freeArray(&Data_p->Background.Ranges);
        Data_p->Background.Ranges = nh_core_initArray(sizeof(ttyr_terminal_AttributeRange), 32);
    }

    if (Grid_p->rows <= 0 || Grid_p->Rows.size == 0) {
        TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
    }

    int total = 0;
    int row = 0;
    int col = 0;

    ttyr_tty_Glyph Glyph = ((ttyr_terminal_Tile*)((nh_core_List*)Grid_p->Rows.pp[0])->pp[0])->Glyph;
    bool begin = true;

    // Default Foreground/Background.
    while (true)
    {
        ttyr_tty_Glyph NextGlyph = Glyph;
        total = ttyr_terminal_getCurrentAttributeRange(Grid_p, &NextGlyph, &col, &row, foreground) * 6;
        if (!total && !begin) {break;}
        begin = false;

        ttyr_terminal_AttributeRange *Range_p = nh_core_incrementArray(
            foreground ? &Data_p->Foreground.Ranges : &Data_p->Background.Ranges);

        Range_p->Glyph = Glyph;
        Range_p->indices = total;

        Glyph = NextGlyph;
    }

TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

// UPDATE ==========================================================================================

static TTYR_TERMINAL_RESULT ttyr_terminal_updateForegroundData(
    ttyr_terminal_Grid *Grid_p, ttyr_terminal_GraphicsForeground *Foreground_p)
{
TTYR_TERMINAL_BEGIN()

    nh_core_freeArray(&Foreground_p->Vertices);
    nh_core_freeArray(&Foreground_p->Indices);
    nh_core_freeArray(&Foreground_p->Vertices2);
    nh_core_freeArray(&Foreground_p->Indices2);

    nh_core_Array Vertices = nh_core_initArray(sizeof(float), 2000);
    nh_core_Array Indices = nh_core_initArray(sizeof(uint32_t), 1024);
    nh_core_Array Vertices2 = nh_core_initArray(sizeof(float), 2000);
    nh_core_Array Indices2 = nh_core_initArray(sizeof(uint32_t), 1024);

    int offset1 = 0;
    int offset2 = 0;

    for (int i = 0; i < Grid_p->Rows.size; ++i) {
        nh_core_List *Row_p = Grid_p->Rows.pp[i];

        for (int j = 0; j < Row_p->size; ++j) {
            ttyr_terminal_Tile *Tile_p = Row_p->pp[j];
            if (!Tile_p || !Tile_p->Glyph.codepoint || Tile_p->Glyph.codepoint == ' ') {continue;}

            if (Tile_p->Glyph.mark & TTYR_TTY_MARK_LINE_GRAPHICS) {
                nh_core_appendToArray(&Vertices2, Tile_p->Foreground.vertices_p, 24);
                uint32_t indices_p[12] = {
                    offset1, offset1 + 1, offset1 + 2, offset1, offset1 + 2, offset1 + 3,
                    offset1 + 4, offset1 + 5, offset1 + 6, offset1 + 4, offset1 + 6, offset1 + 7
                };
                nh_core_appendToArray(&Indices2, indices_p, 12);
                offset1 += 8;
            } else {
                nh_core_appendToArray(&Vertices, Tile_p->Foreground.vertices_p, 20);
                uint32_t indices_p[6] = {offset2, offset2 + 1, offset2 + 2, offset2, offset2 + 2, offset2 + 3};
                nh_core_appendToArray(&Indices, indices_p, 6);
                offset2 += 4;
            }
        }
    }

    Foreground_p->Vertices = Vertices;
    Foreground_p->Indices = Indices;
    Foreground_p->Vertices2 = Vertices2;
    Foreground_p->Indices2 = Indices2;

TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

static TTYR_TERMINAL_RESULT ttyr_terminal_updateBackgroundData(
    ttyr_terminal_Grid *Grid_p, ttyr_terminal_GraphicsBackground *Background_p)
{
TTYR_TERMINAL_BEGIN()

    nh_core_freeArray(&Background_p->Vertices);
    nh_core_freeArray(&Background_p->Indices);

    nh_core_Array Vertices = nh_core_initArray(sizeof(float), 2000);
    nh_core_Array Indices = nh_core_initArray(sizeof(uint32_t), 1024);

    int offset = 0;

    for (int i = 0; i < Grid_p->Rows.size; ++i) {
        nh_core_List *Row_p = Grid_p->Rows.pp[i];
        for (int j = 0; j < Row_p->size; ++j) {
            ttyr_terminal_Tile *Tile_p = Row_p->pp[j];
            if (!Tile_p->Glyph.Background.custom && !Tile_p->Glyph.Attributes.reverse && !Tile_p->Glyph.Attributes.blink) {continue;}
            nh_core_appendToArray(&Vertices, Tile_p->Background.vertices_p, 12);
            uint32_t indices_p[6] = {offset, offset + 1, offset + 2, offset, offset + 2, offset + 3};
            nh_core_appendToArray(&Indices, indices_p, 6);
            offset += 4;
        }
    }

    Background_p->Vertices = Vertices;
    Background_p->Indices = Indices;

TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

static TTYR_TERMINAL_RESULT ttyr_terminal_updateBoxesData(
    ttyr_terminal_Grid *Grid_p, ttyr_terminal_GraphicsBoxes *Boxes_p)
{
TTYR_TERMINAL_BEGIN()

    nh_core_freeArray(&Boxes_p->Vertices);
    Boxes_p->Vertices = nh_core_initArray(sizeof(float), 64);
 
    for (int i = 0; i < Grid_p->Boxes.length; ++i) {
        nh_core_appendToArray(
            &Boxes_p->Vertices, ((ttyr_terminal_Box*)Grid_p->Boxes.p)[i].innerVertices_p, 18);
        nh_core_appendToArray(
            &Boxes_p->Vertices, ((ttyr_terminal_Box*)Grid_p->Boxes.p)[i].outerVertices_p, 18);
    }

TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

TTYR_TERMINAL_RESULT ttyr_terminal_updateGraphicsData(
    ttyr_terminal_GraphicsData *Data_p, ttyr_terminal_Grid *Grid_p)
{
TTYR_TERMINAL_BEGIN()

    TTYR_TERMINAL_CHECK(ttyr_terminal_updateForegroundData(Grid_p, &Data_p->Foreground))
    TTYR_TERMINAL_CHECK(ttyr_terminal_updateBackgroundData(Grid_p, &Data_p->Background))
    TTYR_TERMINAL_CHECK(ttyr_terminal_updateBoxesData(Grid_p, &Data_p->Boxes))

    TTYR_TERMINAL_CHECK(ttyr_terminal_computeRange(Data_p, Grid_p, true))
    TTYR_TERMINAL_CHECK(ttyr_terminal_computeRange(Data_p, Grid_p, false))
    TTYR_TERMINAL_CHECK(ttyr_terminal_computeRangeForLineGraphics(Data_p, Grid_p))

TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

bool ttyr_terminal_updateBlinkOrGradient(
    ttyr_terminal_GraphicsState *State_p)
{
TTYR_TERMINAL_BEGIN()

    bool update = false;
    nh_core_SystemTime Time = nh_core_getSystemTime();

    ttyr_terminal_Config Config = ttyr_terminal_getConfig();
    if (nh_core_getSystemTimeDiffInSeconds(State_p->Blink.LastBlink, Time) >= Config.blinkFrequency) {
        update = true;
        State_p->Blink.LastBlink = Time;
        State_p->Blink.on = !State_p->Blink.on;
    }

    if (Config.accents > 1 && nh_core_getSystemTimeDiffInSeconds(State_p->Gradient.LastChange, Time) >= State_p->Gradient.interval) { 
        update = true; 
        State_p->Gradient.LastChange = Time; 
        if (State_p->Gradient.ratio >= 1.0f) {
            State_p->Gradient.index = State_p->Gradient.index == Config.accents-1 ? 0 : State_p->Gradient.index+1; 
            State_p->Gradient.ratio = 0.0f;
        }
        State_p->Gradient.Color = ttyr_terminal_getGradientColor(&Config, &State_p->Gradient); 
    } 

TTYR_TERMINAL_END(update)
}

// VIEWPORT ========================================================================================

TTYR_TERMINAL_RESULT ttyr_terminal_handleViewportChange(
    ttyr_terminal_Graphics *Graphics_p, nh_gfx_Viewport *Viewport_p)
{
TTYR_TERMINAL_BEGIN()

    // Check if it's the initial call.
    if (!Graphics_p->State.Viewport_p) 
    {
        switch (Viewport_p->Surface_p->api)
        {
            case NH_API_GRAPHICS_BACKEND_VULKAN :
//                ttyr_terminal_initVulkanText(Viewport_p->Surface_p->Vulkan.GPU_p, 
//                    &Graphics_p->Foreground.Vulkan);
                break;
            case NH_API_GRAPHICS_BACKEND_OPENGL :
                break;
            default :
                TTYR_TERMINAL_END(TTYR_TERMINAL_ERROR_BAD_STATE)
        }
    }

    ttyr_terminal_Config Config = ttyr_terminal_getConfig();
 
    Viewport_p->Settings.ClearColor = Config.Background;
    Graphics_p->State.Viewport_p = Viewport_p;

TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

// RENDER ==========================================================================================

TTYR_TERMINAL_RESULT ttyr_terminal_renderGraphics(
    ttyr_terminal_Graphics *Graphics_p, ttyr_terminal_Grid *Grid_p, ttyr_terminal_Grid *Grid2_p)
{
TTYR_TERMINAL_BEGIN()

    switch (Graphics_p->State.Viewport_p->Surface_p->api)
    {
        case NH_API_GRAPHICS_BACKEND_VULKAN :
//            TTYR_TERMINAL_CHECK(ttyr_terminal_renderUsingVulkan(Graphics_p))
            break;
       case NH_API_GRAPHICS_BACKEND_OPENGL :
            TTYR_TERMINAL_CHECK(ttyr_terminal_renderUsingOpenGL(Graphics_p, Grid_p, Grid2_p))
            break;
        default :
            TTYR_TERMINAL_END(TTYR_TERMINAL_ERROR_BAD_STATE)
    }

TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

// COLOR ===========================================================================================

nh_Color ttyr_terminal_getGlyphColor(
    ttyr_terminal_GraphicsState *State_p, ttyr_tty_Glyph *Glyph_p, bool foreground)
{
TTYR_TERMINAL_BEGIN()

    ttyr_terminal_Config Config = ttyr_terminal_getConfig();
 
    if (foreground) {
        if (Glyph_p->Attributes.reverse || (Glyph_p->Attributes.blink && State_p->Blink.on)) {
            if (Glyph_p->Background.custom) {
                TTYR_TERMINAL_END(Glyph_p->Background.Color)
            }
            TTYR_TERMINAL_END(Config.Background)
        }
        if (Glyph_p->mark & TTYR_TTY_MARK_ACCENT) {
            TTYR_TERMINAL_END(State_p->Gradient.Color)
        }
        if (Glyph_p->Foreground.custom) {
            TTYR_TERMINAL_END(Glyph_p->Foreground.Color)
        }
        TTYR_TERMINAL_END(Config.Foreground)
    }

    // Background.
    if ((Glyph_p->Attributes.reverse && !(Glyph_p->Attributes.blink && State_p->Blink.on)) 
    || (!Glyph_p->Attributes.reverse &&   Glyph_p->Attributes.blink && State_p->Blink.on)) {
        if (Glyph_p->mark & TTYR_TTY_MARK_ACCENT) {
            TTYR_TERMINAL_END(State_p->Gradient.Color)
        }
        if (Glyph_p->Foreground.custom) {
            TTYR_TERMINAL_END(Glyph_p->Foreground.Color)
        }
        TTYR_TERMINAL_END(Config.Foreground)
    }
    if (Glyph_p->Background.custom) {
        TTYR_TERMINAL_END(Glyph_p->Background.Color)
    }

TTYR_TERMINAL_END(Config.Background)
}

