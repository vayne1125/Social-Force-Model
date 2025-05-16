#include <vector>
#include "Vec3.h"
#pragma once
using namespace std;
class Goal {
public:
	Goal(Vector3<float> point1, Vector3<float> point2) {
		this->point1 = point1;
		this->point2 = point2;
	}
	Vector3<float> point1;
	Vector3<float> point2;
};