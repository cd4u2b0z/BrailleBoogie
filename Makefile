# ASCII Dancer Makefile v3.2
# Supports config files, 256-color themes, ground/shadow effects
# v2.4: Control bus, UI reactivity, enhanced particles
# v3.0: Advanced BPM tracker, dynamic energy analysis, background effects
# v3.2: macOS CoreAudio support

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -I./src
LDFLAGS = -lm -lfftw3 -lncursesw -lpthread

# Detect operating system
UNAME_S := $(shell uname -s)

PIPEWIRE_CFLAGS := $(shell pkg-config --cflags libpipewire-0.3 2>/dev/null)
PIPEWIRE_LIBS := $(shell pkg-config --libs libpipewire-0.3 2>/dev/null)
PULSE_CFLAGS := $(shell pkg-config --cflags libpulse-simple 2>/dev/null)
PULSE_LIBS := $(shell pkg-config --libs libpulse-simple libpulse 2>/dev/null)

# Common source files (v2.1 additions: config, colors)
COMMON_SRCS = src/main.c \
              src/audio/common.c \
              src/fft/cavacore.c \
              src/config/config.c \
              src/render/colors.c \
              src/render/render_new.c \
              src/effects/particles.c \
              src/effects/trails.c \
              src/effects/effects.c \
              src/audio/rhythm.c

# v2.4 additions: Control bus and UI reactivity
V24_SRCS = src/control/control_bus.c \
           src/ui/ui_reactive.c \
           src/ui/help_overlay.c

# v3.0 additions: Advanced audio analysis and background effects
V30_SRCS = src/audio/bpm_tracker.c \
           src/audio/energy_analyzer.c \
           src/effects/background_fx.c

# v3.0+: Export, audio picker, terminal caps, profiler
V30P_SRCS = src/export/frame_recorder.c \
            src/audio/audio_picker.c \
            src/ui/term_caps.c \
            src/ui/profiler.c

# Frame-based dancer (uses your custom braille frames)
FRAME_SRCS = src/dancer/dancer_rhythm.c

# Braille skeleton dancer (procedural with smooth interpolation)
BRAILLE_SRCS = src/braille/braille_canvas.c \
               src/braille/skeleton_dancer.c \
               src/braille/braille_dancer.c

# Audio sources
AUDIO_SRCS =

# macOS CoreAudio
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -D__APPLE__
    LDFLAGS += -framework CoreAudio -framework AudioToolbox -framework CoreFoundation
    AUDIO_SRCS += src/audio/coreaudio.c
    # macOS may need explicit ncurses path
    CFLAGS += -I/opt/homebrew/opt/ncurses/include -I/usr/local/opt/ncurses/include
    LDFLAGS := $(filter-out -lncursesw,$(LDFLAGS)) -L/opt/homebrew/opt/ncurses/lib -L/usr/local/opt/ncurses/lib -lncursesw
else
    # Linux audio backends
    ifneq ($(PIPEWIRE_CFLAGS),)
        CFLAGS += -DPIPEWIRE $(PIPEWIRE_CFLAGS)
        LDFLAGS += $(PIPEWIRE_LIBS)
        AUDIO_SRCS += src/audio/pipewire.c
    endif

    ifneq ($(PULSE_CFLAGS),)
        CFLAGS += -DPULSE $(PULSE_CFLAGS)
        LDFLAGS += $(PULSE_LIBS)
        AUDIO_SRCS += src/audio/pulse.c
    endif
endif

# Default: frame-based dancer
SRCS = $(COMMON_SRCS) $(FRAME_SRCS) $(AUDIO_SRCS)
OBJS = $(SRCS:.c=.o)
TARGET = asciidancer

# Braille target sources (includes v2.4, v3.0, and v3.0+ modules)
BRAILLE_ALL_SRCS = $(COMMON_SRCS) $(BRAILLE_SRCS) $(V24_SRCS) $(V30_SRCS) $(V30P_SRCS) $(AUDIO_SRCS)
BRAILLE_OBJS = $(BRAILLE_ALL_SRCS:.c=.o)

.PHONY: all braille clean install

# Build frame-based dancer (default)
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Build braille skeleton dancer
braille: clean-objs $(BRAILLE_OBJS)
	$(CC) $(BRAILLE_OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean-objs:
	find src -name "*.o" -delete 2>/dev/null || true

clean: clean-objs
	rm -f $(TARGET)

# Install to ~/.local/bin
install: $(TARGET)
	mkdir -p ~/.local/bin
	cp $(TARGET) ~/.local/bin/
	@echo "Installed to ~/.local/bin/$(TARGET)"
