#include <signal.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define MINIAUDIO_IMPLEMENTATION
#include "libs/miniaudio.h"

#include "gifplay.h"

static const char* message = "Segmentation Fault";
static GIFPlayer gifp;
static const char* sound_path;
static ma_engine sound_engine;
static ma_sound sound;

void dll_init() __attribute__((constructor));

void sig_handler(int sig, siginfo_t* siginfo, void* ctx) {
    fprintf(stderr, "%s\n", message);

    if (sound_path) {
        ma_sound_start(&sound);
    }

    gifplayer_run(&gifp, false);
    exit(1);
}

void dll_init() {
    const char* msg = getenv("SEGVGIF_MSG");
    const char* gif_path = getenv("SEGVGIF_PATH");
    const char* upscale_str = getenv("SEGVGIF_UPSCALE");

    if (msg) message = msg;
    gif_path = gif_path == NULL ? "gifs/segfault.gif" : gif_path;
    int upscale = upscale_str == NULL ? 1 : atoi(upscale_str);
   
    gifp = init_gifplayer(gif_path, upscale);
    if (!GIF_PLAYER_VALID(gifp)) {
        printf("Could not initialize GIF player\n");
        return;
    }

    sound_path = getenv("SEGVGIF_SOUND");
    if (sound_path) {
        ma_result result = ma_engine_init(NULL, &sound_engine);
        if (result != MA_SUCCESS) {
            printf("Failed to init MA engine\n");
            return;
        }

       result = ma_sound_init_from_file(&sound_engine, sound_path, 0, NULL, NULL, &sound);
       if (result != MA_SUCCESS) {
            printf("Failed to load sound file: %s\n", sound_path);
       }
    }
    
    assert(0 == sigaction(SIGSEGV, &(struct sigaction) {.sa_sigaction = sig_handler, .sa_flags = SA_SIGINFO | SA_ONSTACK}, NULL));
}

