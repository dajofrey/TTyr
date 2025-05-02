// LICENSE NOTICE ==================================================================================

/**
 * Termoskanne - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See LICENSE.LGPL file.
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

static TK_TERMINAL_RESULT tk_terminal_getInternalMonospaceFonts(
    nh_core_List *Fonts_p)
{
    nh_gfx_FontStyle Style;
    nh_gfx_parseFontStyle(&Style, "normal");

    nh_gfx_FontFamily Family = nh_gfx_initFontFamily(NULL);
    Family.generic_p[NH_GFX_GENERIC_FONT_FAMILY_MONOSPACE] = true;

    *Fonts_p = nh_gfx_getFonts(Family, Style, true);
    if (Fonts_p->size <= 0) {return TK_TERMINAL_ERROR_BAD_STATE;}

    nh_gfx_freeFontStyle(&Style);

    return TK_TERMINAL_SUCCESS;
}

static tk_terminal_GraphicsAction tk_terminal_initGraphicsAction()
{
    tk_terminal_GraphicsAction Action;
    Action.init = true;

    return Action;
}

// INIT/FREE =======================================================================================

static TK_TERMINAL_RESULT tk_terminal_initGraphicsData(
    tk_terminal_GraphicsData *Data_p)
{
    Data_p->Foreground.Action = tk_terminal_initGraphicsAction();
    Data_p->Foreground.Vertices = nh_core_initArray(sizeof(float), 1024);
    Data_p->Foreground.Indices  = nh_core_initArray(sizeof(uint32_t), 1024);
    Data_p->Foreground.Vertices2 = nh_core_initArray(sizeof(float), 1024);
    Data_p->Foreground.Indices2  = nh_core_initArray(sizeof(uint32_t), 1024);
    Data_p->Foreground.Ranges = nh_core_initArray(sizeof(tk_terminal_AttributeRange), 32);
    Data_p->Foreground.Ranges2 = nh_core_initArray(sizeof(tk_terminal_AttributeRange), 32);
    Data_p->Foreground.Colors = nh_core_initArray(sizeof(float), 2048);
    Data_p->Foreground.Colors2 = nh_core_initArray(sizeof(float), 2048);

    tk_terminal_initOpenGLForeground(&Data_p->Foreground.OpenGL);

    Data_p->Background.Action = tk_terminal_initGraphicsAction();
    Data_p->Background.Vertices = nh_core_initArray(sizeof(float), 1024);
    Data_p->Background.Colors = nh_core_initArray(sizeof(float), 2048);
    Data_p->Background.Indices  = nh_core_initArray(sizeof(uint32_t), 1024);
    Data_p->Background.Ranges = nh_core_initArray(sizeof(tk_terminal_AttributeRange), 32);

    tk_terminal_initOpenGLBackground(&Data_p->Background.OpenGL);

    return TK_TERMINAL_SUCCESS;
}

static TK_TERMINAL_RESULT tk_terminal_initGraphicsState(
    tk_terminal_Config *Config_p, tk_terminal_GraphicsState *State_p)
{
    memset(State_p, 0, sizeof(tk_terminal_GraphicsState));

    TK_TERMINAL_CHECK(tk_terminal_getInternalMonospaceFonts(&State_p->Fonts))

    State_p->Map = nh_core_createHashMap();
    State_p->Glyphs = nh_core_initList(128);
    State_p->Codepoints = nh_core_initList(128);

    State_p->AccentGradient.Color = tk_terminal_getGradientColor(&State_p->AccentGradient, Config_p->Accents_p, Config_p->accents);
    State_p->AccentGradient.interval = 0.1;
    State_p->AccentGradient.LastChange = nh_core_getSystemTime();

    State_p->BackgroundGradient = State_p->AccentGradient;
    State_p->BackgroundGradient.Color = tk_terminal_getGradientColor(&State_p->BackgroundGradient, Config_p->Backgrounds_p, Config_p->backgrounds);

    State_p->Blink.LastBlink = nh_core_getSystemTime();
 
    return TK_TERMINAL_SUCCESS;
}

TK_TERMINAL_RESULT tk_terminal_initGraphics(
    tk_terminal_Config *Config_p, tk_terminal_Graphics *Graphics_p)
{
    memset(Graphics_p, 0, sizeof(tk_terminal_Graphics));

    TK_TERMINAL_CHECK(tk_terminal_initGraphicsState(Config_p, &Graphics_p->State))
    TK_TERMINAL_CHECK(tk_terminal_initGraphicsData(&Graphics_p->MainData))
    TK_TERMINAL_CHECK(tk_terminal_initGraphicsData(&Graphics_p->ElevatedData))
    TK_TERMINAL_CHECK(tk_terminal_initGraphicsData(&Graphics_p->BackdropData))
 
    Graphics_p->Dim.Action = tk_terminal_initGraphicsAction();
    Graphics_p->Dim.Vertices = nh_core_initArray(sizeof(float), 255);
    Graphics_p->Dim.Colors = nh_core_initArray(sizeof(float), 255);
    tk_terminal_initOpenGLDim(&Graphics_p->Dim.OpenGL);

    Graphics_p->Boxes.Action = tk_terminal_initGraphicsAction();
    Graphics_p->Boxes.Data = nh_core_initArray(sizeof(tk_terminal_Box), 16);

    tk_terminal_initOpenGLBoxes(&Graphics_p->Boxes.OpenGL);
 
    return TK_TERMINAL_SUCCESS;
}

static TK_TERMINAL_RESULT tk_terminal_freeGraphicsData(
    tk_terminal_GraphicsData *Data_p)
{
    nh_core_freeArray(&Data_p->Foreground.Vertices);
    nh_core_freeArray(&Data_p->Foreground.Indices);
    nh_core_freeArray(&Data_p->Foreground.Ranges);
    nh_core_freeArray(&Data_p->Foreground.Vertices2);
    nh_core_freeArray(&Data_p->Foreground.Indices2);
    nh_core_freeArray(&Data_p->Foreground.Ranges2);
    nh_core_freeArray(&Data_p->Foreground.Colors);
    nh_core_freeArray(&Data_p->Foreground.Colors2);

    tk_terminal_freeOpenGLForeground(&Data_p->Foreground.OpenGL);

    nh_core_freeArray(&Data_p->Background.Vertices);
    nh_core_freeArray(&Data_p->Background.Colors);
    nh_core_freeArray(&Data_p->Background.Indices);
    nh_core_freeArray(&Data_p->Background.Ranges);
 
    tk_terminal_freeOpenGLBackground(&Data_p->Background.OpenGL);

    return TK_TERMINAL_SUCCESS;
}

TK_TERMINAL_RESULT tk_terminal_freeGraphics(
    tk_terminal_Graphics *Graphics_p)
{
    TK_TERMINAL_CHECK(tk_terminal_freeGraphicsData(&Graphics_p->MainData))
    TK_TERMINAL_CHECK(tk_terminal_freeGraphicsData(&Graphics_p->ElevatedData))
    TK_TERMINAL_CHECK(tk_terminal_freeGraphicsData(&Graphics_p->BackdropData))

    nh_core_freeArray(&Graphics_p->Dim.Vertices);
    nh_core_freeArray(&Graphics_p->Dim.Colors);
    nh_core_freeArray(&Graphics_p->Boxes.Data);

    tk_terminal_freeOpenGLBoxes(&Graphics_p->Boxes.OpenGL);

    nh_core_freeList(&Graphics_p->State.Fonts, false);
    nh_core_freeList(&Graphics_p->State.Glyphs, true);
    nh_core_freeList(&Graphics_p->State.Codepoints, true);

    nh_core_freeHashMap(Graphics_p->State.Map);

    return TK_TERMINAL_SUCCESS;
}

// RANGES ==========================================================================================

static int tk_terminal_getCurrentAttributeRangeForLineGraphics(
    tk_terminal_Grid *Grid_p, tk_core_Glyph *Current_p, int *col_p, int *row_p)
{
    int total = 0;

    for (int row = *row_p; row < Grid_p->rows; ++row) {
        for (int col = *col_p; col < Grid_p->cols; ++col) {
            tk_core_Glyph Glyph = ((tk_terminal_Tile*)((nh_core_List*)Grid_p->Rows.pp[row])->pp[col])->Glyph;
            if (!(Glyph.mark & TK_CORE_MARK_LINE_GRAPHICS)) {
                continue;
            }
            if (tk_terminal_compareForegroundAttributes(Current_p, &Glyph)) {
                *col_p = col;
                *row_p = row;
                *Current_p = Glyph;
                return total;
            }
            ++total;
        }
        *col_p = 0;
    }

    *col_p = Grid_p->cols;
    *row_p = Grid_p->rows;

    return total;
}

static int tk_terminal_getCurrentAttributeRange(
    tk_terminal_GraphicsState *State_p, tk_terminal_Grid *Grid_p, tk_core_Glyph *Current_p, int *col_p, 
    int *row_p, bool foreground)
{
    int total = 0;

    for (int row = *row_p; row < Grid_p->rows; ++row) {
        for (int col = *col_p; col < Grid_p->cols; ++col) {
            tk_core_Glyph Glyph = ((tk_terminal_Tile*)((nh_core_List*)Grid_p->Rows.pp[row])->pp[col])->Glyph;
            if (foreground && (!Glyph.codepoint || Glyph.codepoint == ' ' || Glyph.mark & TK_CORE_MARK_LINE_GRAPHICS)) {
                continue;
            }
            if (!foreground && (!Glyph.Background.custom && !Glyph.Attributes.reverse && !Glyph.Attributes.blink)) {
                continue;
            }
            if (!foreground && Glyph.Attributes.blink && !State_p->Blink.on) {
                continue;
            }
            if ((foreground && tk_terminal_compareForegroundAttributes(Current_p, &Glyph))
            || (!foreground && tk_terminal_compareBackgroundAttributes(Current_p, &Glyph))) {
                *col_p = col;
                *row_p = row;
                *Current_p = Glyph;
                return total;
            }
            ++total;
        }
        *col_p = 0;
    }

    *col_p = Grid_p->cols;
    *row_p = Grid_p->rows;

    return total;
}

static TK_TERMINAL_RESULT tk_terminal_computeRangeForLineGraphics(
    tk_terminal_GraphicsData *Data_p, tk_terminal_Grid *Grid_p)
{
    nh_core_freeArray(&Data_p->Foreground.Ranges2);
    Data_p->Foreground.Ranges2 = nh_core_initArray(sizeof(tk_terminal_AttributeRange), 64);

    if (Grid_p->rows <= 0 || Grid_p->Rows.size == 0) {
        return TK_TERMINAL_SUCCESS;
    }

    int total = 0;
    int row = 0;
    int col = 0;

    tk_core_Glyph Glyph = ((tk_terminal_Tile*)((nh_core_List*)Grid_p->Rows.pp[0])->pp[0])->Glyph;
    bool begin = true;

    while (true)
    {
        tk_core_Glyph NextGlyph = Glyph;
        total = tk_terminal_getCurrentAttributeRangeForLineGraphics(Grid_p, &NextGlyph, &col, &row) * 12;
        if (!total && !begin) {break;}
        begin = false;

        tk_terminal_AttributeRange *Range_p = 
            (tk_terminal_AttributeRange*)nh_core_incrementArray(&Data_p->Foreground.Ranges2);

        Range_p->Glyph = Glyph;
        Range_p->indices = total;

        Glyph = NextGlyph;
    }

    return TK_TERMINAL_SUCCESS;
}

static TK_TERMINAL_RESULT tk_terminal_computeRange(
    tk_terminal_GraphicsState *State_p, tk_terminal_GraphicsData *Data_p, tk_terminal_Grid *Grid_p, 
    bool foreground)
{
    if (foreground) {
        nh_core_freeArray(&Data_p->Foreground.Ranges);
        Data_p->Foreground.Ranges = nh_core_initArray(sizeof(tk_terminal_AttributeRange), 32);
    } else {
        nh_core_freeArray(&Data_p->Background.Ranges);
        Data_p->Background.Ranges = nh_core_initArray(sizeof(tk_terminal_AttributeRange), 32);
    }

    if (Grid_p->rows <= 0 || Grid_p->Rows.size == 0) {
        return TK_TERMINAL_SUCCESS;
    }

    int total = 0;
    int row = 0;
    int col = 0;

    tk_core_Glyph Glyph = ((tk_terminal_Tile*)((nh_core_List*)Grid_p->Rows.pp[0])->pp[0])->Glyph;
    bool begin = true;

    // Default Foreground/Background.
    while (true)
    {
        tk_core_Glyph NextGlyph = Glyph;
 
        total = tk_terminal_getCurrentAttributeRange(State_p, Grid_p, &NextGlyph, &col, &row, foreground) * 6;
        if (!total && !begin) {break;}
        begin = false;

        tk_terminal_AttributeRange *Range_p = (tk_terminal_AttributeRange*)nh_core_incrementArray(
            foreground ? &Data_p->Foreground.Ranges : &Data_p->Background.Ranges);

        Range_p->Glyph = Glyph;
        Range_p->indices = total;

        Glyph = NextGlyph;
    }

    return TK_TERMINAL_SUCCESS;
}

// UPDATE ==========================================================================================

static TK_TERMINAL_RESULT tk_terminal_updateForegroundData(
    tk_terminal_Config *Config_p, tk_terminal_GraphicsState *State_p, tk_terminal_Grid *Grid_p,
    tk_terminal_GraphicsForeground *Foreground_p, int shift)
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
            tk_terminal_Tile *Tile_p = Row_p->pp[j];
            if (!Tile_p || !Tile_p->Glyph.codepoint || Tile_p->Glyph.codepoint == ' ') {continue;}

            if (Tile_p->Glyph.mark & TK_CORE_MARK_LINE_GRAPHICS) {
                nh_core_appendToArray(&Vertices2, Tile_p->Foreground.vertices_p, 24);
                uint32_t indices_p[12] = {
                    offset1, offset1 + 1, offset1 + 2, offset1, offset1 + 2, offset1 + 3,
                    offset1 + 4, offset1 + 5, offset1 + 6, offset1 + 4, offset1 + 6, offset1 + 7
                };
                nh_core_appendToArray(&Indices2, indices_p, 12);
                offset1 += 8;

                // add color data
                tk_core_Color Color = tk_terminal_getGlyphColor(Config_p, State_p, &Tile_p->Glyph, true, j+shift, i, Grid_p);
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
                tk_core_Color Color = tk_terminal_getGlyphColor(Config_p, State_p, &Tile_p->Glyph, true, j+shift, i, Grid_p);
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

    return TK_TERMINAL_SUCCESS;
}

static TK_TERMINAL_RESULT tk_terminal_updateBackgroundData(
    tk_terminal_Config *Config_p, tk_terminal_GraphicsState *State_p, tk_terminal_Grid *Grid_p,
    tk_terminal_GraphicsBackground *Background_p, int shift)
{
    nh_core_freeArray(&Background_p->Vertices);
    nh_core_freeArray(&Background_p->Colors);
    nh_core_freeArray(&Background_p->Indices);

    Background_p->Vertices = nh_core_initArray(sizeof(float), 2048);
    Background_p->Colors = nh_core_initArray(sizeof(float), 2048);
    Background_p->Indices = nh_core_initArray(sizeof(uint32_t), 1024);

    int offset = 0;

    for (int i = 0; i < Grid_p->Rows.size; ++i) {
        nh_core_List *Row_p = Grid_p->Rows.pp[i];
        for (int j = 0; j < Row_p->size; ++j) {
            tk_terminal_Tile *Tile_p = Row_p->pp[j];
            if (!Tile_p->Glyph.Background.custom && !Tile_p->Glyph.Attributes.reverse && !Tile_p->Glyph.Attributes.blink) {continue;}

            nh_core_appendToArray(&Background_p->Vertices, Tile_p->Background.vertices_p, 12);
            uint32_t indices_p[6] = {offset, offset + 1, offset + 2, offset, offset + 2, offset + 3};
            nh_core_appendToArray(&Background_p->Indices, indices_p, 6);
            offset += 4;

            // add color data
            tk_core_Color Color = tk_terminal_getGlyphColor(Config_p, State_p, &Tile_p->Glyph, false, j+shift, i, Grid_p);
            for (int v = 0; v < 4; ++v) {
                nh_core_appendToArray(&Background_p->Colors, &Color.r, 1);
                nh_core_appendToArray(&Background_p->Colors, &Color.g, 1);
                nh_core_appendToArray(&Background_p->Colors, &Color.b, 1);
            }
        }
    }

    return TK_TERMINAL_SUCCESS;
}

static TK_TERMINAL_RESULT tk_terminal_updateBoxesData(
    tk_terminal_Graphics *Graphics_p, tk_terminal_Config *Config_p, tk_terminal_Grid *Grid_p)
{
    nh_core_freeArray(&Graphics_p->Boxes.Vertices);
    nh_core_freeArray(&Graphics_p->Boxes.Colors);

    Graphics_p->Boxes.Vertices = nh_core_initArray(sizeof(float), 64);
    Graphics_p->Boxes.Colors = nh_core_initArray(sizeof(float), 64);
 
    for (int i = 0; i < Graphics_p->Boxes.Data.length; ++i) {
        tk_terminal_Box *Box_p = ((tk_terminal_Box*)Graphics_p->Boxes.Data.p)+i;
        if (Box_p->UpperLeft.x < 0 || Box_p->UpperLeft.y < 0 || Box_p->LowerRight.x < 0 || Box_p->LowerRight.y < 0) {continue;}
        nh_core_appendToArray(&Graphics_p->Boxes.Vertices, Box_p->innerVertices_p, 18);
        nh_core_appendToArray(&Graphics_p->Boxes.Vertices, Box_p->outerVertices_p, 18);
        // add color data
        tk_terminal_Tile *Tile_p = tk_terminal_getTile(Grid_p, Box_p->UpperLeft.y, Box_p->UpperLeft.x);
        Tile_p->Glyph.Attributes.reverse = true;
        Tile_p->Glyph.mark |= TK_CORE_MARK_ACCENT;
        tk_core_Color Color = tk_terminal_getGlyphColor(Config_p, &Graphics_p->State, &Tile_p->Glyph, false, Box_p->UpperLeft.x+2, Box_p->UpperLeft.y+1, Grid_p);
        Tile_p->Glyph.Attributes.reverse = false;
        Tile_p->Glyph.mark &= TK_CORE_MARK_ACCENT;
        for (int v = 0; v < 12; ++v) {
            nh_core_appendToArray(&Graphics_p->Boxes.Colors, &Color.r, 1);
            nh_core_appendToArray(&Graphics_p->Boxes.Colors, &Color.g, 1);
            nh_core_appendToArray(&Graphics_p->Boxes.Colors, &Color.b, 1);
        }
    }

    return TK_TERMINAL_SUCCESS;
}

static TK_TERMINAL_RESULT tk_terminal_updateDimData(
    tk_terminal_GraphicsState *State_p, tk_terminal_Grid *Grid_p, tk_terminal_Dim *Dim_p)
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

    return TK_TERMINAL_SUCCESS;
}

static TK_TERMINAL_RESULT tk_terminal_updateGridGraphics(
    tk_terminal_Config *Config_p, tk_terminal_GraphicsState *State_p, tk_terminal_GraphicsData *Data_p,
    tk_terminal_Grid *Grid_p, int offset)
{
    TK_TERMINAL_CHECK(tk_terminal_updateForegroundData(Config_p, State_p, Grid_p, &Data_p->Foreground, offset))
    TK_TERMINAL_CHECK(tk_terminal_updateBackgroundData(Config_p, State_p, Grid_p, &Data_p->Background, offset))

    TK_TERMINAL_CHECK(tk_terminal_computeRange(State_p, Data_p, Grid_p, true))
    TK_TERMINAL_CHECK(tk_terminal_computeRange(State_p, Data_p, Grid_p, false))
    TK_TERMINAL_CHECK(tk_terminal_computeRangeForLineGraphics(Data_p, Grid_p))

    return TK_TERMINAL_SUCCESS;
}

TK_TERMINAL_RESULT tk_terminal_updateGraphics(
    tk_terminal_Config *Config_p, tk_terminal_Graphics *Graphics_p, tk_terminal_Grid *Grid_p,
    tk_terminal_Grid *BackdropGrid_p, tk_terminal_Grid *ElevatedGrid_p, bool titlebarOn)
{
    int shift = titlebarOn ? 1 : 3;

    TK_TERMINAL_CHECK(tk_terminal_updateGridGraphics( 
        Config_p, &Graphics_p->State, &Graphics_p->MainData, Grid_p, shift))
    TK_TERMINAL_CHECK(tk_terminal_updateGridGraphics( 
        Config_p, &Graphics_p->State, &Graphics_p->ElevatedData, ElevatedGrid_p, shift))
    TK_TERMINAL_CHECK(tk_terminal_updateGridGraphics( 
        Config_p, &Graphics_p->State, &Graphics_p->BackdropData, BackdropGrid_p, 0)) 

    TK_TERMINAL_CHECK(tk_terminal_updateDimData(&Graphics_p->State, Grid_p, &Graphics_p->Dim))
    TK_TERMINAL_CHECK(tk_terminal_updateBoxesData(Graphics_p, Config_p, Grid_p))

    return TK_TERMINAL_SUCCESS;
}
 
bool tk_terminal_updateBlinkOrGradient(
    tk_terminal_Config *Config_p, tk_terminal_GraphicsState *State_p)
{
    bool update = false;
    nh_core_SystemTime Time = nh_core_getSystemTime();

    if (nh_core_getSystemTimeDiffInSeconds(State_p->Blink.LastBlink, Time) >= Config_p->blinkFrequency) {
        update = true;
        State_p->Blink.LastBlink = Time;
        State_p->Blink.on = !State_p->Blink.on;
    }

    if (Config_p->accents > 1 && nh_core_getSystemTimeDiffInSeconds(State_p->AccentGradient.LastChange, Time) >= State_p->AccentGradient.interval) { 
        update = true; 
        State_p->AccentGradient.LastChange = Time; 
        if (State_p->AccentGradient.ratio >= 1.0f) {
            State_p->AccentGradient.index = State_p->AccentGradient.index == Config_p->accents-1 ? 0 : State_p->AccentGradient.index+1; 
            State_p->AccentGradient.ratio = 0.0f;
        }
        State_p->AccentGradient.Color = tk_terminal_getGradientColor(&State_p->AccentGradient, Config_p->Accents_p, Config_p->accents);
    } 

    if (Config_p->backgrounds > 1 && nh_core_getSystemTimeDiffInSeconds(State_p->BackgroundGradient.LastChange, Time) >= State_p->BackgroundGradient.interval) {
        update = true; 
        State_p->BackgroundGradient.LastChange = Time; 
        if (State_p->BackgroundGradient.ratio >= 1.0f) {
            State_p->BackgroundGradient.index = State_p->BackgroundGradient.index == Config_p->backgrounds-1 ? 0 : State_p->BackgroundGradient.index+1; 
            State_p->BackgroundGradient.ratio = 0.0f;
        }
        State_p->BackgroundGradient.Color = tk_terminal_getGradientColor(&State_p->BackgroundGradient, Config_p->Backgrounds_p, Config_p->backgrounds);
    } 

    // Clear color needs to be updated.
    State_p->Viewport_p->Settings.ClearColor.r = State_p->BackgroundGradient.Color.r;
    State_p->Viewport_p->Settings.ClearColor.g = State_p->BackgroundGradient.Color.g;
    State_p->Viewport_p->Settings.ClearColor.b = State_p->BackgroundGradient.Color.b;

    return update;
}

// VIEWPORT ========================================================================================

TK_TERMINAL_RESULT tk_terminal_handleViewportChange(
    tk_terminal_Graphics *Graphics_p, nh_gfx_Viewport *Viewport_p)
{
    // Check if it's the initial call.
    if (!Graphics_p->State.Viewport_p) 
    {
        switch (Viewport_p->Surface_p->api)
        {
            case NH_API_GRAPHICS_BACKEND_VULKAN :
//                tk_terminal_initVulkanText(Viewport_p->Surface_p->Vulkan.GPU_p, 
//                    &Graphics_p->Foreground.Vulkan);
                break;
            case NH_API_GRAPHICS_BACKEND_OPENGL :
                break;
            default :
                return TK_TERMINAL_ERROR_BAD_STATE;
        }
    }

    Viewport_p->Settings.ClearColor.r = Graphics_p->State.BackgroundGradient.Color.r;
    Viewport_p->Settings.ClearColor.g = Graphics_p->State.BackgroundGradient.Color.g;
    Viewport_p->Settings.ClearColor.b = Graphics_p->State.BackgroundGradient.Color.b;

    Graphics_p->State.Viewport_p = Viewport_p;

    return TK_TERMINAL_SUCCESS;
}

// RENDER ==========================================================================================

TK_TERMINAL_RESULT tk_terminal_renderGraphics(
    tk_terminal_Config *Config_p, tk_terminal_Graphics *Graphics_p, tk_terminal_Grid *Grid_p,
    tk_terminal_Grid *ElevatedGrid_p, tk_terminal_Grid *BackdropGrid_p)
{
    switch (Graphics_p->State.Viewport_p->Surface_p->api)
    {
        case NH_API_GRAPHICS_BACKEND_VULKAN :
//            TK_TERMINAL_CHECK(tk_terminal_renderUsingVulkan(Graphics_p))
            break;
       case NH_API_GRAPHICS_BACKEND_OPENGL :
            TK_TERMINAL_CHECK(tk_terminal_renderUsingOpenGL(Config_p, Graphics_p, Grid_p, BackdropGrid_p))
            break;
        default :
            return TK_TERMINAL_ERROR_BAD_STATE;
    }

    return TK_TERMINAL_SUCCESS;
}
