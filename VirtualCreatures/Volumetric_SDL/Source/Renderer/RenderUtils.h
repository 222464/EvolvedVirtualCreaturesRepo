#pragma once

#include <Constructs/Matrix4x4f.h>

void DrawSphere(float radius, int res);

int HighestMipMapLevel(int size);

void DrawNormalizedQuad();
void DrawQuadOriginCenter(float halfWidth, float halfHeight);
void DrawQuadOriginBottomLeft(float width, float height);