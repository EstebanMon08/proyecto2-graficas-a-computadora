#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include "raylib.h"
#include "config.hpp"
#include "render.hpp"

struct MaterialDef {
    const char* name;
    const char* texture;
    Vec3 fallback;
    float albedo, specular, shininess;
};

static const std::vector<MaterialDef> MATERIALS = {
    {"Body",       "assets/Body.png",       Vec3(0.90f, 0.70f, 0.60f), 0.9f, 0.2f, 30.0f},
    {"Dress",      "assets/Dress.png",      Vec3(0.80f, 0.20f, 0.30f), 0.9f, 0.1f, 20.0f},
    {"Hair",       "assets/Hair.png",       Vec3(0.30f, 0.20f, 0.10f), 0.9f, 0.3f, 40.0f},
    {"Eye1",       "assets/Eye1.png",       Vec3(0.95f, 0.95f, 0.95f), 0.9f, 0.5f, 80.0f},
    {"Eye2",       "assets/Eye2.png",       Vec3(0.20f, 0.40f, 0.80f), 0.9f, 0.5f, 80.0f},
    {"Shoe1",      "assets/Shoe1.png",      Vec3(0.40f, 0.25f, 0.15f), 0.9f, 0.2f, 30.0f},
    {"Shoe2",      "assets/Shoe2.png",      Vec3(0.40f, 0.25f, 0.15f), 0.9f, 0.2f, 30.0f},
    {"Flower1",    "assets/Flower1.png",    Vec3(0.95f, 0.80f, 0.20f), 0.9f, 0.1f, 20.0f},
    {"Flower2",    "assets/Flower2.png",    Vec3(0.90f, 0.40f, 0.60f), 0.9f, 0.1f, 20.0f},
    {"Flower3",    "assets/Flower3.png",    Vec3(0.70f, 0.40f, 0.90f), 0.9f, 0.1f, 20.0f},
    {"SpearPlant", "assets/SpearPlant.png", Vec3(0.20f, 0.60f, 0.20f), 0.9f, 0.1f, 20.0f},
    {"SpearStick", "assets/SpearStick.png", Vec3(0.50f, 0.35f, 0.20f), 0.9f, 0.1f, 20.0f},
    {"SpearTip",   "assets/SpearTip.png",   Vec3(0.70f, 0.70f, 0.75f), 0.9f, 0.6f, 80.0f},
};

static std::vector<std::string> materialNames() {
    std::vector<std::string> names;
    for (const MaterialDef& m : MATERIALS) names.push_back(m.name);
    return names;
}

static std::vector<SurfaceMaterial> buildMaterials() {
    std::vector<SurfaceMaterial> out;
    for (const MaterialDef& m : MATERIALS)
        out.push_back({ImageTexture::load(m.texture, m.fallback), m.albedo, m.specular, m.shininess});
    return out;
}

int main() {
    Scene scene;
    if (!loadObj(MODEL_PATH, materialNames(), scene.tris)) return 1;
    std::cout << "Cargados " << scene.tris.size() << " triangulos\n";
    scene.bvh.build(scene.tris);
    std::cout << "BVH: " << scene.bvh.nodeCount() << " nodos\n";

    Vec3 lo = scene.tris[0].v[0], hi = lo;
    for (const Triangle& t : scene.tris)
        for (const Vec3& v : t.v) { lo = vmin(lo, v); hi = vmax(hi, v); }
    Vec3 center = (lo + hi) * 0.5f;
    float size = length(hi - lo);

    SetTraceLogLevel(LOG_WARNING);
    InitWindow(WIDTH, HEIGHT, "Diorama");
    scene.materials = buildMaterials();
    scene.light = center + LIGHT_OFFSET * size;

    OrbitCamera cam;
    cam.target = center;
    cam.radius = size * 1.5f;

    Image blank = GenImageColor(WIDTH, HEIGHT, BLACK);
    Texture2D screen = LoadTextureFromImage(blank);
    UnloadImage(blank);
    std::vector<uint8_t> buf((size_t)WIDTH * HEIGHT * 4);
    bool needFull = true;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        bool moved = false;
        if (IsKeyDown(KEY_LEFT))  { cam.yaw   -= ROT_SPEED * dt; moved = true; }
        if (IsKeyDown(KEY_RIGHT)) { cam.yaw   += ROT_SPEED * dt; moved = true; }
        if (IsKeyDown(KEY_UP))    { cam.pitch += ROT_SPEED * dt; moved = true; }
        if (IsKeyDown(KEY_DOWN))  { cam.pitch -= ROT_SPEED * dt; moved = true; }
        if (IsKeyDown(KEY_W)) { cam.radius *= 1.0f - ZOOM_SPEED * dt; moved = true; }
        if (IsKeyDown(KEY_S)) { cam.radius *= 1.0f + ZOOM_SPEED * dt; moved = true; }
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) { cam.radius *= 1.0f - wheel * 0.1f; moved = true; }
        cam.pitch = std::clamp(cam.pitch, -1.5f, 1.5f);
        cam.radius = std::max(cam.radius, size * 0.2f);

        if (moved) {
            render(scene, cam, buf, PREVIEW_STEP);
            UpdateTexture(screen, buf.data());
            needFull = true;
        } else if (needFull) {
            render(scene, cam, buf, 1);
            UpdateTexture(screen, buf.data());
            needFull = false;
        }
        BeginDrawing();
        DrawTexture(screen, 0, 0, WHITE);
        DrawFPS(8, 8);
        EndDrawing();
    }
    UnloadTexture(screen);
    CloseWindow();
    return 0;
}
