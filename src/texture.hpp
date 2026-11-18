#pragma once
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include "vec3.hpp"

#ifndef NO_RAYLIB
#include "raylib.h"
#endif

class ImageTexture {
public:
    static ImageTexture solid(Vec3 c) {
        ImageTexture t;
        t.w_ = t.h_ = 1;
        t.data_ = {c};
        return t;
    }

    static ImageTexture load(const std::string& path, Vec3 fallback) {
#ifndef NO_RAYLIB
        Image img = LoadImage(path.c_str());
        if (img.data != nullptr) {
            Color* px = LoadImageColors(img);
            ImageTexture t;
            t.w_ = img.width;
            t.h_ = img.height;
            t.data_.reserve((size_t)t.w_ * t.h_);
            for (int i = 0; i < t.w_ * t.h_; ++i)
                t.data_.push_back({std::pow(px[i].r / 255.0f, 2.2f),
                                   std::pow(px[i].g / 255.0f, 2.2f),
                                   std::pow(px[i].b / 255.0f, 2.2f)});
            UnloadImageColors(px);
            UnloadImage(img);
            return t;
        }
#endif
        std::cerr << "AVISO: no pude cargar " << path << ", uso color solido\n";
        return solid(fallback);
    }

    Vec3 sample(float u, float v) const {
        int x = std::min((int)((u - std::floor(u)) * w_), w_ - 1);
        int y = std::min((int)((v - std::floor(v)) * h_), h_ - 1);
        return data_[(size_t)y * w_ + x];
    }

private:
    int w_ = 1, h_ = 1;
    std::vector<Vec3> data_{Vec3(1, 1, 1)};
};
