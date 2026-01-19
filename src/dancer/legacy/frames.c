// ASCII art frames for the dancer
// Each body part has multiple frames based on intensity

#include "dancer.h"

// ==== ARM FRAMES ====
// 4 lines each, representing shoulders and arms

static const char *arms_down[] = {
    "                        ",
    "        \\    /         ",
    "         \\  /          ",
    "          \\/           "
};

static const char *arms_out[] = {
    "                        ",
    "                        ",
    "  \\              /     ",
    "   \\            /      "
};

static const char *arms_one_up[] = {
    "             /          ",
    "            /           ",
    "  \\        /           ",
    "   \\                   "
};

static const char *arms_both_up[] = {
    "   \\          /        ",
    "    \\        /         ",
    "     \\      /          ",
    "                        "
};

// ==== TORSO FRAMES ====
// 4 lines each, representing chest/body

static const char *torso_straight[] = {
    "        .-\"\"\"-. ",
    "       /  o o  \\",
    "       |   ^   |",
    "        \\_---_/ "
};

static const char *torso_lean_slight[] = {
    "        .-\"\"\"-. ",
    "       /  o o  \\",
    "       |   >   |",
    "        \\_---_/ "
};

static const char *torso_lean_forward[] = {
    "        .-\"\"\"-. ",
    "       /  ^ ^  \\",
    "       |   o   |",
    "        \\_===_/ "
};

// ==== LEG FRAMES ====
// 4 lines each, representing hips and legs

static const char *legs_standing[] = {
    "          | |           ",
    "          | |           ",
    "          | |           ",
    "         _/ \\_         "
};

static const char *legs_bent[] = {
    "          | |           ",
    "         /   \\         ",
    "        |     |         ",
    "        /     \\        "
};

static const char *legs_step[] = {
    "         /   \\         ",
    "        /     \\        ",
    "       /       \\       ",
    "      /         \\      "
};

static const char *legs_jump[] = {
    "       /         \\     ",
    "      /           \\    ",
    "     /             \\   ",
    "    <               >  "
};

// Frame accessor functions

const char **get_arms_frame(int index) {
    switch (index) {
    case 0: return arms_down;
    case 1: return arms_out;
    case 2: return arms_one_up;
    case 3: return arms_both_up;
    default: return arms_down;
    }
}

const char **get_torso_frame(int index) {
    switch (index) {
    case 0: return torso_straight;
    case 1: return torso_lean_slight;
    case 2: return torso_lean_forward;
    default: return torso_straight;
    }
}

const char **get_legs_frame(int index) {
    switch (index) {
    case 0: return legs_standing;
    case 1: return legs_bent;
    case 2: return legs_step;
    case 3: return legs_jump;
    default: return legs_standing;
    }
}
