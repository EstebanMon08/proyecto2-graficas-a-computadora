#pragma once
#include <algorithm>
#include <vector>
#include "mesh.hpp"
#include "vec3.hpp"

struct BvhNode {
    Vec3 bmin, bmax;
    int a = 0;
    int count = 0;
};

inline float axisOf(Vec3 v, int axis) { return axis == 0 ? v.x : (axis == 1 ? v.y : v.z); }

inline float safeInv(float x) {
    if (std::fabs(x) < 1e-12f) return x < 0.0f ? -1e12f : 1e12f;
    return 1.0f / x;
}

inline bool hitBox(const BvhNode& n, Vec3 o, Vec3 invD, float tmax, float& tnear) {
    float t1 = (n.bmin.x - o.x) * invD.x, t2 = (n.bmax.x - o.x) * invD.x;
    float tn = std::min(t1, t2), tf = std::max(t1, t2);
    t1 = (n.bmin.y - o.y) * invD.y; t2 = (n.bmax.y - o.y) * invD.y;
    tn = std::max(tn, std::min(t1, t2)); tf = std::min(tf, std::max(t1, t2));
    t1 = (n.bmin.z - o.z) * invD.z; t2 = (n.bmax.z - o.z) * invD.z;
    tn = std::max(tn, std::min(t1, t2)); tf = std::min(tf, std::max(t1, t2));
    tnear = tn;
    return tf >= std::max(tn, 0.0f) && tn < tmax;
}

class Bvh {
public:
    static constexpr int LEAF_SIZE = 4;

    void build(std::vector<Triangle>& tris) {
        nodes_.clear();
        nodes_.reserve(tris.size() / 2 + 4);
        nodes_.emplace_back();
        buildNode(0, tris, 0, (int)tris.size());
    }

    bool closest(const std::vector<Triangle>& tris, Vec3 o, Vec3 d, Hit& best) const {
        Vec3 invD(safeInv(d.x), safeInv(d.y), safeInv(d.z));
        best.t = 1e30f;
        bool found = false;
        int stackIdx[64];
        float stackT[64];
        int sp = 0;
        float tn;
        if (!hitBox(nodes_[0], o, invD, best.t, tn)) return false;
        stackIdx[sp] = 0; stackT[sp++] = tn;

        while (sp > 0) {
            --sp;
            if (stackT[sp] >= best.t) continue;
            const BvhNode& nd = nodes_[stackIdx[sp]];
            if (nd.count > 0) {
                for (int i = nd.a; i < nd.a + nd.count; ++i) {
                    float t, u, v;
                    if (intersectTriangle(o, d, tris[i], t, u, v) && t < best.t) {
                        best = {t, u, v, i};
                        found = true;
                    }
                }
            } else {
                float tl, tr;
                bool hl = hitBox(nodes_[nd.a], o, invD, best.t, tl);
                bool hr = hitBox(nodes_[nd.a + 1], o, invD, best.t, tr);
                if (hl && hr) {
                    if (tl < tr) {
                        stackIdx[sp] = nd.a + 1; stackT[sp++] = tr;
                        stackIdx[sp] = nd.a;     stackT[sp++] = tl;
                    } else {
                        stackIdx[sp] = nd.a;     stackT[sp++] = tl;
                        stackIdx[sp] = nd.a + 1; stackT[sp++] = tr;
                    }
                } else if (hl) {
                    stackIdx[sp] = nd.a;     stackT[sp++] = tl;
                } else if (hr) {
                    stackIdx[sp] = nd.a + 1; stackT[sp++] = tr;
                }
            }
        }
        return found;
    }

    bool any(const std::vector<Triangle>& tris, Vec3 o, Vec3 d, float maxT) const {
        Vec3 invD(safeInv(d.x), safeInv(d.y), safeInv(d.z));
        int stack[64];
        int sp = 0;
        float tn;
        if (!hitBox(nodes_[0], o, invD, maxT, tn)) return false;
        stack[sp++] = 0;

        while (sp > 0) {
            const BvhNode& nd = nodes_[stack[--sp]];
            if (nd.count > 0) {
                for (int i = nd.a; i < nd.a + nd.count; ++i) {
                    float t, u, v;
                    if (intersectTriangle(o, d, tris[i], t, u, v) && t < maxT) return true;
                }
            } else {
                if (hitBox(nodes_[nd.a], o, invD, maxT, tn))     stack[sp++] = nd.a;
                if (hitBox(nodes_[nd.a + 1], o, invD, maxT, tn)) stack[sp++] = nd.a + 1;
            }
        }
        return false;
    }

    size_t nodeCount() const { return nodes_.size(); }

private:
    std::vector<BvhNode> nodes_;

    static Vec3 centroid(const Triangle& t) { return (t.v[0] + t.v[1] + t.v[2]) * (1.0f / 3.0f); }

    void buildNode(int idx, std::vector<Triangle>& tris, int begin, int end) {
        Vec3 lo(1e30f, 1e30f, 1e30f), hi(-1e30f, -1e30f, -1e30f);
        Vec3 clo = lo, chi = hi;
        for (int i = begin; i < end; ++i) {
            for (int j = 0; j < 3; ++j) { lo = vmin(lo, tris[i].v[j]); hi = vmax(hi, tris[i].v[j]); }
            Vec3 c = centroid(tris[i]);
            clo = vmin(clo, c); chi = vmax(chi, c);
        }
        nodes_[idx].bmin = lo;
        nodes_[idx].bmax = hi;

        int n = end - begin;
        Vec3 ext = chi - clo;
        int axis = (ext.x >= ext.y && ext.x >= ext.z) ? 0 : (ext.y >= ext.z ? 1 : 2);
        if (n <= LEAF_SIZE || axisOf(ext, axis) <= 0.0f) {
            nodes_[idx].a = begin;
            nodes_[idx].count = n;
            return;
        }

        int mid = begin + n / 2;
        std::nth_element(tris.begin() + begin, tris.begin() + mid, tris.begin() + end,
                         [axis](const Triangle& p, const Triangle& q) {
                             return axisOf(centroid(p), axis) < axisOf(centroid(q), axis);
                         });

        int left = (int)nodes_.size();
        nodes_.emplace_back();
        nodes_.emplace_back();
        nodes_[idx].a = left;
        nodes_[idx].count = 0;
        buildNode(left, tris, begin, mid);
        buildNode(left + 1, tris, mid, end);
    }
};
