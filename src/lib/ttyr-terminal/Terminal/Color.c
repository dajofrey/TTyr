// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
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

// Pseudo-random function based on col, row and time
static float randf(int x, int y, float time) {
    return fmodf(sinf(x * 12.9898f + y * 78.233f + time * 43758.5453f) * 43758.5453f, 1.0f);
}

static ttyr_core_Color get_strobe_color(int col, int row, float time, ttyr_core_Color base) {
    float flicker = randf(col, row, floorf(time * 15.0f));  // Change every 1/15th second
    ttyr_core_Color random_color;
    float min_brightness = 0.5f;
    random_color.r = min_brightness + randf(col, row, time + 0.1f) * (1.0f - min_brightness);
    random_color.g = min_brightness + randf(col, row, time + 0.5f) * (1.0f - min_brightness);
    random_color.b = min_brightness + randf(col, row, time + 1.0f) * (1.0f - min_brightness);
    return random_color;
}

static ttyr_core_Color get_wave_color(int col, int row, float time, int total_cols, int total_rows, ttyr_core_Color base) {
    float wave_length = 200.0f;
    float freq_x = (2.0f * M_PI) / wave_length;
    float aspect_ratio = (float)total_rows / (float)total_cols;
    float freq_y = freq_x / aspect_ratio;
    float speed = 5.0f;

    float t = (sinf(col * freq_x + row * freq_y + time * speed) + 1.0f) / 2.0f;

    // Inverted color
    ttyr_core_Color inverted;
    inverted.r = 1.0f - base.r;
    inverted.g = 1.0f - base.g;
    inverted.b = 1.0f - base.b;

    // Interpolate between base and inverted
    ttyr_core_Color result;
    result.r = base.r + (inverted.r - base.r) * t;
    result.g = base.g + (inverted.g - base.g) * t;
    result.b = base.b + (inverted.b - base.b) * t;

    return result;
}

static ttyr_core_Color get_rainbow_color(int col, int row, float time, ttyr_core_Color base) {
    float speed = 2.0f;
    float hue = fmodf((float)col * 0.1f + time * speed, 1.0f);

    ttyr_core_Color color;
    color.r = fabsf(hue - 0.5f) * 2.0f;
    color.g = fabsf(fmodf(hue + 0.33f, 1.0f) - 0.5f) * 2.0f;
    color.b = fabsf(fmodf(hue + 0.66f, 1.0f) - 0.5f) * 2.0f;
    return color;
}

static ttyr_core_Color get_matrix_rain_color(int col, int row, float time, int total_rows) {
    // Adjust droplet "start times" for more randomization
    float drop_start_time = randf(col, row, 0.0f) * 20.0f;  // Randomize start point
    float speed = 8.0f;

    // Determine if the drop is falling at this time
    float pos = fmodf(time * speed + drop_start_time, (float)(total_rows + 5));

    // Make sure drops reset once they move off screen (looping effect)
    if (pos < row && pos + 1.0f > row) {
        // This row should contain a rain drop at this position
        float brightness = 1.0f - (row - pos) * 0.1f;  // Fade based on distance from head

        // Randomize character density/brightness
        brightness = brightness * (randf(col, row, time + 5.0f) * 0.8f + 0.2f); 

        ttyr_core_Color color;
        color.r = 0.0f;   // Greenish rain
        color.g = brightness;  // Vary the brightness of the green rain
        color.b = 0.0f;

        return color;
    }
    
    // If no rain drop falls at this position, return no color
    ttyr_core_Color color = { 0.0f, 0.0f, 0.0f };  // No rain here
    return color;
}

static ttyr_core_Color get_falling_rain_color(int col, int row, float time, int total_rows, int total_cols) {
    // Randomize the start time for each drop to get a more constant flow of rain
    float drop_start_time = randf(col, row, 0.0f) * 20.0f;  // Randomize starting time for each drop

    // Speed of the falling rain
    float speed = 10.0f;  // Controls how fast the drops fall

    // Position of the drop at the current time
    float drop_position = fmodf(time * speed + drop_start_time, (float)(total_rows + 5));  // Wrap around effect

    // Only show a raindrop in a column if it's at the current row
    if (drop_position > row && drop_position - 1.0f < row) {
        // Brightness decreases as the raindrop falls
        float brightness = 1.0f - (drop_position - row) * 0.2f;  // Fade as it falls

        // Add randomness to the color/brightness for a dynamic effect
        brightness = brightness * (randf(col, row, time + 5.0f) * 0.8f + 0.2f);

        ttyr_core_Color rain_color;
        rain_color.r = 0.0f;      // Keep the raindrop color greenish
        rain_color.g = brightness; // Green component varies based on brightness
        rain_color.b = 0.0f;      // No blue component for the rain

        return rain_color;
    }

    // If no raindrop at this position, return no color (background)
    ttyr_core_Color no_rain = { 0.0f, 0.0f, 0.0f };
    return no_rain;
}

