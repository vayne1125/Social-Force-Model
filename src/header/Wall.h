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
	std::vector<float> Spawn() {
		std::vector<float> vertices;
		vertices.push_back(point1.x());
		vertices.push_back(point1.y() );
		vertices.push_back(point1.z() );
		vertices.push_back(point2.x());
		vertices.push_back(point2.y());
		vertices.push_back(point2.z());
		return vertices;
	}
	Vector3<float> point1;
	Vector3<float> point2;


}; 
