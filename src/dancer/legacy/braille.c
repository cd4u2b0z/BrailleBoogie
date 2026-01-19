// Unicode Braille character rendering implementation

#include "braille.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Bit positions for each dot in a braille character
// Indexed by (y * 2 + x) where x is 0-1 and y is 0-3
static const uint8_t dot_bits[8] = {
    0x01, 0x08,  // Row 0: dots 1, 4
    0x02, 0x10,  // Row 1: dots 2, 5
    0x04, 0x20,  // Row 2: dots 3, 6
    0x40, 0x80   // Row 3: dots 7, 8
};

struct braille_canvas *braille_create(int char_width, int char_height) {
    struct braille_canvas *canvas = malloc(sizeof(struct braille_canvas));
    if (!canvas) return NULL;
    
    canvas->width = char_width;
    canvas->height = char_height;
    canvas->pixel_width = char_width * 2;
    canvas->pixel_height = char_height * 4;
    canvas->dots = calloc(char_width * char_height, sizeof(uint8_t));
    
    if (!canvas->dots) {
        free(canvas);
        return NULL;
    }
    
    return canvas;
}

void braille_free(struct braille_canvas *canvas) {
    if (canvas) {
        free(canvas->dots);
        free(canvas);
    }
}

void braille_clear(struct braille_canvas *canvas) {
    memset(canvas->dots, 0, canvas->width * canvas->height);
}

void braille_set_pixel(struct braille_canvas *canvas, int x, int y) {
    if (x < 0 || x >= canvas->pixel_width || y < 0 || y >= canvas->pixel_height)
        return;
    
    int char_x = x / 2;
    int char_y = y / 4;
    int dot_x = x % 2;
    int dot_y = y % 4;
    
    int char_idx = char_y * canvas->width + char_x;
    int dot_idx = dot_y * 2 + dot_x;
    
    canvas->dots[char_idx] |= dot_bits[dot_idx];
}

void braille_clear_pixel(struct braille_canvas *canvas, int x, int y) {
    if (x < 0 || x >= canvas->pixel_width || y < 0 || y >= canvas->pixel_height)
        return;
    
    int char_x = x / 2;
    int char_y = y / 4;
    int dot_x = x % 2;
    int dot_y = y % 4;
    
    int char_idx = char_y * canvas->width + char_x;
    int dot_idx = dot_y * 2 + dot_x;
    
    canvas->dots[char_idx] &= ~dot_bits[dot_idx];
}

void braille_line(struct braille_canvas *canvas, int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    
    while (1) {
        braille_set_pixel(canvas, x0, y0);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void braille_filled_circle(struct braille_canvas *canvas, int cx, int cy, int r) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x * x + y * y <= r * r) {
                braille_set_pixel(canvas, cx + x, cy + y);
            }
        }
    }
}

void braille_circle(struct braille_canvas *canvas, int cx, int cy, int r) {
    int x = r;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        braille_set_pixel(canvas, cx + x, cy + y);
        braille_set_pixel(canvas, cx + y, cy + x);
        braille_set_pixel(canvas, cx - y, cy + x);
        braille_set_pixel(canvas, cx - x, cy + y);
        braille_set_pixel(canvas, cx - x, cy - y);
        braille_set_pixel(canvas, cx - y, cy - x);
        braille_set_pixel(canvas, cx + y, cy - x);
        braille_set_pixel(canvas, cx + x, cy - y);
        
        y++;
        err += 1 + 2 * y;
        if (2 * (err - x) + 1 > 0) {
            x--;
            err += 1 - 2 * x;
        }
    }
}

void braille_ellipse(struct braille_canvas *canvas, int cx, int cy, int rx, int ry) {
    for (int angle = 0; angle < 360; angle++) {
        double rad = angle * M_PI / 180.0;
        int x = cx + (int)(rx * cos(rad));
        int y = cy + (int)(ry * sin(rad));
        braille_set_pixel(canvas, x, y);
    }
}

// Encode a Unicode codepoint to UTF-8
static int utf8_encode(uint32_t codepoint, char *out) {
    if (codepoint < 0x80) {
        out[0] = (char)codepoint;
        return 1;
    } else if (codepoint < 0x800) {
        out[0] = (char)(0xC0 | (codepoint >> 6));
        out[1] = (char)(0x80 | (codepoint & 0x3F));
        return 2;
    } else if (codepoint < 0x10000) {
        out[0] = (char)(0xE0 | (codepoint >> 12));
        out[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        out[2] = (char)(0x80 | (codepoint & 0x3F));
        return 3;
    }
    return 0;
}

void braille_render(struct braille_canvas *canvas, char *output) {
    char *ptr = output;
    
    for (int y = 0; y < canvas->height; y++) {
        for (int x = 0; x < canvas->width; x++) {
            uint8_t dots = canvas->dots[y * canvas->width + x];
            uint32_t codepoint = BRAILLE_BASE + dots;
            ptr += utf8_encode(codepoint, ptr);
        }
        *ptr++ = '\n';
    }
    *ptr = '\0';
}

int braille_buffer_size(struct braille_canvas *canvas) {
    // Each braille char is 3 bytes UTF-8, plus newlines, plus null
    return canvas->width * canvas->height * 3 + canvas->height + 1;
}
