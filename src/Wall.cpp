#include "./header/Wall.h"
std::vector<float> Wall::Spawn() {
	std::vector<float> vertices;
	vertices.push_back(point1.x());
	vertices.push_back(point1.y() );
	vertices.push_back(point1.z() );
	vertices.push_back(point2.x());
	vertices.push_back(point2.y());
	vertices.push_back(point2.z());
	return vertices;
}