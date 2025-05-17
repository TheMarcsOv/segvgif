#include <stdio.h>
#include <stdlib.h>
#include "gifplay.h"

int main(int argc, char** argv) {
    if (argc < 2)
        return 1;
    
    printf("X11 test\n");
    GIFPlayer gifp = init_gifplayer(argv[1], argc >= 3 ? atoi(argv[2]) : 1);
    if (!GIF_PLAYER_VALID(gifp)) {
        return 1;
    }

    gifplayer_run(&gifp, 0);    
    
    return 0;
}
