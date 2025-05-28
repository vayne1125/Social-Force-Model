#pragma once
#include"Pedestrian.h"
void ControllPedestrian(std::vector<Pedestrian*> &p, std::vector<Pedestrian*>& all_p, std::vector<Wall* >& wall,  double time) {
	for (int i = 0; i < p.size(); i++) {
        if(p[i]->is_live() == false) continue;
		if(p[i]->is_fall() == true) continue;
		p[i]->ComputeForce(p, all_p, wall, time);
		p[i]->ApplyForce(time);
	}
}

void handleCollision(std::vector<Pedestrian*>& all_p, float radius) {
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

float getAvgSpeed(std::vector<Pedestrian*>& all_p) {
	float total_speed = 0.0f;
	int cnt = 0;
	for (int i = 0; i < all_p.size(); i++) {
        if(all_p[i]->is_live() == false) continue;
		if(all_p[i]->is_fall() == true) continue;
		total_speed += glm::length(all_p[i]->get_current_velocity());
        cnt++;
	}
	if (cnt > 0) {
        total_speed /= cnt;
		return total_speed;
	}
	return 0.0f;
}