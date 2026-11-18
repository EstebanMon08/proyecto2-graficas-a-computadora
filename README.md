# Diorama Raytracer (C++17 + Raylib)

## Uso
1. Pon tu modelo en `assets/model.obj` y sus texturas en `assets/`.
2. Edita `MATERIAL_NAMES` y `buildMaterials()` en `src/main.cpp` para que coincidan con los `usemtl` de tu OBJ.
3. Compila y corre:
   ```
   cmake -B build
   cmake --build build
   ./build/diorama        (Windows: build\Debug\diorama.exe o build\Release\diorama.exe)
   ```
   Si `assets/` no se copia, ejecuta desde la carpeta que la contiene.

Controles: flechas = rotar, W/S o rueda = zoom.

## Si va lento
Baja `WIDTH`/`HEIGHT` en `src/config.hpp`. Sin BVH (hito 2) el costo es proporcional al numero de triangulos.

## Prueba sin ventana
```
g++ -std=c++17 -O2 -DNO_RAYLIB -Isrc tools/test_render.cpp -o test_render -pthread
./test_render assets/model.obj
```
Genera `out.ppm`.
