// ncurses rendering implementation with UTF-8 support

#include "render.h"
#include "../dancer/dancer.h"
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>

static int term_rows, term_cols;

int render_init(void) {
    // Enable UTF-8 support
    setlocale(LC_ALL, "");
    
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
        init_pair(6, COLOR_MAGENTA, -1);  // Energy indicator
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
    // Buffer for braille output (4 bytes per char + newlines + null)
    char frame[FRAME_WIDTH * FRAME_HEIGHT * 4 + FRAME_HEIGHT + 1];
    dancer_compose_frame(state, frame);

    // Calculate center position
    int start_row = (term_rows - FRAME_HEIGHT) / 2 - 2;
    int start_col = (term_cols - FRAME_WIDTH) / 2;

    if (start_row < 0) start_row = 0;
    if (start_col < 0) start_col = 0;

    // Draw each line
    if (has_colors()) attron(COLOR_PAIR(1) | A_BOLD);

    char *line = frame;
    int row = 0;
    while (*line && row < FRAME_HEIGHT) {
        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';

        // Use mvprintw which handles UTF-8 with proper locale
        mvprintw(start_row + row, start_col, "%s", line);

        if (newline) {
            line = newline + 1;
        } else {
            break;
        }
        row++;
    }

    if (has_colors()) attroff(COLOR_PAIR(1) | A_BOLD);
}

void render_bars(double bass, double mid, double treble) {
    int bar_width = 20;
    int bar_row = term_rows - 5;
    int total_width = bar_width * 3 + 10;
    int start_col = (term_cols - total_width) / 2;
    
    if (start_col < 0) start_col = 2;
    if (bar_row < FRAME_HEIGHT + 2) return;

    int bass_col = start_col;
    int mid_col = start_col + bar_width + 5;
    int treble_col = start_col + (bar_width + 5) * 2;

    // Draw labels
    if (has_colors()) attron(COLOR_PAIR(2));
    mvprintw(bar_row - 1, bass_col + bar_width/2 - 2, "BASS");
    if (has_colors()) attroff(COLOR_PAIR(2));
    
    if (has_colors()) attron(COLOR_PAIR(3));
    mvprintw(bar_row - 1, mid_col + bar_width/2 - 1, "MID");
    if (has_colors()) attroff(COLOR_PAIR(3));
    
    if (has_colors()) attron(COLOR_PAIR(4));
    mvprintw(bar_row - 1, treble_col + bar_width/2 - 3, "TREBLE");
    if (has_colors()) attroff(COLOR_PAIR(4));

    // Draw bars using block characters for smoother look
    int bass_len = (int)(bass * bar_width);
    int mid_len = (int)(mid * bar_width);
    int treble_len = (int)(treble * bar_width);

    // Bass bar
    if (has_colors()) attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(bar_row, bass_col, "[");
    for (int i = 0; i < bar_width; i++) {
        if (i < bass_len)
            printw("█");
        else
            printw(" ");
    }
    printw("]");
    if (has_colors()) attroff(COLOR_PAIR(2) | A_BOLD);

    // Mid bar
    if (has_colors()) attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(bar_row, mid_col, "[");
    for (int i = 0; i < bar_width; i++) {
        if (i < mid_len)
            printw("█");
        else
            printw(" ");
    }
    printw("]");
    if (has_colors()) attroff(COLOR_PAIR(3) | A_BOLD);

    // Treble bar
    if (has_colors()) attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(bar_row, treble_col, "[");
    for (int i = 0; i < bar_width; i++) {
        if (i < treble_len)
            printw("█");
        else
            printw(" ");
    }
    printw("]");
    if (has_colors()) attroff(COLOR_PAIR(4) | A_BOLD);
    
    // Energy indicator
    double total_energy = (bass + mid + treble) / 3.0;
    if (has_colors()) attron(COLOR_PAIR(6));
    mvprintw(bar_row + 2, (term_cols - 30) / 2, "Energy: ");
    int energy_bar = (int)(total_energy * 20);
    for (int i = 0; i < 20; i++) {
        if (i < energy_bar)
            printw("▓");
        else
            printw("░");
    }
    if (has_colors()) attroff(COLOR_PAIR(6));
}

void render_frame_info(struct dancer_state *state) {
    // Show current frame number in bottom corner
    if (has_colors()) attron(COLOR_PAIR(5));
    mvprintw(term_rows - 1, 2, "Frame: %d | B:%.2f M:%.2f T:%.2f | Press 'q' to quit",
             state->current_frame,
             state->bass_intensity,
             state->mid_intensity,
             state->treble_intensity);
    if (has_colors()) attroff(COLOR_PAIR(5));
}

void render_refresh(void) {
    refresh();
}

void render_info(const char *text) {
    if (has_colors()) attron(COLOR_PAIR(5));
    mvprintw(term_rows - 1, 2, "%s", text);
    if (has_colors()) attroff(COLOR_PAIR(5));
}

int render_getch(void) {
    return getch();
}
