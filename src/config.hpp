#pragma once
#include "vec3.hpp"

constexpr int WIDTH  = 960;
constexpr int HEIGHT = 720;

inline constexpr const char* MODEL_PATH = "assets/model.obj";

constexpr float FOV_DEG = 45.0f;

inline constexpr Vec3 LIGHT_OFFSET{1.0f, 2.0f, 1.0f};
constexpr float LIGHT_INTENSITY = 1.0f;
constexpr float AMBIENT = 0.15f;

inline constexpr Vec3 SKY_COLOR{0.5f, 0.7f, 1.0f};

constexpr float ROT_SPEED  = 1.5f;
constexpr float ZOOM_SPEED = 1.0f;

constexpr int PREVIEW_STEP = 3;
