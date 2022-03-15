#pragma once
#include "includes.h"



struct Vec2 {
    float x, y;
};

struct Vec3 {
    float x, y, z;
};

struct Vec4 {
    float x, y, z, w;
};

bool WorldToScreen(Vec3 pos, Vec2& screen);

void DrawFilledRect(int x, int y, int w, int h, D3DCOLOR color);

void DrawLine(int x1, int y1, int x2, int y2, int thickness, D3DCOLOR color);

void DrawLine2(Vec2 src, Vec2 dst, int thickness, D3DCOLOR color);

void DrawEspBox2D(Vec2 top, Vec2 bot, int thickness, D3DCOLOR color);

void DrawEspBox3D(Vec3 top, Vec3 bot, float a, int width, int thickness, D3DCOLOR color);

void TextToScreen(const char* text, float x, float y, D3DCOLOR color, int height, int width);