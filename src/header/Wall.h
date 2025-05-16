#include <vector>
#include<iostream>
#include "Vec3.h"
#pragma once
class Wall {
public:
	Wall(Vector3<float> point1, Vector3<float> point2) {
		this->point1 = point1;
		this->point2 = point2;


	}
	std::vector<float> Spawn();
	Vector3<float> point1;
	Vector3<float> point2;


}; 
