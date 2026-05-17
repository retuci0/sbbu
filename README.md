# super bert bros ultimate

> faithful C++ port of [super bert bros](https://github.com/retuci0/super-bert-bros)


## dependencies

```
libsdl2-dev
libsdl2-image-dev
libsdl2-mixer-dev
libsdl2-ttf-dev
cmake >= 3.16
```

on Arch (btw):
```
sudo pacman -S sdl2 sdl2_image sdl2_mixer sdl2_ttf cmake base-devel
```

## font

the build will search for common system fonts automatically. to override, place any TTF file at `assets/font.ttf`.

## build

```bash
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

## run

must be run next to `assets/` folder

```bash
cd ../super-bert-bros
../super-bert-bros-cpp/build/super_bert_bros
```

## controls

| Action           | Player 1    | Player 2  |
|------------------|-------------|-----------|
| Move left/right  | A / D       | ← / →     |
| Jump             | Space       | ↑         |
| Shoot            | E           | RCtrl     |
| Toggle hitboxes  | B           | —         |
| Pause            | Esc         | —         |
| Quit             | 0           | —         |