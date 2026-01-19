/*
 * Braille Canvas - High-resolution terminal graphics using Unicode Braille
 * 
 * Braille characters (U+2800-U+28FF) provide a 2x4 dot grid per cell,
 * giving 2x horizontal and 4x vertical resolution vs regular characters.
 *
 * Dot layout:     Bit values:
 *   1  4           0x01  0x08
 *   2  5           0x02  0x10
 *   3  6           0x04  0x20
 *   7  8           0x40  0x80
 */

#ifndef BRAILLE_CANVAS_H
#define BRAILLE_CANVAS_H

#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>

/* Braille base character and dimensions */
#define BRAILLE_BASE     0x2800
#define BRAILLE_CELL_W   2   /* pixels per cell horizontally */
#define BRAILLE_CELL_H   4   /* pixels per cell vertically */

/* Canvas structure */
typedef struct {
    int pixel_width;      /* Width in pixels (subpixels) */
    int pixel_height;     /* Height in pixels */
    int cell_width;       /* Width in terminal cells */
    int cell_height;      /* Height in terminal cells */
    uint8_t *pixels;      /* Pixel buffer: 1 byte per pixel (0 or 1) */
    wchar_t *cells;       /* Output buffer: braille characters */
    uint8_t *dirty;       /* Dirty flags per cell for partial updates */
} BrailleCanvas;

/* Lookup table for dot positions -> bit values */
static const uint8_t BRAILLE_DOT_BITS[4][2] = {
    {0x01, 0x08},  /* Row 0: dots 1, 4 */
    {0x02, 0x10},  /* Row 1: dots 2, 5 */
    {0x04, 0x20},  /* Row 2: dots 3, 6 */
    {0x40, 0x80}   /* Row 3: dots 7, 8 */
};

/* ============ Canvas Management ============ */

/* Create canvas with given terminal cell dimensions */
BrailleCanvas* braille_canvas_create(int cell_width, int cell_height);

/* Free canvas resources */
void braille_canvas_destroy(BrailleCanvas *canvas);

/* Clear all pixels */
void braille_canvas_clear(BrailleCanvas *canvas);

/* Convert pixel buffer to braille characters */
void braille_canvas_render(BrailleCanvas *canvas);

/* Get the rendered braille string for a row (null-terminated) */
const wchar_t* braille_canvas_get_row(BrailleCanvas *canvas, int row);

/* Get UTF-8 encoded output for ncurses */
int braille_canvas_to_utf8(BrailleCanvas *canvas, int row, char *out, int max_len);

/* ============ Pixel Operations ============ */

/* Set a single pixel (x, y in pixel coordinates) */
void braille_set_pixel(BrailleCanvas *canvas, int x, int y, bool on);

/* Get pixel value */
bool braille_get_pixel(BrailleCanvas *canvas, int x, int y);

/* Toggle pixel */
void braille_toggle_pixel(BrailleCanvas *canvas, int x, int y);

/* ============ Drawing Primitives ============ */

/* Draw line using Bresenham's algorithm */
void braille_draw_line(BrailleCanvas *canvas, int x1, int y1, int x2, int y2);

/* Draw anti-aliased line (Xiaolin Wu's algorithm) - smoother but slower */
void braille_draw_line_aa(BrailleCanvas *canvas, int x1, int y1, int x2, int y2);

/* Draw circle outline */
void braille_draw_circle(BrailleCanvas *canvas, int cx, int cy, int r);

/* Draw filled circle */
void braille_fill_circle(BrailleCanvas *canvas, int cx, int cy, int r);

/* Draw ellipse */
void braille_draw_ellipse(BrailleCanvas *canvas, int cx, int cy, int rx, int ry);

/* Draw rectangle outline */
void braille_draw_rect(BrailleCanvas *canvas, int x, int y, int w, int h);

/* Draw filled rectangle */
void braille_fill_rect(BrailleCanvas *canvas, int x, int y, int w, int h);

/* Draw quadratic bezier curve (3 control points) */
void braille_draw_bezier_quad(BrailleCanvas *canvas, 
                               int x0, int y0,   /* start */
                               int x1, int y1,   /* control */
                               int x2, int y2);  /* end */

/* Draw cubic bezier curve (4 control points) */
void braille_draw_bezier_cubic(BrailleCanvas *canvas,
                                int x0, int y0,   /* start */
                                int x1, int y1,   /* control 1 */
                                int x2, int y2,   /* control 2 */
                                int x3, int y3);  /* end */

/* Draw thick line (width in pixels) */
void braille_draw_thick_line(BrailleCanvas *canvas, 
                              int x1, int y1, int x2, int y2, int thickness);

/* ============ Utility ============ */

/* Flood fill from point */
void braille_flood_fill(BrailleCanvas *canvas, int x, int y, bool fill_value);

/* Copy region */
void braille_copy_region(BrailleCanvas *dst, int dx, int dy,
                         BrailleCanvas *src, int sx, int sy, int w, int h);

#endif /* BRAILLE_CANVAS_H */
