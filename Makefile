CFLAGS=-Wall -O3 -std=gnu11 -fPIC -Ilibs
LDFLAGS=-lm -lX11 -lXrender

all: gifplay libsegvhook.so badprogram

gifplay: gifplay_main.o gifplay.o
	gcc $(CFLAGS) $(LDFLAGS) $^ -o $@

libsegvhook.so: segvhook.o gifplay.o
	gcc $(CFLAGS) $(LDFLAGS) -shared $^ -o $@

badprogram: badprogram.o
	gcc $(CFLAGS) $(LDFLAGS) $^ -o $@

%.o: %.c
	gcc $(CFLAGS) -c $^ -o $@

clean:
	rm badprogram gifplay
	rm *.o *.so
