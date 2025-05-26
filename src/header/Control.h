#pragma once
#include"Pedestrian.h"
int ControllPedestrian(std::vector<Pedestrian*> &p, std::vector<Pedestrian*>& all_p, std::vector<Wall* >& wall,  double time) {
	for (int i = 0; i < p.size(); i++) {
        if(p[i]->is_live() == false) continue;
		if(p[i]->is_fall() == true) continue;

		if (p[i]->ComputeForce(p, all_p, wall, time)) {
			// std::cout << "asd" << std::endl;
			return i;
		}
		p[i]->ApplyForce(time);
	}
	return -1;
}

void handle_collision(std::vector<Pedestrian*>& all_p, float radius) {
	for (int i = 0; i < all_p.size(); i++) {
		if(all_p[i]->is_live() == false) continue;
		if(all_p[i]->is_fall() == true) continue;
		for (int j = i + 1; j < all_p.size(); j++) {
			if(all_p[j]->is_live() == false) continue;
			if(all_p[j]->is_fall() == true) continue; // 對方跌倒直接踩過去w
			glm::vec3 diff = all_p[i]->get_position() - all_p[j]->get_position();
			float distance = glm::length(diff);
			if (distance < radius * 2) {
				all_p[i]->set_fall();
				all_p[j]->set_fall();
			}
		}
	}
}