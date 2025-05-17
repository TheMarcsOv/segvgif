#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>

#define FPS 30

typedef struct {
    unsigned long   flags;
    unsigned long   functions;
    unsigned long   decorations;
    long            inputMode;
    unsigned long   status;
} Hints;

typedef struct {
    uint8_t* frames;
    int* delays;
    int w, h, n;
} GIF;

typedef union {
    uint32_t u;
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
} RGBA;

typedef struct {
    GIF gif;
    int window_w, window_h;
    Display* display;
    Window window;
    GC gc;
    uint8_t* pixels;
    XImage* image;
    Atom wm_delete_window;
} GIFPlayer;

#define GIF_PLAYER_VALID(gifp) ((gifp).display != NULL)

GIFPlayer init_gifplayer(const char* gif_path, int upscale);
void gifplayer_run(GIFPlayer* gifp, bool loop);
