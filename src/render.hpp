#pragma once
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <thread>
#include <vector>
#include "bvh.hpp"
#include "config.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "vec3.hpp"

struct SurfaceMaterial {
    ImageTexture texture;
    float albedo;
    float specular;
    float shininess;
};

struct Scene {
    std::vector<Triangle> tris;
    std::vector<SurfaceMaterial> materials;
    Bvh bvh;
    Vec3 light;
};

struct OrbitCamera {
    Vec3 target;
    float yaw = 0.6f, pitch = 0.4f, radius = 5.0f;

    Vec3 position() const {
        return target + Vec3(std::cos(pitch) * std::sin(yaw), std::sin(pitch),
                             std::cos(pitch) * std::cos(yaw)) * radius;
    }
    void ray(int x, int y, Vec3& o, Vec3& d) const {
        o = position();
        Vec3 forward = normalize(target - o);
        Vec3 right = normalize(cross(forward, Vec3(0, 1, 0)));
        Vec3 up = cross(right, forward);
        float halfH = std::tan(FOV_DEG * 3.14159265f / 180.0f * 0.5f);
        float halfW = halfH * (float)WIDTH / HEIGHT;
        float px = (2.0f * (x + 0.5f) / WIDTH - 1.0f) * halfW;
        float py = (1.0f - 2.0f * (y + 0.5f) / HEIGHT) * halfH;
        d = normalize(forward + right * px + up * py);
    }
};

inline Vec3 trace(const Scene& scene, Vec3 o, Vec3 d) {
    Hit hit;
    if (!scene.bvh.closest(scene.tris, o, d, hit)) return SKY_COLOR;

    const Triangle& tri = scene.tris[hit.tri];
    const SurfaceMaterial& mat = scene.materials[tri.mat];
    float w = 1.0f - hit.u - hit.v;

    Vec3 n = normalize(tri.n[0] * w + tri.n[1] * hit.u + tri.n[2] * hit.v);
    if (dot(n, d) > 0.0f) n = -n;

    float u = tri.uv[0][0] * w + tri.uv[1][0] * hit.u + tri.uv[2][0] * hit.v;
    float v = tri.uv[0][1] * w + tri.uv[1][1] * hit.u + tri.uv[2][1] * hit.v;
    Vec3 base = mat.texture.sample(u, v) * mat.albedo;

    Vec3 p = o + d * hit.t;
    Vec3 toLight = scene.light - p;
    float dist = length(toLight);
    Vec3 l = toLight * (1.0f / dist);

    float lit = scene.bvh.any(scene.tris, p + n * (EPS * 100.0f), l, dist) ? 0.0f : 1.0f;

    float diffuse = std::max(dot(n, l), 0.0f);
    Vec3 half = normalize(l - d);
    float spec = std::pow(std::max(dot(n, half), 0.0f), mat.shininess) * mat.specular;

    return base * (AMBIENT + diffuse * lit * LIGHT_INTENSITY) + Vec3(1, 1, 1) * (spec * lit);
}

inline void render(const Scene& scene, const OrbitCamera& cam, std::vector<uint8_t>& buf, int step = 1) {
    unsigned threads = std::max(1u, std::thread::hardware_concurrency());
    int rows = (HEIGHT + step - 1) / step;
    std::atomic<int> nextRow{0};

    auto to8 = [](float f) {
        return (uint8_t)(std::pow(std::clamp(f, 0.0f, 1.0f), 1.0f / 2.2f) * 255.0f);
    };

    auto worker = [&]() {
        for (;;) {
            int r = nextRow.fetch_add(1);
            if (r >= rows) break;
            int y = r * step;
            for (int x = 0; x < WIDTH; x += step) {
                Vec3 o, d;
                cam.ray(x, y, o, d);
                Vec3 c = trace(scene, o, d);
                uint8_t R = to8(c.x), G = to8(c.y), B = to8(c.z);
                for (int dy = 0; dy < step && y + dy < HEIGHT; ++dy)
                    for (int dx = 0; dx < step && x + dx < WIDTH; ++dx) {
                        size_t i = ((size_t)(y + dy) * WIDTH + (x + dx)) * 4;
                        buf[i] = R; buf[i + 1] = G; buf[i + 2] = B; buf[i + 3] = 255;
                    }
            }
        }
    };

    std::vector<std::thread> pool;
    for (unsigned i = 0; i < threads; ++i) pool.emplace_back(worker);
    for (auto& t : pool) t.join();
}
