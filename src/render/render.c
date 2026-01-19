// ncurses rendering implementation

#include "render.h"
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

static int term_rows, term_cols;

int render_init(void) {
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);  // Non-blocking getch
    curs_set(0);            // Hide cursor
    keypad(stdscr, TRUE);

    // Initialize colors if available
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_CYAN, -1);     // Dancer
        init_pair(2, COLOR_RED, -1);      // Bass bar
        init_pair(3, COLOR_GREEN, -1);    // Mid bar
        init_pair(4, COLOR_BLUE, -1);     // Treble bar
        init_pair(5, COLOR_YELLOW, -1);   // Info text
    }

    getmaxyx(stdscr, term_rows, term_cols);
    return 0;
}

void render_cleanup(void) {
    endwin();
}

void render_clear(void) {
    erase();
    getmaxyx(stdscr, term_rows, term_cols);
}

void render_dancer(struct dancer_state *state) {
    char frame[FRAME_HEIGHT * (FRAME_WIDTH + 2) + 1];
    dancer_compose_frame(state, frame);

    // Calculate center position
    int start_row = (term_rows - FRAME_HEIGHT) / 2;
    int start_col = (term_cols - FRAME_WIDTH) / 2;

    if (start_row < 0) start_row = 0;
    if (start_col < 0) start_col = 0;

    // Draw each line
    if (has_colors()) attron(COLOR_PAIR(1));

    char *line = frame;
    for (int i = 0; i < FRAME_HEIGHT && line; i++) {
        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';

        mvprintw(start_row + i, start_col, "%s", line);

        if (newline) {
            *newline = '\n';
            line = newline + 1;
        } else {
            break;
        }
    }

    if (has_colors()) attroff(COLOR_PAIR(1));
}

void render_bars(double bass, double mid, double treble) {
    int bar_width = 20;
    int bar_row = term_rows - 6;
    int bass_col = (term_cols / 2) - bar_width - 5;
    int mid_col = (term_cols / 2) - (bar_width / 2);
    int treble_col = (term_cols / 2) + 5;

    if (bar_row < FRAME_HEIGHT + 2) return;

    // Draw labels
    mvprintw(bar_row - 1, bass_col, "BASS");
    mvprintw(bar_row - 1, mid_col, "MID");
    mvprintw(bar_row - 1, treble_col, "TREBLE");

    // Draw bars
    int bass_len = (int)(bass * bar_width);
    int mid_len = (int)(mid * bar_width);
    int treble_len = (int)(treble * bar_width);

    char bar_str[32];

    // Bass bar
    if (has_colors()) attron(COLOR_PAIR(2));
    memset(bar_str, '=', bass_len);
    bar_str[bass_len] = '\0';
    mvprintw(bar_row, bass_col, "[%-*s]", bar_width, bar_str);
    if (has_colors()) attroff(COLOR_PAIR(2));

    // Mid bar
    if (has_colors()) attron(COLOR_PAIR(3));
    memset(bar_str, '=', mid_len);
    bar_str[mid_len] = '\0';
    mvprintw(bar_row, mid_col, "[%-*s]", bar_width, bar_str);
    if (has_colors()) attroff(COLOR_PAIR(3));

    // Treble bar
    if (has_colors()) attron(COLOR_PAIR(4));
    memset(bar_str, '=', treble_len);
    bar_str[treble_len] = '\0';
    mvprintw(bar_row, treble_col, "[%-*s]", bar_width, bar_str);
    if (has_colors()) attroff(COLOR_PAIR(4));
}

void render_info(const char *text) {
    if (has_colors()) attron(COLOR_PAIR(5));
    mvprintw(term_rows - 1, 0, "%s", text);
    if (has_colors()) attroff(COLOR_PAIR(5));
}

void render_refresh(void) {
    refresh();
}

int render_getch(void) {
    return getch();
}

void render_get_size(int *rows, int *cols) {
    *rows = term_rows;
    *cols = term_cols;
}
