#include <stdio.h>
#include <X11/X.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"

#include "gifplay.h"

static GIF load_gif(const char* path, int upscale) {
    FILE* f = fopen(path, "rb");
    if (!f)
        return (GIF){};

    fseek(f, 0, SEEK_END);
    ssize_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* buffer = malloc(size);
    fread(buffer, 1, size, f);
    fclose(f);

    int* delays;
    int w, h, z, comp;
    unsigned char* gif = stbi_load_gif_from_memory(buffer, size, &delays, &w, &h, &z, &comp, 4);
    if (!gif)
        return (GIF) {};
    // if (gif) {
    //     printf("x: %d y: %d z: %d, comp: %d\n", w, h, z, comp);
    //     for (int i = 0; i < z; i++) {
    //         printf("delay %d = %d\n", i, delays[i]);
    //     }
    //     int frame = 20;
    //     int fs = frame * w * h; 
    //     for (int y = 0; y < h; y++) {
    //         for (int x = 0; x < w; x++) {
    //             int val = ((int)gif[(fs+y*w+x)*4+0] + (int)gif[(fs+y*w+x)*4+1] + gif[(fs+y*w+x)*4+2]) / 3;
    //             printf("%c", " .o#"[(val >> 6) & 3]);
    //         }
    //         printf("\n");
    //     }
    // }
    int nw = w * upscale;
    int nh = h * upscale;

    uint8_t* new_frames = malloc(z * nw * nh * 4);
    for (int f = 0; f < z; f++) {
        int src_fs = f * w * h;
        int fs = f * nw * nh;
        for (int y = 0; y < nh; y++) {
            int src_y = y / upscale;
            for (int x = 0; x < nw; x++) {
                int src_x = x / upscale;
                RGBA src_val = { .u = *(uint32_t*)&gif[(src_fs+src_y*w+src_x)*4] };

                //Swap RGBA -> BGRA
                new_frames[(fs+y*nw+x)*4 + 0] = src_val.b;
                new_frames[(fs+y*nw+x)*4 + 1] = src_val.g;
                new_frames[(fs+y*nw+x)*4 + 2] = src_val.r;
                new_frames[(fs+y*nw+x)*4 + 3] = src_val.a;
            }
        }
    }
    
    return (GIF) {new_frames, delays, nw, nh, z};
}

GIFPlayer init_gifplayer(const char* gif_path, int upscale) {
    GIFPlayer gp = {};
    // GIF Image
    gp.gif = load_gif(gif_path, upscale);
    if (!gp.gif.frames) {
        printf("Failed to load GIF %s\n", gif_path);
        return gp;
    }

    int width = gp.gif.w, height = gp.gif.h;

    char* display_str = getenv("DISPLAY");
    Display* display = XOpenDisplay(display_str);

    if (!display) {
        fprintf(stderr, "XOpenDisplay failed\n");
        return gp;
    }

    int screen = XDefaultScreen(display);
    // printf("Using screen %d\n", screen);

    if (XRenderQueryFormats(display) == 0) {
        return gp;
    }


    XVisualInfo visual_info;
    if (XMatchVisualInfo(display, screen, 32, TrueColor, &visual_info) == 0) {
        XCloseDisplay(display);
        fprintf(stderr, "XMatchVisual error\n");
        return gp;
    }
    
    // XRenderPictFormat* pic_format = XRenderFindVisualFormat(display, visual_info.visual);
    // printf("Alpha mask: %X\n", pic_format->direct.alphaMask);
    Visual* visual = visual_info.visual;

    XSetWindowAttributes attr;
    attr.colormap = XCreateColormap(display, RootWindow(display, screen), visual, AllocNone);
    attr.border_pixel = 0;
    attr.background_pixel = 0;
    
    gp.window = XCreateWindow(display, DefaultRootWindow(display),
                                  0, 0, width, height,
                                  0, visual_info.depth, InputOutput,
                                  visual, CWBackPixel | CWColormap | CWBorderPixel, &attr);
    
    XSetStandardProperties(display, gp.window, "GIF Player", "GIF Player", None, NULL, 0, NULL);

    Hints hints;
    hints.flags = 2;
    hints.decorations = 0;
    Atom prop = XInternAtom(display, "_MOTIF_WM_HINTS", True);
    XChangeProperty(display, gp.window, prop, prop, 32, PropModeReplace, (unsigned char*)&hints, 5);

    gp.wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, gp.window, &gp.wm_delete_window, 1);
    
    XSelectInput(display, gp.window, ExposureMask);

    gp.gc = XCreateGC(display, gp.window, 0, 0);
    XSetBackground(display, gp.gc, BlackPixel(display, screen));
    XSetForeground(display, gp.gc, WhitePixel(display, screen));

    unsigned long opacity = (unsigned long)(0xFFFFFFFFul * 1);
    Atom XA_NET_WM_WINDOW_OPACITY = XInternAtom(display, "_NET_WM_WINDOW_OPACITY", False);
    XChangeProperty(display, gp.window, XA_NET_WM_WINDOW_OPACITY, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char *) &opacity, 1L);

       
    gp.pixels = (uint8_t*) malloc(4 * gp.gif.w * gp.gif.h);
    gp.image = XCreateImage(display, visual, visual_info.depth, ZPixmap, 0, (char*)gp.pixels, gp.gif.w, gp.gif.h, 32, gp.gif.w * 4);

    gp.display = display;
    return gp;
}

void gifplayer_run(GIFPlayer* gifp, bool loop) {
    
    XClearWindow(gifp->display, gifp->window);
    XMapRaised(gifp->display, gifp->window);
    
    int t = 0;
    int running = 1;
    while (running) {
        while (XPending(gifp->display) > 0) {
            XEvent event;
            XNextEvent(gifp->display, &event);
            switch (event.type) {
                case Expose:
                    break;
                case ClientMessage:
                    if ((Atom) event.xclient.data.l[0] == gifp->wm_delete_window) {
                        running = 0;
                    }
                    break;
            }
            if (event.type == Expose) {
                XClearWindow(gifp->display, gifp->window);
            }
        }

        int f = t % gifp->gif.n;
        size_t fs = f * gifp->gif.w * gifp->gif.h * 4;
        memset(gifp->pixels, 0, gifp->gif.w * gifp->gif.h * 4);
        memcpy(gifp->pixels, gifp->gif.frames + fs, gifp->gif.w * gifp->gif.h * 4);
        XPutImage(gifp->display, gifp->window, gifp->gc, gifp->image, 0, 0, 0, 0, gifp->gif.w, gifp->gif.h);
        
        t++;
        usleep(1000 * gifp->gif.delays[f]);
        
        if (!loop && t == gifp->gif.n) {
            return;
        }
    }

    XFreeGC(gifp->display, gifp->gc);
    XDestroyWindow(gifp->display, gifp->window);
    XCloseDisplay(gifp->display);
}
