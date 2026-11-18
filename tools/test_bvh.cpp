#include <chrono>
#include <cstdio>
#include <random>
#include "bvh.hpp"

int main() {
    std::vector<Triangle> tris;
    const int N = 84, M = 84;
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < M; ++j) {
            auto P = [&](int a, int b) {
                float th = 3.14159265f * a / N, ph = 6.2831853f * b / M;
                return Vec3(std::sin(th) * std::cos(ph), std::cos(th), std::sin(th) * std::sin(ph));
            };
            Triangle t{};
            Vec3 p00 = P(i, j), p10 = P(i + 1, j), p01 = P(i, j + 1), p11 = P(i + 1, j + 1);
            t.v[0] = p00; t.v[1] = p10; t.v[2] = p11; tris.push_back(t);
            t.v[0] = p00; t.v[1] = p11; t.v[2] = p01; tris.push_back(t);
        }
    std::vector<Triangle> brute = tris;
    Bvh bvh;
    bvh.build(tris);
    std::printf("%zu triangulos, %zu nodos\n", tris.size(), bvh.nodeCount());

    std::mt19937 rng(1);
    std::uniform_real_distribution<float> U(-1.0f, 1.0f);
    const int R = 20000;
    std::vector<std::pair<Vec3, Vec3>> rays;
    for (int i = 0; i < R; ++i) {
        Vec3 o(U(rng) * 3, U(rng) * 3, 4.0f + U(rng));
        Vec3 target(U(rng) * 1.2f, U(rng) * 1.2f, U(rng) * 1.2f);
        rays.push_back({o, normalize(target - o)});
    }

    int mismatches = 0, hits = 0;
    for (auto& [o, d] : rays) {
        Hit a, b;
        bool ha = closestHit(brute, o, d, a);
        bool hb = bvh.closest(tris, o, d, b);
        if (ha != hb || (ha && std::fabs(a.t - b.t) > 1e-4f)) ++mismatches;
        if (ha) ++hits;
        bool sa = anyHit(brute, o, d, 5.0f), sb = bvh.any(tris, o, d, 5.0f);
        if (sa != sb) ++mismatches;
    }
    std::printf("rayos: %d, con impacto: %d, diferencias BVH vs fuerza bruta: %d\n", R, hits, mismatches);

    auto t0 = std::chrono::steady_clock::now();
    Hit h; int sink = 0;
    for (int k = 0; k < 2; ++k) for (auto& [o, d] : rays) sink += closestHit(brute, o, d, h);
    auto t1 = std::chrono::steady_clock::now();
    for (int k = 0; k < 2; ++k) for (auto& [o, d] : rays) sink += bvh.closest(tris, o, d, h);
    auto t2 = std::chrono::steady_clock::now();
    auto ms = [](auto a, auto b) { return std::chrono::duration<double, std::milli>(b - a).count(); };
    std::printf("fuerza bruta: %.1f ms | BVH: %.1f ms | (%d)\n", ms(t0, t1), ms(t1, t2), sink);
    return mismatches != 0;
}