//static ttyr_core_Color get_matrix_rain_color(int col, int row, float time, int total_rows) {
//    // Each column has its own time offset (creates staggered drops)
//    float col_offset = randf(col, 0, 0.0f) * 10.0f;
//    float speed = 15.0f;
//
//    // Simulated falling head position
//    float head_pos = fmodf(time * speed + col_offset, (float)(total_rows + 10));
//
//    // Distance from head
//    float dist = (float)row - head_pos;
//
//    // Trail logic
//    float brightness = 0.0f;
//    if (dist >= 0.0f && dist < 10.0f) {
//        brightness = 1.0f - (dist / 10.0f);  // Fade from head to tail
//    }
//
//    // Occasional bright white head
//    float head_glow = (dist < 0.5f) ? 1.0f : 0.0f;
//    float is_white = randf(col, row, floorf(time)) > 0.95f ? 1.0f : 0.0f;
//
//    ttyr_core_Color color;
//    if (is_white && head_glow > 0.0f) {
//        // occasional white highlight
//        color.r = color.g = color.b = 1.0f;
//    } else {
//        color.r = 0.0f;
//        color.g = brightness;
//        color.b = 0.0f;
//    }
//
//    return color;
//}

static ttyr_core_Color get_popcorn_color(int col, int row, float time) {
    float burst = randf(col, row, floorf(time * 10.0f));
    if (burst > 0.92f) {
        ttyr_core_Color color;
        color.r = randf(col, row, time);
        color.g = randf(col, row, time + 1.0f);
        color.b = randf(col, row, time + 2.0f);
        return color;
    } else {
        ttyr_core_Color dark;
        dark.r = 0.05f;
        dark.g = 0.05f;
        dark.b = 0.05f;
        return dark;
    }
}

static ttyr_core_Color get_bubble_color(int col, int row, float time) {
    float shift = (col + row) * 0.1f + time;

    ttyr_core_Color color;
    color.r = (sinf(shift) + 1.0f) * 0.4f + 0.2f;
    color.g = (sinf(shift + 2.0f) + 1.0f) * 0.4f + 0.2f;
    color.b = (sinf(shift + 4.0f) + 1.0f) * 0.4f + 0.2f;
    return color;
}

static ttyr_core_Color get_crabby_crawlers_color(int col, int row, float time) {
    float jitter = randf(col, row, floorf(time * 4.0f)) * 0.3f;
    float wave = sinf(row * 0.5f + time * 3.0f + jitter);

    ttyr_core_Color color;
    color.r = 0.8f * wave;
    color.g = 0.3f + 0.3f * randf(col, row, time);
    color.b = 0.1f;
    return color;
}

static ttyr_core_Color ttyr_terminal_getAccentColor(
    ttyr_terminal_Config *Config_p, int col, int row, int total_cols, int total_rows, ttyr_core_Color base)
{
//    float time_now = (float)clock() / CLOCKS_PER_SEC;
float time_now = 0;
    switch (Config_p->style) {
        case 1 : return get_wave_color(col, row, time_now, total_cols, total_rows, base);
        case 2 : return get_strobe_color(col, row, time_now, base);
        case 3 : return get_rainbow_color(col, row, time_now, base);
        case 4 : return get_popcorn_color(col, row, time_now);
        case 5 : return get_bubble_color(col, row, time_now);
    }
    return base;
}

ttyr_core_Color ttyr_terminal_getGlyphColor(
    ttyr_terminal_Config *Config_p, ttyr_terminal_GraphicsState *State_p, ttyr_core_Glyph *Glyph_p, bool foreground, int col, int row, ttyr_terminal_Grid *Grid_p)
{
    if (foreground) {
        if (Glyph_p->Attributes.reverse || (Glyph_p->Attributes.blink && State_p->Blink.on)) {
            if (Glyph_p->Background.custom) {
                return Glyph_p->Background.Color;
            }
            if (Glyph_p->mark & TTYR_CORE_MARK_ACCENT) {
                ttyr_core_Color Color = ttyr_terminal_getAccentColor(Config_p, col, row, Grid_p->cols, Grid_p->rows, State_p->AccentGradient.Color);
                Color.r *= 0.3f;
                Color.g *= 0.3f;
                Color.b *= 0.3f;
                return Color;
            }
            return State_p->BackgroundGradient.Color;
        }
        if (Glyph_p->mark & TTYR_CORE_MARK_ACCENT) {
            return ttyr_terminal_getAccentColor(Config_p, col, row, Grid_p->cols, Grid_p->rows, State_p->AccentGradient.Color);
        }
        if (Glyph_p->Foreground.custom) {
            return Glyph_p->Foreground.Color;
        }
        return Config_p->Foreground;
    }

    // Background.
    if ((Glyph_p->Attributes.reverse && !(Glyph_p->Attributes.blink && State_p->Blink.on)) 
    || (!Glyph_p->Attributes.reverse &&   Glyph_p->Attributes.blink && State_p->Blink.on)) {
        if (Glyph_p->mark & TTYR_CORE_MARK_ACCENT) {
            return ttyr_terminal_getAccentColor(Config_p, col, row, Grid_p->cols, Grid_p->rows, State_p->AccentGradient.Color);
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

ttyr_core_Color ttyr_terminal_getGradientColor(
    ttyr_terminal_GraphicsGradient *Gradient_p, ttyr_core_Color *Colors_p, int colors)
{
    if (colors == 1) {
        return Colors_p[0];
    }

    ttyr_core_Color Color1 = Colors_p[Gradient_p->index];
    ttyr_core_Color Color2 = Gradient_p->index == colors-1 ? Colors_p[0] : Colors_p[Gradient_p->index+1];

    ttyr_core_Color Result;
    Result.r = Color1.r + Gradient_p->ratio * (Color2.r - Color1.r);
    Result.g = Color1.g + Gradient_p->ratio * (Color2.g - Color1.g);
    Result.b = Color1.b + Gradient_p->ratio * (Color2.b - Color1.b);
    Result.a = 1.0f;

    Gradient_p->ratio += 0.01f;

    return Result;
}
