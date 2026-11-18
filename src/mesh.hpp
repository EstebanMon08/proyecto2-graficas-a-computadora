#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "vec3.hpp"

struct Triangle {
    Vec3 v[3];
    float uv[3][2];
    Vec3 n[3];
    int mat = 0;
};

struct Hit {
    float t, u, v;
    int tri;
};

constexpr float EPS = 1e-5f;

inline bool intersectTriangle(Vec3 o, Vec3 d, const Triangle& tri, float& t, float& u, float& v) {
    Vec3 e1 = tri.v[1] - tri.v[0];
    Vec3 e2 = tri.v[2] - tri.v[0];
    Vec3 h = cross(d, e2);
    float a = dot(e1, h);
    if (std::fabs(a) < 1e-9f) return false;
    float f = 1.0f / a;
    Vec3 s = o - tri.v[0];
    u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f) return false;
    Vec3 q = cross(s, e1);
    v = f * dot(d, q);
    if (v < 0.0f || u + v > 1.0f) return false;
    t = f * dot(e2, q);
    return t > EPS;
}

inline bool closestHit(const std::vector<Triangle>& tris, Vec3 o, Vec3 d, Hit& best) {
    bool found = false;
    best.t = 1e30f;
    for (int i = 0; i < (int)tris.size(); ++i) {
        float t, u, v;
        if (intersectTriangle(o, d, tris[i], t, u, v) && t < best.t) {
            best = {t, u, v, i};
            found = true;
        }
    }
    return found;
}

inline bool anyHit(const std::vector<Triangle>& tris, Vec3 o, Vec3 d, float maxT) {
    for (const Triangle& tri : tris) {
        float t, u, v;
        if (intersectTriangle(o, d, tri, t, u, v) && t < maxT) return true;
    }
    return false;
}

inline int resolveIndex(const std::string& s, int count) {
    if (s.empty()) return -1;
    int i = std::atoi(s.c_str());
    if (i > 0) return i - 1;
    if (i < 0) return count + i;
    return -1;
}

inline bool loadObj(const std::string& path, const std::vector<std::string>& knownMaterials,
                    std::vector<Triangle>& out) {
    std::ifstream file(path);
    if (!file) { std::cerr << "No pude abrir " << path << "\n"; return false; }

    std::vector<Vec3> vs, vns;
    std::vector<std::pair<float, float>> vts;
    int curMat = 0;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream it(line);
        std::string tag;
        it >> tag;
        if (tag == "v") {
            float x, y, z;
            if (it >> x >> y >> z) vs.push_back({x, y, z});
        } else if (tag == "vt") {
            float u, v;
            if (it >> u >> v) vts.push_back({u, 1.0f - v});
        } else if (tag == "vn") {
            float x, y, z;
            if (it >> x >> y >> z) vns.push_back(normalize({x, y, z}));
        } else if (tag == "usemtl") {
            std::string name;
            it >> name;
            curMat = -1;
            for (size_t k = 0; k < knownMaterials.size(); ++k)
                if (knownMaterials[k] == name) curMat = (int)k;
            if (curMat < 0) {
                std::cerr << "AVISO: material '" << name << "' no esta en MATERIAL_NAMES, uso el 0\n";
                curMat = 0;
            }
        } else if (tag == "f") {
            struct Corner { int v, t, n; };
            std::vector<Corner> corners;
            std::string tok;
            while (it >> tok) {
                std::string parts[3];
                int p = 0;
                for (char c : tok) { if (c == '/') { if (++p > 2) break; } else parts[p] += c; }
                Corner c{resolveIndex(parts[0], (int)vs.size()),
                         resolveIndex(parts[1], (int)vts.size()),
                         resolveIndex(parts[2], (int)vns.size())};
                if (c.v >= 0 && c.v < (int)vs.size()) corners.push_back(c);
            }
            for (size_t k = 1; k + 1 < corners.size(); ++k) {
                Corner c[3] = {corners[0], corners[k], corners[k + 1]};
                Triangle tri;
                tri.mat = curMat;
                for (int j = 0; j < 3; ++j) tri.v[j] = vs[c[j].v];
                Vec3 flat = normalize(cross(tri.v[1] - tri.v[0], tri.v[2] - tri.v[0]));
                for (int j = 0; j < 3; ++j) {
                    tri.uv[j][0] = tri.uv[j][1] = 0.0f;
                    tri.n[j] = flat;
                    if (c[j].t >= 0 && c[j].t < (int)vts.size()) {
                        tri.uv[j][0] = vts[c[j].t].first;
                        tri.uv[j][1] = vts[c[j].t].second;
                    }
                    if (c[j].n >= 0 && c[j].n < (int)vns.size()) tri.n[j] = vns[c[j].n];
                }
                out.push_back(tri);
            }
        }
    }
    if (out.empty()) { std::cerr << path << ": no se encontraron triangulos\n"; return false; }
    return true;
}
