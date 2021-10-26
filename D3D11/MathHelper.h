#pragma once
#include <random>
#include <cstdlib>
#include "d3dUtil.h"

extern std::default_random_engine engine;
extern float getRandomFloat(float min, float max);
extern XMVECTOR getRandomVector(float min, float max);
extern float degToRadians(float angle);