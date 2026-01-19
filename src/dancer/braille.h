// Unicode Braille character rendering
// Braille patterns give us 2x4 dot matrix per character for smooth graphics

#pragma once

#include <stdint.h>

// Braille Unicode block starts at U+2800
// Each character is a 2x4 dot matrix:
//   Dot positions:    Bit values:
//   [1] [4]           0x01  0x08
//   [2] [5]           0x02  0x10
//   [3] [6]           0x04  0x20
//   [7] [8]           0x40  0x80

#define BRAILLE_BASE 0x2800

// Canvas for drawing with braille
// Each cell represents a 2x4 pixel area
struct braille_canvas {
    int width;          // Width in braille characters
    int height;         // Height in braille characters
    int pixel_width;    // Width in pixels (width * 2)
    int pixel_height;   // Height in pixels (height * 4)
    uint8_t *dots;      // Dot data (width * height bytes)
};

// Create a new braille canvas
struct braille_canvas *braille_create(int char_width, int char_height);

// Free a braille canvas
void braille_free(struct braille_canvas *canvas);

// Clear the canvas
void braille_clear(struct braille_canvas *canvas);

// Set a single pixel (x, y in pixel coordinates)
void braille_set_pixel(struct braille_canvas *canvas, int x, int y);

// Clear a single pixel
void braille_clear_pixel(struct braille_canvas *canvas, int x, int y);

// Draw a line (Bresenham's algorithm)
void braille_line(struct braille_canvas *canvas, int x0, int y0, int x1, int y1);

// Draw a filled circle
void braille_filled_circle(struct braille_canvas *canvas, int cx, int cy, int r);

// Draw a circle outline
void braille_circle(struct braille_canvas *canvas, int cx, int cy, int r);

// Draw an ellipse
void braille_ellipse(struct braille_canvas *canvas, int cx, int cy, int rx, int ry);

// Render canvas to UTF-8 string buffer
// Buffer must be large enough: width * 4 (UTF-8) * height + height (newlines) + 1
void braille_render(struct braille_canvas *canvas, char *output);

// Get required buffer size for render output
int braille_buffer_size(struct braille_canvas *canvas);
