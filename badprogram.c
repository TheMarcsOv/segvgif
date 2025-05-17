#include <stdio.h>
#include <unistd.h>

int main() {
    printf("Very normal program\n");
    sleep(1);
    *((int*)0) = 0;
    return 0;
}
