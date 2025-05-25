#pragma once
#include"Pedestrian.h"
int ControllPedestrian(std::vector<Pedestrian*> &p, std::vector<Pedestrian*>& all_p, std::vector<Wall* >& wall,  double time) {
	for (int i = 0; i < p.size(); i++) {
        if(p[i]->is_live() == false) continue;
		if (p[i]->ComputeForce(p, all_p, wall,time)) {
			// std::cout << "asd" << std::endl;
			return i;
		}
		
		p[i]->ApplyForce(time);
	}
	return -1;
}