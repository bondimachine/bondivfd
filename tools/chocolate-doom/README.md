# Chocolate Doom → BondiVFD

Parche para [Chocolate Doom](https://github.com/chocolate-doom/chocolate-doom)
que manda la barra de estado del juego a un display con el ejemplo
**BondiWeb** de BondiVFD: munición, salud, armadura, llaves y la cara del
Doomguy, tal como la elige el juego (dolor, mirando a los costados, ouch,
sonrisa malvada, furia, modo dios y muerto).

## Cómo funciona

El parche agrega a `src/doom/st_stuff.c` una función que corre en cada tic
del juego (`ST_Ticker`). Arma un texto como

```
ammo=47&health=61&armor=100&keys=br&face=12
```

y lo manda por UDP (SDL_net, que Chocolate Doom ya usa para jugar en red)
al puerto 5050 del ESP32, sólo cuando algo cambia y una vez por segundo
para que el display sepa que el juego sigue. `face` es el `st_faceindex` del
juego; `ammo=-1` es un arma sin munición. Si pasan 10 s sin datos (por
ejemplo, el juego en pausa o cerrado), el display vuelve a su show.

El mismo texto se puede mandar por HTTP a `POST /api/doom/live`, así que
cualquier otro port u otro juego puede usar el display igual.

## Compilar

Probado sobre el commit `a2dfd51` de Chocolate Doom, en macOS con Homebrew:

```bash
brew install cmake sdl2 sdl2_net
```
```bash
git clone https://github.com/chocolate-doom/chocolate-doom.git && cd chocolate-doom
```
```bash
git apply /ruta/a/bondivfd/tools/chocolate-doom/bondivfd.patch
```
```bash
cmake -S . -B build -DENABLE_SDL2_MIXER=OFF && cmake --build build --target chocolate-doom
```

(`-DENABLE_SDL2_MIXER=OFF` sólo hace falta si no se instaló `sdl2_mixer`;
sin él no hay música.)

## Jugar

```bash
./build/src/chocolate-doom -iwad doom.wad -bondivfd bondi.local
```

`-bondivfd host[:puerto]` acepta un nombre o una IP (por defecto el puerto
5050). Sin esa opción, el juego no manda nada. Al arrancar imprime
`BondiVFD: mandando la barra de estado a ...`.

Sin un IWAD de Doom, [Freedoom](https://freedoom.github.io/) funciona igual.
