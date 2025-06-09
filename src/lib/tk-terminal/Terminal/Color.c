// LICENSE NOTICE ==================================================================================

/**
 * Termoskanne - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

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

// FUNCTIONS =======================================================================================

static tk_core_Color tk_terminal_colorFromRGB(
    int r, int g, int b)
{
    tk_core_Color Color;
    Color.r = (float)r/(float)255;
    Color.g = (float)g/(float)255;
    Color.b = (float)b/(float)255;
    return Color; 
}

static tk_core_Color tk_terminal_blend(
    tk_core_Color a, tk_core_Color b, float t)
{
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    tk_core_Color result;
    result.r = a.r + (b.r - a.r) * t;
    result.g = a.g + (b.g - a.g) * t;
    result.b = a.b + (b.b - a.b) * t;
    return result;
}

// Pseudo-random function based on col, row and time
static float randf(int x, int y, float time) {
    return fmodf(sinf(x * 12.9898f + y * 78.233f + time * 43758.5453f) * 43758.5453f, 1.0f);
}

static tk_core_Color tk_terminal_getStrobeColor(
    int col, int row, float time, tk_core_Color base)
{
    float flicker = randf(col, row, floorf(time * 15.0f));  // Change every 1/15th second
    tk_core_Color random_color;
    float min_brightness = 0.5f;
    random_color.r = min_brightness + randf(col, row, time + 0.1f) * (1.0f - min_brightness);
    random_color.g = min_brightness + randf(col, row, time + 0.5f) * (1.0f - min_brightness);
    random_color.b = min_brightness + randf(col, row, time + 1.0f) * (1.0f - min_brightness);
    return random_color;
}

static tk_core_Color tk_terminal_getWaveColor(
    int col, int row, float time, int total_cols, int total_rows, tk_core_Color base)
{
    float wave_length = 200.0f;
    float freq_x = (2.0f * M_PI) / wave_length;
    float aspect_ratio = (float)total_rows / (float)total_cols;
    float freq_y = freq_x / aspect_ratio;
    float speed = 5.0f;

    float t = (sinf(col * freq_x + row * freq_y + time * speed) + 1.0f) / 2.0f;

    // Inverted color
    tk_core_Color inverted;
    inverted.r = 1.0f - base.r;
    inverted.g = 1.0f - base.g;
    inverted.b = 1.0f - base.b;

    // Interpolate between base and inverted
    tk_core_Color result;
    result.r = base.r + (inverted.r - base.r) * t;
    result.g = base.g + (inverted.g - base.g) * t;
    result.b = base.b + (inverted.b - base.b) * t;

    return result;
}

static tk_core_Color tk_terminal_getRainbowColor(
    int col, int row, float time) 
{
    float speed = 2.0f;
    float hue = fmodf((float)col * 0.1f + time * speed, 1.0f);
    tk_core_Color color;
    color.r = fabsf(hue - 0.5f) * 2.0f;
    color.g = fabsf(fmodf(hue + 0.33f, 1.0f) - 0.5f) * 2.0f;
    color.b = fabsf(fmodf(hue + 0.66f, 1.0f) - 0.5f) * 2.0f;
    return color;
}

static tk_core_Color tk_terminal_getBubbleColor(
    int col, int row, float time) 
{
    float shift = (col + row) * 0.1f + time;
    tk_core_Color color;
    color.r = (sinf(shift) + 1.0f) * 0.4f + 0.2f;
    color.g = (sinf(shift + 2.0f) + 1.0f) * 0.4f + 0.2f;
    color.b = (sinf(shift + 4.0f) + 1.0f) * 0.4f + 0.2f;
    return color;
}

static tk_core_Color tk_terminal_getPastelCheckerColor(
    int col, int row, tk_core_Color base)
{
    int alt = (col / 2 + row / 2) % 2;
    if (alt)
        return tk_terminal_colorFromRGB(255, 235, 205); // Light peach
    else
        return tk_terminal_colorFromRGB(110, 145, 155); // Pale blue
}

static tk_core_Color tk_terminal_getOceanColor(
    int col, int row, int total_cols, int total_rows, tk_core_Color base)
{
    float depth = (float)row / total_rows;
    tk_core_Color shallow = tk_terminal_colorFromRGB(102, 255, 255);  // Light cyan
    tk_core_Color deep = tk_terminal_colorFromRGB(0, 0, 128);         // Navy blue
    return tk_terminal_blend(shallow, deep, depth);
}

static tk_core_Color tk_terminal_getSunsetColor(
    int col, int row, int total_cols, int total_rows, tk_core_Color base)
{
    float y = (float)row / total_rows;
    float t = y * y * (3.0f - 2.0f * y);  // Smoothstep
    tk_core_Color sunGlow = tk_terminal_colorFromRGB(255, 204, 92);   // Yellow/orange near sun
    tk_core_Color horizonPink = tk_terminal_colorFromRGB(255, 105, 180);  // Pink
    tk_core_Color twilight = tk_terminal_colorFromRGB(128, 0, 128);    // Purple
    tk_core_Color nightSky = tk_terminal_colorFromRGB(25, 25, 112);    // Midnight blue
    tk_core_Color lowBlend = tk_terminal_blend(sunGlow, horizonPink, t);
    tk_core_Color highBlend = tk_terminal_blend(twilight, nightSky, t);
    tk_core_Color result = tk_terminal_blend(lowBlend, highBlend, t * 0.8f + 0.2f);
    return result;
}

static tk_core_Color tk_terminal_getAccentColor(
    tk_terminal_Config *Config_p, int col, int row, int total_cols, int total_rows, tk_core_Color base)
{
//  float time_now = (float)clock() / CLOCKS_PER_SEC;
    float time_now = 0;
    switch (Config_p->style) {
        case 1 : return tk_terminal_getWaveColor(col, row, time_now, total_cols, total_rows, base);
        case 2 : return tk_terminal_getStrobeColor(col, row, time_now, base);
        case 3 : return tk_terminal_getRainbowColor(col, row, time_now);
        case 4 : return tk_terminal_getBubbleColor(col, row, time_now);
        case 5 : return tk_terminal_getPastelCheckerColor(col, row, base);
        case 6 : return tk_terminal_getOceanColor(col, row, total_cols, total_rows, base);
        case 7 : return tk_terminal_getSunsetColor(col, row, total_cols, total_rows, base);
    }
    return base;
}

tk_core_Color tk_terminal_getGlyphColor(
    tk_terminal_Config *Config_p, tk_terminal_GraphicsState *State_p, tk_core_Glyph *Glyph_p, bool foreground, int col, int row, tk_terminal_Grid *Grid_p)
{
    if (foreground) {
        if (Glyph_p->Attributes.reverse || (Glyph_p->Attributes.blink && State_p->Blink.on)) {
            if (Glyph_p->Background.custom) {
                return Glyph_p->Background.Color;
            }
            if (Glyph_p->mark & TK_CORE_MARK_ACCENT) {
                tk_core_Color Color = tk_terminal_getAccentColor(Config_p, col, row, Grid_p->cols, Grid_p->rows, State_p->AccentGradient.Color);
                Color.r *= 0.3f;
                Color.g *= 0.3f;
                Color.b *= 0.3f;
                return Color;
            }
            return State_p->BackgroundGradient.Color;
        }
        if (Glyph_p->mark & TK_CORE_MARK_ACCENT) {
            return tk_terminal_getAccentColor(Config_p, col, row, Grid_p->cols, Grid_p->rows, State_p->AccentGradient.Color);
        }
        if (Glyph_p->Foreground.custom) {
            return Glyph_p->Foreground.Color;
        }
        return Config_p->Foreground;
    }

    // Background.
    if ((Glyph_p->Attributes.reverse && !(Glyph_p->Attributes.blink && State_p->Blink.on)) 
    || (!Glyph_p->Attributes.reverse &&   Glyph_p->Attributes.blink && State_p->Blink.on)) {
        if (Glyph_p->mark & TK_CORE_MARK_ACCENT) {
            return tk_terminal_getAccentColor(Config_p, col, row, Grid_p->cols, Grid_p->rows, State_p->AccentGradient.Color);
        }
        if (Glyph_p->Foreground.custom) {
            return Glyph_p->Foreground.Color;
        }
        return Config_p->Foreground;
    }
    if (Glyph_p->Background.custom) {
        return Glyph_p->Background.Color;
    }

    return State_p->BackgroundGradient.Color;
}

tk_core_Color tk_terminal_getGradientColor(
    tk_terminal_GraphicsGradient *Gradient_p, tk_core_Color *Colors_p, int colors)
{
    if (colors == 1) {
        return Colors_p[0];
    }

    tk_core_Color Color1 = Colors_p[Gradient_p->index];
    tk_core_Color Color2 = Gradient_p->index == colors-1 ? Colors_p[0] : Colors_p[Gradient_p->index+1];

    tk_core_Color Result;
    Result.r = Color1.r + Gradient_p->ratio * (Color2.r - Color1.r);
    Result.g = Color1.g + Gradient_p->ratio * (Color2.g - Color1.g);
    Result.b = Color1.b + Gradient_p->ratio * (Color2.b - Color1.b);
    Result.a = 1.0f;

    Gradient_p->ratio += 0.01f;

    return Result;
}
