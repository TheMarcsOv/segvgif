# SEGV Gif Player
A shared library hook that notifies you when a program segfaults.

## Usage
This program requires an X11 desktop or a Wayland Desktop with XWayland (in fact, I have developped this on the latter).

Build requirements:
* GCC
* Xorg development libraries (i.e. xorg-dev)

Compilation:
```sh
make -j$(nproc)
```

Demo:
```sh
bash                  # It is recommended to run this on a separate shell
source segvhook.env   # Default environment variables
./badprogram          # Run a program that triggers a SEGFAULT
exit
```

Manually running a specific program with segvhook:
```sh
LD_PRELOAD=libsegvhook.so SEGVGIF_PATH=$(gifpath) SEGVGIF_SOUND=$(soundpath) ./badprogram
```

You can also export LD_PRELOAD so that segvhook is attached to any process ran on the current shell.
```sh
export LD_PRELOAD=libsegvhook.so
./badprogram
```

## Configuration
segvhook is configured through environment variables. See segvhook.env as an example.

* `SEGVGIF_PATH`: Path to the GIF file to be shown by segvhook. Must be set for segvhook to run. Can be absolute or relative.
* `SEGVGIF_UPSCALE`: Scaling factor for the image, must be an integer >= 1.
* `SEGVGIF_SOUND`: Path to the sound file to be played by segvhook. Optional.
* `SEGVGIF_MSG`: Message to be printed when the signal is triggered. If not set, prints the default message.

## Caveats
* The hook works by using LD_PRELOAD. Use at your own risk.
* The dll_init function is run every time `libsegvhook.so` is loaded to a program.
It initializes the X11 window and loads the configured assets, causing a noticeable initial delay.
* stb_image.h animated GIF loading is not perfect, some GIFs may render with artifacts, especially in transparent regions. 

## Credits

### Dependencies
* [stb_image.h](https://github.com/nothings/stb) by nothings/Sean Barett
* [miniaudio.h](https://github.com/mackron/miniaudio) by mackron/David Reid

### Assets
* gifs/segfault.gif: Original GIF without caption [1](https://tenor.com/es/view/death-gif-23535500)
* sounds/wetfart.wav: Wet fart/Brap sound effect [1](https://knowyourmeme.com/memes/brap-sound-effect-nerf-this)[2](https://www.youtube.com/watch?v=4gcs5k8n-FY)
