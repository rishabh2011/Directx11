#include "MathHelper.h"

std::default_random_engine engine{ static_cast<unsigned int>(time(0)) };

float getRandomFloat(float min, float max)
{
	std::uniform_real_distribution<float> randomFloat{ min, max };
	return randomFloat(engine);
}

XMVECTOR getRandomVector(float min, float max)
{
	std::uniform_real_distribution<float> randomFloat{ min, max };
	XMVECTOR randomVector = XMVectorSet(randomFloat(engine), randomFloat(engine), randomFloat(engine), randomFloat(engine));
	return randomVector;
}