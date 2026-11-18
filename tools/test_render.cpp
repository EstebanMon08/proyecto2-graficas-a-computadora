#include <cstdio>
#include "render.hpp"

int main(int argc, char** argv) {
    const char* path = argc > 1 ? argv[1] : MODEL_PATH;
    Scene scene;
    if (!loadObj(path, {"default"}, scene.tris)) return 1;
    Vec3 lo = scene.tris[0].v[0], hi = lo;
    for (auto& t : scene.tris) for (auto& v : t.v) { lo = vmin(lo, v); hi = vmax(hi, v); }
    Vec3 c = (lo + hi) * 0.5f; float size = length(hi - lo);
    scene.materials.push_back({ImageTexture::solid({0.8f, 0.5f, 0.3f}), 0.9f, 0.3f, 40.0f});
    scene.bvh.build(scene.tris);
    scene.light = c + LIGHT_OFFSET * size;
    OrbitCamera cam; cam.target = c; cam.radius = size * 1.5f;
    std::vector<uint8_t> buf((size_t)WIDTH * HEIGHT * 4);
    render(scene, cam, buf);
    FILE* f = fopen("out.ppm", "wb");
    fprintf(f, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
    for (size_t i = 0; i < (size_t)WIDTH * HEIGHT; ++i) fwrite(&buf[i * 4], 1, 3, f);
    fclose(f);
    printf("%zu triangulos -> out.ppm\n", scene.tris.size());
}
