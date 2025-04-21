// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Graphics.h"
#include "Color.h"

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
    nh_gfx_FontStyle Style;
    nh_gfx_parseFontStyle(&Style, "normal");

    nh_gfx_FontFamily Family = nh_gfx_initFontFamily(NULL);
    Family.generic_p[NH_GFX_GENERIC_FONT_FAMILY_MONOSPACE] = true;

    *Fonts_p = nh_gfx_getFonts(Family, Style, true);
    if (Fonts_p->size <= 0) {return TTYR_TERMINAL_ERROR_BAD_STATE;}

    nh_gfx_freeFontStyle(&Style);

    return TTYR_TERMINAL_SUCCESS;
}

static ttyr_terminal_GraphicsAction ttyr_terminal_initGraphicsAction()
{
    ttyr_terminal_GraphicsAction Action;
    Action.init = true;

    return Action;
}

// INIT/FREE =======================================================================================

static TTYR_TERMINAL_RESULT ttyr_terminal_initGraphicsData(
    ttyr_terminal_GraphicsData *Data_p)
{
    Data_p->Foreground.Action = ttyr_terminal_initGraphicsAction();
    Data_p->Foreground.Vertices = nh_core_initArray(sizeof(float), 1024);
    Data_p->Foreground.Indices  = nh_core_initArray(sizeof(uint32_t), 1024);
    Data_p->Foreground.Vertices2 = nh_core_initArray(sizeof(float), 1024);
    Data_p->Foreground.Indices2  = nh_core_initArray(sizeof(uint32_t), 1024);
    Data_p->Foreground.Ranges = nh_core_initArray(sizeof(ttyr_terminal_AttributeRange), 32);
    Data_p->Foreground.Ranges2 = nh_core_initArray(sizeof(ttyr_terminal_AttributeRange), 32);
    Data_p->Foreground.Colors = nh_core_initArray(sizeof(float), 2048);
    Data_p->Foreground.Colors2 = nh_core_initArray(sizeof(float), 2048);

    ttyr_terminal_initOpenGLForeground(&Data_p->Foreground.OpenGL);

    Data_p->Background.Action = ttyr_terminal_initGraphicsAction();
    Data_p->Background.Vertices = nh_core_initArray(sizeof(float), 1024);
    Data_p->Background.Colors = nh_core_initArray(sizeof(float), 2048);
    Data_p->Background.Indices  = nh_core_initArray(sizeof(uint32_t), 1024);
    Data_p->Background.Ranges = nh_core_initArray(sizeof(ttyr_terminal_AttributeRange), 32);

    ttyr_terminal_initOpenGLBackground(&Data_p->Background.OpenGL);

    Data_p->Boxes.Action = ttyr_terminal_initGraphicsAction();
    ttyr_terminal_initOpenGLBoxes(&Data_p->Boxes.OpenGL);
 
    Data_p->Dim.Action = ttyr_terminal_initGraphicsAction();
    Data_p->Dim.Vertices = nh_core_initArray(sizeof(float), 255);
    Data_p->Dim.Colors = nh_core_initArray(sizeof(float), 255);

    ttyr_terminal_initOpenGLDim(&Data_p->Dim.OpenGL);

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_initGraphicsState(
    ttyr_terminal_GraphicsState *State_p)
{
    memset(State_p, 0, sizeof(ttyr_terminal_GraphicsState));

    ttyr_terminal_Config Config = ttyr_terminal_getConfig();

    TTYR_TERMINAL_CHECK(ttyr_terminal_getInternalMonospaceFonts(&State_p->Fonts))

    State_p->Map = nh_core_createHashMap();
    State_p->Glyphs = nh_core_initList(128);
    State_p->Codepoints = nh_core_initList(128);

    State_p->AccentGradient.Color = ttyr_terminal_getGradientColor(&State_p->AccentGradient, Config.Accents_p, Config.accents);
    State_p->AccentGradient.interval = 0.1;
    State_p->AccentGradient.LastChange = nh_core_getSystemTime();

    State_p->BackgroundGradient = State_p->AccentGradient;
    State_p->BackgroundGradient.Color = ttyr_terminal_getGradientColor(&State_p->BackgroundGradient, Config.Backgrounds_p, Config.backgrounds);

    State_p->Blink.LastBlink = nh_core_getSystemTime();
 
    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_initGraphics(
    ttyr_terminal_Graphics *Graphics_p)
{
    memset(Graphics_p, 0, sizeof(ttyr_terminal_Graphics));

    TTYR_TERMINAL_CHECK(ttyr_terminal_initGraphicsState(&Graphics_p->State))
    TTYR_TERMINAL_CHECK(ttyr_terminal_initGraphicsData(&Graphics_p->Data1))
    TTYR_TERMINAL_CHECK(ttyr_terminal_initGraphicsData(&Graphics_p->Data2))
    TTYR_TERMINAL_CHECK(ttyr_terminal_initGraphicsData(&Graphics_p->BorderData))
 
    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_freeGraphicsData(
    ttyr_terminal_GraphicsData *Data_p)
{
    nh_core_freeArray(&Data_p->Foreground.Vertices);
    nh_core_freeArray(&Data_p->Foreground.Indices);
    nh_core_freeArray(&Data_p->Foreground.Ranges);
    nh_core_freeArray(&Data_p->Foreground.Vertices2);
    nh_core_freeArray(&Data_p->Foreground.Indices2);
    nh_core_freeArray(&Data_p->Foreground.Ranges2);
    nh_core_freeArray(&Data_p->Foreground.Colors);
    nh_core_freeArray(&Data_p->Foreground.Colors2);

    ttyr_terminal_freeOpenGLForeground(&Data_p->Foreground.OpenGL);

    nh_core_freeArray(&Data_p->Background.Vertices);
    nh_core_freeArray(&Data_p->Background.Colors);
    nh_core_freeArray(&Data_p->Background.Indices);
    nh_core_freeArray(&Data_p->Background.Ranges);
 
    ttyr_terminal_freeOpenGLBackground(&Data_p->Background.OpenGL);
    ttyr_terminal_freeOpenGLBoxes(&Data_p->Boxes.OpenGL);

    nh_core_freeArray(&Data_p->Dim.Vertices);
    nh_core_freeArray(&Data_p->Dim.Colors);
 
    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_freeGraphics(
    ttyr_terminal_Graphics *Graphics_p)
{
    TTYR_TERMINAL_CHECK(ttyr_terminal_freeGraphicsData(&Graphics_p->Data1))
    TTYR_TERMINAL_CHECK(ttyr_terminal_freeGraphicsData(&Graphics_p->Data2))
    TTYR_TERMINAL_CHECK(ttyr_terminal_freeGraphicsData(&Graphics_p->BorderData))

    nh_core_freeList(&Graphics_p->State.Fonts, false);
    nh_core_freeList(&Graphics_p->State.Glyphs, true);
    nh_core_freeList(&Graphics_p->State.Codepoints, true);

    nh_core_freeHashMap(Graphics_p->State.Map);

    return TTYR_TERMINAL_SUCCESS;
}

// RANGES ==========================================================================================

static int ttyr_terminal_getCurrentAttributeRangeForLineGraphics(
    ttyr_terminal_Grid *Grid_p, ttyr_core_Glyph *Current_p, int *col_p, int *row_p, 
    nh_core_Array *Cols_p, nh_core_Array *Rows_p)
{
    int total = 0;

    for (int row = *row_p; row < Grid_p->rows; ++row) {
        for (int col = *col_p; col < Grid_p->cols; ++col) {
            ttyr_core_Glyph Glyph = ((ttyr_terminal_Tile*)((nh_core_List*)Grid_p->Rows.pp[row])->pp[col])->Glyph;
            if (!(Glyph.mark & TTYR_CORE_MARK_LINE_GRAPHICS)) {
                continue;
            }
            if (ttyr_terminal_compareForegroundAttributes(Current_p, &Glyph)) {
                *col_p = col;
                *row_p = row;
                *Current_p = Glyph;
                return total;
            }
            ++total;
            int *c_p = nh_core_incrementArray(Cols_p);
            *c_p = col;
            int *r_p = nh_core_incrementArray(Rows_p);
            *r_p = row;
        }
        *col_p = 0;
    }

    *col_p = Grid_p->cols;
    *row_p = Grid_p->rows;

    return total;
}

static int ttyr_terminal_getCurrentAttributeRange(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_Grid *Grid_p, ttyr_core_Glyph *Current_p, int *col_p, 
    int *row_p, bool foreground, nh_core_Array *Cols_p, nh_core_Array *Rows_p)
{
    int total = 0;

    for (int row = *row_p; row < Grid_p->rows; ++row) {
        for (int col = *col_p; col < Grid_p->cols; ++col) {
            ttyr_core_Glyph Glyph = ((ttyr_terminal_Tile*)((nh_core_List*)Grid_p->Rows.pp[row])->pp[col])->Glyph;
            if (foreground && (!Glyph.codepoint || Glyph.codepoint == ' ' || Glyph.mark & TTYR_CORE_MARK_LINE_GRAPHICS)) {
                continue;
            }
            if (!foreground && (!Glyph.Background.custom && !Glyph.Attributes.reverse && !Glyph.Attributes.blink)) {
                continue;
            }
            if (!foreground && Glyph.Attributes.blink && !State_p->Blink.on) {
                continue;
            }
            if ((foreground && ttyr_terminal_compareForegroundAttributes(Current_p, &Glyph))
            || (!foreground && ttyr_terminal_compareBackgroundAttributes(Current_p, &Glyph))) {
                *col_p = col;
                *row_p = row;
                *Current_p = Glyph;
                return total;
            }
            int *c_p = nh_core_incrementArray(Cols_p);
            *c_p = col;
            int *r_p = nh_core_incrementArray(Rows_p);
            *r_p = row;
            ++total;
        }
        *col_p = 0;
    }

    *col_p = Grid_p->cols;
    *row_p = Grid_p->rows;

    return total;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_computeRangeForLineGraphics(
    ttyr_terminal_GraphicsData *Data_p, ttyr_terminal_Grid *Grid_p)
{
    nh_core_freeArray(&Data_p->Foreground.Ranges2);
    Data_p->Foreground.Ranges2 = nh_core_initArray(sizeof(ttyr_terminal_AttributeRange), 64);

    if (Grid_p->rows <= 0 || Grid_p->Rows.size == 0) {
        return TTYR_TERMINAL_SUCCESS;
    }

    int total = 0;
    int row = 0;
    int col = 0;

    ttyr_core_Glyph Glyph = ((ttyr_terminal_Tile*)((nh_core_List*)Grid_p->Rows.pp[0])->pp[0])->Glyph;
    bool begin = true;

    while (true)
    {
        ttyr_core_Glyph NextGlyph = Glyph;
        nh_core_Array Cols = nh_core_initArray(sizeof(int), 255);
        nh_core_Array Rows = nh_core_initArray(sizeof(int), 255);
        total = ttyr_terminal_getCurrentAttributeRangeForLineGraphics(Grid_p, &NextGlyph, &col, &row, &Cols, &Rows) * 12;
        if (!total && !begin) {break;}
        begin = false;

        ttyr_terminal_AttributeRange *Range_p = nh_core_incrementArray(&Data_p->Foreground.Ranges2);

        Range_p->Cols = Cols;
        Range_p->Rows = Rows;
        Range_p->Glyph = Glyph;
        Range_p->indices = total;

        Glyph = NextGlyph;
    }

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_computeRange(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_GraphicsData *Data_p, ttyr_terminal_Grid *Grid_p, 
    bool foreground)
{
    if (foreground) {
        nh_core_freeArray(&Data_p->Foreground.Ranges);
        Data_p->Foreground.Ranges = nh_core_initArray(sizeof(ttyr_terminal_AttributeRange), 32);
    } else {
        nh_core_freeArray(&Data_p->Background.Ranges);
        Data_p->Background.Ranges = nh_core_initArray(sizeof(ttyr_terminal_AttributeRange), 32);
    }

    if (Grid_p->rows <= 0 || Grid_p->Rows.size == 0) {
        return TTYR_TERMINAL_SUCCESS;
    }

    int total = 0;
    int row = 0;
    int col = 0;

    ttyr_core_Glyph Glyph = ((ttyr_terminal_Tile*)((nh_core_List*)Grid_p->Rows.pp[0])->pp[0])->Glyph;
    bool begin = true;

    // Default Foreground/Background.
    while (true)
    {
        ttyr_core_Glyph NextGlyph = Glyph;
        nh_core_Array Cols = nh_core_initArray(sizeof(int), 255);
        nh_core_Array Rows = nh_core_initArray(sizeof(int), 255);
 
        total = ttyr_terminal_getCurrentAttributeRange(State_p, Grid_p, &NextGlyph, &col, &row, foreground, &Cols, &Rows) * 6;
        if (!total && !begin) {break;}
        begin = false;

        ttyr_terminal_AttributeRange *Range_p = nh_core_incrementArray(
            foreground ? &Data_p->Foreground.Ranges : &Data_p->Background.Ranges);

        Range_p->Glyph = Glyph;
        Range_p->indices = total;
        Range_p->Cols = Cols;
        Range_p->Rows = Rows;

        Glyph = NextGlyph;
    }

    return TTYR_TERMINAL_SUCCESS;
}

// UPDATE ==========================================================================================

static TTYR_TERMINAL_RESULT ttyr_terminal_updateForegroundData(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_Grid *Grid_p, ttyr_terminal_GraphicsForeground *Foreground_p)
{
    nh_core_freeArray(&Foreground_p->Vertices);
    nh_core_freeArray(&Foreground_p->Indices);
    nh_core_freeArray(&Foreground_p->Vertices2);
    nh_core_freeArray(&Foreground_p->Indices2);
    nh_core_freeArray(&Foreground_p->Colors);
    nh_core_freeArray(&Foreground_p->Colors2);

    nh_core_Array Vertices = nh_core_initArray(sizeof(float), 2000);
    nh_core_Array Indices = nh_core_initArray(sizeof(uint32_t), 1024);
    nh_core_Array Vertices2 = nh_core_initArray(sizeof(float), 2000);
    nh_core_Array Indices2 = nh_core_initArray(sizeof(uint32_t), 1024);
    nh_core_Array Colors = nh_core_initArray(sizeof(float), 2048);
    nh_core_Array Colors2 = nh_core_initArray(sizeof(float), 2048);

    int offset1 = 0;
    int offset2 = 0;

    for (int i = 0; i < Grid_p->Rows.size; ++i) {
        nh_core_List *Row_p = Grid_p->Rows.pp[i];

        for (int j = 0; j < Row_p->size; ++j) {
            ttyr_terminal_Tile *Tile_p = Row_p->pp[j];
            if (!Tile_p || !Tile_p->Glyph.codepoint || Tile_p->Glyph.codepoint == ' ') {continue;}

            if (Tile_p->Glyph.mark & TTYR_CORE_MARK_LINE_GRAPHICS) {
                nh_core_appendToArray(&Vertices2, Tile_p->Foreground.vertices_p, 24);
                uint32_t indices_p[12] = {
                    offset1, offset1 + 1, offset1 + 2, offset1, offset1 + 2, offset1 + 3,
                    offset1 + 4, offset1 + 5, offset1 + 6, offset1 + 4, offset1 + 6, offset1 + 7
                };
                nh_core_appendToArray(&Indices2, indices_p, 12);
                offset1 += 8;

                // add color data
                ttyr_core_Color Color = ttyr_terminal_getGlyphColor(State_p, &Tile_p->Glyph, true, j, i, Grid_p);
                for (int v = 0; v < 8; ++v) {
                    nh_core_appendToArray(&Colors2, &Color.r, 1);
                    nh_core_appendToArray(&Colors2, &Color.g, 1);
                    nh_core_appendToArray(&Colors2, &Color.b, 1);
                }
            } else {
                nh_core_appendToArray(&Vertices, Tile_p->Foreground.vertices_p, 20);
                uint32_t indices_p[6] = {offset2, offset2 + 1, offset2 + 2, offset2, offset2 + 2, offset2 + 3};
                nh_core_appendToArray(&Indices, indices_p, 6);
                offset2 += 4;

                // add color data
                ttyr_core_Color Color = ttyr_terminal_getGlyphColor(State_p, &Tile_p->Glyph, true, j, i, Grid_p);
                for (int v = 0; v < 4; ++v) {
                    nh_core_appendToArray(&Colors, &Color.r, 1);
                    nh_core_appendToArray(&Colors, &Color.g, 1);
                    nh_core_appendToArray(&Colors, &Color.b, 1);
                }
            }
        }
    }

    Foreground_p->Vertices = Vertices;
    Foreground_p->Indices = Indices;
    Foreground_p->Vertices2 = Vertices2;
    Foreground_p->Indices2 = Indices2;
    Foreground_p->Colors = Colors;
    Foreground_p->Colors2 = Colors2;

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_updateBackgroundData(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_Grid *Grid_p, ttyr_terminal_GraphicsBackground *Background_p)
{
    nh_core_freeArray(&Background_p->Vertices);
    nh_core_freeArray(&Background_p->Colors);
    nh_core_freeArray(&Background_p->Indices);

    nh_core_Array Vertices = nh_core_initArray(sizeof(float), 2048);
    nh_core_Array Colors = nh_core_initArray(sizeof(float), 2048);
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

            // add color data
            ttyr_core_Color Color = ttyr_terminal_getGlyphColor(State_p, &Tile_p->Glyph, false, j, i, Grid_p);
            for (int v = 0; v < 4; ++v) {
                nh_core_appendToArray(&Colors, &Color.r, 1);
                nh_core_appendToArray(&Colors, &Color.g, 1);
                nh_core_appendToArray(&Colors, &Color.b, 1);
            }
        }
    }

    Background_p->Vertices = Vertices;
    Background_p->Colors = Colors;
    Background_p->Indices = Indices;

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_updateBoxesData(
    ttyr_terminal_Grid *Grid_p, ttyr_terminal_GraphicsBoxes *Boxes_p)
{
    nh_core_freeArray(&Boxes_p->Vertices);
    Boxes_p->Vertices = nh_core_initArray(sizeof(float), 64);
 
    for (int i = 0; i < Grid_p->Boxes.length; ++i) {
        nh_core_appendToArray(
            &Boxes_p->Vertices, ((ttyr_terminal_Box*)Grid_p->Boxes.p)[i].innerVertices_p, 18);
        nh_core_appendToArray(
            &Boxes_p->Vertices, ((ttyr_terminal_Box*)Grid_p->Boxes.p)[i].outerVertices_p, 18);
    }

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_updateDimData(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_Grid *Grid_p, ttyr_terminal_GraphicsDim *Dim_p)
{
    nh_core_freeArray(&Dim_p->Vertices);
    nh_core_freeArray(&Dim_p->Colors);

    nh_core_Array Vertices = nh_core_initArray(sizeof(float), 255);
    nh_core_Array Colors = nh_core_initArray(sizeof(float), 255);

    float z = 0.45f; // or whatever z-layer you want for dimming
    
    float vertices[] = {
        // x, y, z
        -1.0f, -1.0f, z,
         1.0f, -1.0f, z,
         1.0f,  1.0f, z,
        -1.0f, -1.0f, z,
         1.0f,  1.0f, z,
        -1.0f,  1.0f, z
    };

    float quadColors[24] = {
        0.0f, 0.0f, 0.0f, 0.7f,
        0.0f, 0.0f, 0.0f, 0.7f,
        0.0f, 0.0f, 0.0f, 0.7f,
    
        0.0f, 0.0f, 0.0f, 0.7f,
        0.0f, 0.0f, 0.0f, 0.7f,
        0.0f, 0.0f, 0.0f, 0.7f
    };

    for (int i = 0; i < 18; ++i) {
        nh_core_appendToArray(&Vertices, &vertices[i], 1);
    }

    for (int i = 0; i < 24; ++i) {
        nh_core_appendToArray(&Colors, &quadColors[i], 1);
    }

    Dim_p->Vertices = Vertices;
    Dim_p->Colors = Colors;

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_updateGraphicsData(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_GraphicsData *Data_p, ttyr_terminal_Grid *Grid_p)
{
    TTYR_TERMINAL_CHECK(ttyr_terminal_updateForegroundData(State_p, Grid_p, &Data_p->Foreground))
    TTYR_TERMINAL_CHECK(ttyr_terminal_updateBackgroundData(State_p, Grid_p, &Data_p->Background))
    TTYR_TERMINAL_CHECK(ttyr_terminal_updateBoxesData(Grid_p, &Data_p->Boxes))
    TTYR_TERMINAL_CHECK(ttyr_terminal_updateDimData(State_p, Grid_p, &Data_p->Dim))

    TTYR_TERMINAL_CHECK(ttyr_terminal_computeRange(State_p, Data_p, Grid_p, true))
    TTYR_TERMINAL_CHECK(ttyr_terminal_computeRange(State_p, Data_p, Grid_p, false))
    TTYR_TERMINAL_CHECK(ttyr_terminal_computeRangeForLineGraphics(Data_p, Grid_p))

    return TTYR_TERMINAL_SUCCESS;
}

bool ttyr_terminal_updateBlinkOrGradient(
    ttyr_terminal_GraphicsState *State_p)
{
    bool update = false;
    nh_core_SystemTime Time = nh_core_getSystemTime();

    ttyr_terminal_Config Config = ttyr_terminal_getConfig();
    if (nh_core_getSystemTimeDiffInSeconds(State_p->Blink.LastBlink, Time) >= Config.blinkFrequency) {
        update = true;
        State_p->Blink.LastBlink = Time;
        State_p->Blink.on = !State_p->Blink.on;
    }

    if (Config.accents > 1 && nh_core_getSystemTimeDiffInSeconds(State_p->AccentGradient.LastChange, Time) >= State_p->AccentGradient.interval) { 
        update = true; 
        State_p->AccentGradient.LastChange = Time; 
        if (State_p->AccentGradient.ratio >= 1.0f) {
            State_p->AccentGradient.index = State_p->AccentGradient.index == Config.accents-1 ? 0 : State_p->AccentGradient.index+1; 
            State_p->AccentGradient.ratio = 0.0f;
        }
        State_p->AccentGradient.Color = ttyr_terminal_getGradientColor(&State_p->AccentGradient, Config.Accents_p, Config.accents);
    } 

    if (Config.backgrounds > 1 && nh_core_getSystemTimeDiffInSeconds(State_p->BackgroundGradient.LastChange, Time) >= State_p->BackgroundGradient.interval) { 
        update = true; 
        State_p->BackgroundGradient.LastChange = Time; 
        if (State_p->BackgroundGradient.ratio >= 1.0f) {
            State_p->BackgroundGradient.index = State_p->BackgroundGradient.index == Config.backgrounds-1 ? 0 : State_p->BackgroundGradient.index+1; 
            State_p->BackgroundGradient.ratio = 0.0f;
        }
        State_p->BackgroundGradient.Color = ttyr_terminal_getGradientColor(&State_p->BackgroundGradient, Config.Backgrounds_p, Config.backgrounds);
    } 

    // Clear color needs to be updated.
    State_p->Viewport_p->Settings.ClearColor.r = State_p->BackgroundGradient.Color.r;
    State_p->Viewport_p->Settings.ClearColor.g = State_p->BackgroundGradient.Color.g;
    State_p->Viewport_p->Settings.ClearColor.b = State_p->BackgroundGradient.Color.b;

    return update;
}

// VIEWPORT ========================================================================================

TTYR_TERMINAL_RESULT ttyr_terminal_handleViewportChange(
    ttyr_terminal_Graphics *Graphics_p, nh_gfx_Viewport *Viewport_p)
{
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
                return TTYR_TERMINAL_ERROR_BAD_STATE;
        }
    }

    Viewport_p->Settings.ClearColor.r = Graphics_p->State.BackgroundGradient.Color.r;
    Viewport_p->Settings.ClearColor.g = Graphics_p->State.BackgroundGradient.Color.g;
    Viewport_p->Settings.ClearColor.b = Graphics_p->State.BackgroundGradient.Color.b;

    Graphics_p->State.Viewport_p = Viewport_p;

    return TTYR_TERMINAL_SUCCESS;
}

// RENDER ==========================================================================================

TTYR_TERMINAL_RESULT ttyr_terminal_renderGraphics(
    ttyr_terminal_Graphics *Graphics_p, ttyr_terminal_Grid *Grid_p, ttyr_terminal_Grid *Grid2_p, 
    ttyr_terminal_Grid *BorderGrid_p)
{
    switch (Graphics_p->State.Viewport_p->Surface_p->api)
    {
        case NH_API_GRAPHICS_BACKEND_VULKAN :
//            TTYR_TERMINAL_CHECK(ttyr_terminal_renderUsingVulkan(Graphics_p))
            break;
       case NH_API_GRAPHICS_BACKEND_OPENGL :
            TTYR_TERMINAL_CHECK(ttyr_terminal_renderUsingOpenGL(Graphics_p, Grid_p, Grid2_p, BorderGrid_p))
            break;
        default :
            return TTYR_TERMINAL_ERROR_BAD_STATE;
    }

    return TTYR_TERMINAL_SUCCESS;
}
