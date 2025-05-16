#include "./header/Pedestrian.h"
#include <random>
#include <math.h>
#include <algorithm>
# define M_PI 3.14159265358979323846
std::vector<float> Pedestrian:: DrawCircle() {

	this->vertices.clear();
	float aspect = 2000 / 1000; 
	vertices.push_back(this->position.x());
	vertices.push_back(this->position.y());
	vertices.push_back(this->position.z());
	for (int i = 0; i <= segement; i++) {
		float theta = 2.0f * 3.1415926f * float(i) / float(segement);

		float x = this->position.x() +radius * cosf(theta);
		float y = this->position.y() +radius * sinf(theta);
		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(0.0f);
	}
	return vertices;
}
std::vector<float> Pedestrian::Walk() {
	return DrawCircle();
}
float Pedestrian::generategaussian(float mean, float standard_deviation)
{
	std::random_device rd;  
	std::mt19937 gen(rd()); 

	std::uniform_real_distribution<> dis(0.0, 1.0);

	float u1 = 1.0f - dis(gen);
	float u2 = 1.0f - dis(gen);
	float randStdNormal = sqrt(-2.0f * log(u1)) * sin(2.0f * M_PI * u2);
	return mean + standard_deviation * randStdNormal;
}
bool Pedestrian::ComputeForce(std::vector<Pedestrian*>& p,std::vector<Pedestrian*>& all_p, std::vector<Wall* >& wall,float time) {

	if (Comput_Goal_Distance()) {
		if (Goal_Number < Goal.size() - 1)
			Goal_Number++;
		else
			destroy = true;
	}
	if (destroy) {
		auto it = std::find(all_p.begin(), all_p.end(), this);

		if (it != all_p.end()) {
			all_p.erase(it);
		}
		it = std::find(p.begin(), p.end(), this);
		if (it != p.end()) {
			p.erase(it);
		}
		if (friend_number != NULL)
			friend_number->friend_number = NULL;
		delete this;
		return true;
	}
	float goal_distance = std::numeric_limits<float>::max();
	if (Goal_Number > 0)
	{
		Vector3<float>dir = Goal[Goal_Number - 1].point2 - Goal[Goal_Number - 1].point1;
		Vector3<float> p_g = position - Goal[Goal_Number - 1].point1;
		if (((Goal[Goal_Number - 1].point1 + ClosePoint(dir, p_g) * dir) - position).norm() < goal_distance)
		{
			goal_distance = ((Goal[Goal_Number - 1].point1 + ClosePoint(dir, p_g) * dir) - position).norm();
			if (goal_distance >= 0)
			{
				Vector3<float> now_distance = ((Goal[Goal_Number - 1].point1 + ClosePoint(dir, p_g) * dir) - position);
				Vector3<float> pre_distance = (Goal[Goal_Number - 1].point1 + ClosePoint(dir, p_g) * dir) - prePosition[Goal_Number - 1];
				if (dot(now_distance, pre_distance) >= 0)
					Goal_Number--;
				else if (Goal_Number < prePosition.size() - 1)
					prePosition.push_back(position);
			}
		}

		
	}
	force = Vector3<float>(0, 0, 0);
	if (actual_velocity.norm() != 0)
		edge = actual_velocity / actual_velocity.norm();
	Vector3<float>dir = Goal[Goal_Number].point2 - Goal[Goal_Number].point1;
	Vector3<float> p_g = position - Goal[Goal_Number].point1;
	Vector3<float> close = Goal[Goal_Number].point1 + ClosePoint(dir, p_g) * dir;
	desired_direction = (close - position) / (close - position).norm();
	force = force + ((desired_direction * desired_speed - actual_velocity) / relaxation_time);//point1


	for (int i = 0; i < all_p.size(); i++) {
		if (all_p[i] != this) {

			Vector3<float> diff = position - all_p[i]->position;

			float distance = diff.norm();
			Vector3<float> nij = diff / diff.norm();
			Vector3<float> tij = Vector3<float>(-nij.y(), nij.x(), 0);
			Vector3<float> deltaV = actual_velocity;

			//Calculate b (collision distance)
			Vector3<float> future_diff = diff - (all_p[i]->velocity-velocity) * (float)time ;
			float fab = pow((distance + future_diff.norm()),2)  - dot(((all_p[i]->velocity-velocity) * (float)time), ((all_p[i]->velocity - velocity) * (float)time));
			float b = 0.5 * std::sqrt(fab);

			// Calculate repulsive force
			 
			Vector3<float> repulsive_force = A * exp(-b / B) * ((distance + future_diff.norm()) / (2 * b)) * (0.5f) * ((diff / diff.norm()) + (future_diff / future_diff.norm()));
			if (friend_number != NULL && friend_number == all_p[i])
				force = force + Weight(Vector3<float>(0, 0, 0) - repulsive_force) * repulsive_force * 0.5f;
			else
				force = force + Weight(Vector3<float>(0, 0, 0) - repulsive_force) * repulsive_force;

		}//pedestrian
	}
	
	for (int i = 0; i < wall.size(); i++)
	{
		Vector3<float>dir = wall[i]->point2 - wall[i]->point1;
		Vector3<float> p_g = position - wall[i]->point1;
		Vector3<float> close = wall[i]->point1 + ClosePoint(dir, p_g) * dir;
		Vector3<float> diff = position - close;
		float distance = diff.norm();
		Vector3<float> niw=diff/diff.norm();
		float ri = radius;
		Vector3<float> tiw = Vector3<float>(-niw.y(), niw.x(), 0);
		float vt = dot(actual_velocity, tiw);

		//Calculate b (collision distance)
		Vector3<float> future_diff = diff + velocity.norm() * (float)time*edge;

		float b = distance;

		// Calculate repulsive force
		Vector3<float> repulsive_force = A * exp(-b / B) * ((distance + future_diff.norm()) / (2 * b)) * (0.5f) * ((diff / diff.norm()) + (future_diff / future_diff.norm()));
		force = force + Weight(Vector3<float>(0,0,0) - repulsive_force) * repulsive_force;
	} //Obstacle
	//attractive
	for (int i = 0; i < all_p.size(); i++) {
		if (friend_number == NULL)
			break;
		if (friend_number != all_p[i])
			continue;

		if (all_p[i] != this) {
			Vector3<float> diff = all_p[i]->position - position;
			float rij = all_p[i]->radius + radius;

			float distance = diff.norm();
			if (distance  > 5)
				continue;
			Vector3<float> nij = diff / diff.norm();
			Vector3<float> tij = Vector3<float>(-nij.y(), nij.x(), 0);
			Vector3<float> deltaV = actual_velocity;
			float vt = dot(deltaV, tij);

			Vector3<float> future_diff = diff + velocity.norm() * (float)time * edge;
			float b = future_diff.norm();
			Vector3<float> repulsive_force = Ai * exp(-b / Bi) * ((distance + future_diff.norm()) / (2 * b)) * (0.5f) * ((diff / diff.norm()) + (future_diff / future_diff.norm()));
			
			
			if (direction == all_p[i]->direction && friend_type == 'p' && (dot(desired_direction, diff)  < 0 && distance >= 1.8f * 2 * radius)) {
				actual_velocity = desired_direction;
			}
			if (distance <= 2 * radius + 5 )
				force = force + repulsive_force;
		}//pedestrian
		
	}
	
	for (int i = 0; i < attractive_wall.size(); i++) {
		Vector3<float>dir = attractive_wall[i]->point2 - attractive_wall[i]->point1;
		Vector3<float> p_g = position - attractive_wall[i]->point1;
		Vector3<float> close = attractive_wall[i]->point1 + ClosePoint(dir, p_g) * dir;
		Vector3<float> diff = close - position;
		float distance = diff.norm();
		if (distance < 5 && attractive_time>=0.0001f) {
			Vector3<float> niw = diff / diff.norm();
			float ri = radius;
			Vector3<float> tiw = Vector3<float>(-niw.y(), niw.x(), 0);
			float vt = dot(actual_velocity, tiw);

			Vector3<float> future_diff = diff + velocity.norm() * (float)time * edge;

			float b = distance;

			Vector3<float> repulsive_force = A * exp(-b / attractive_time) * ((distance + future_diff.norm()) / (2 * b)) * (0.5f) * ((diff / diff.norm()) + (future_diff / future_diff.norm()));
			force = force + Weight(Vector3<float>(0, 0, 0) - repulsive_force) * repulsive_force;
			if (distance < radius + 0.5f) {
				actual_velocity = Vector3<float>(0, 0, 0);
				attractive_time -= time;
				if (attractive_time < 0.0001f)
					attractive_time = 0.0001f;
			}
		}
	}
	
	return false;
}
void Pedestrian::ApplyForce(double time) {
	
	actual_velocity = actual_velocity + force * (float)time;
	float g = 1;

	if (actual_velocity.norm() > max_speed)
	{

		actual_velocity = (actual_velocity / actual_velocity.norm()) * max_speed;
	}

	position = position + actual_velocity * (float)time;

}
bool Pedestrian::Comput_Goal_Distance() {
	Vector3<float>dir=Goal[Goal_Number].point2 - Goal[Goal_Number].point1;
	Vector3<float> p_g = position - Goal[Goal_Number].point1;
	return ((p_g - (dot(p_g, dir) / dot(dir, dir)) * dir).norm()<0.05f && dot(dir,p_g)>=0 && dot(Goal[Goal_Number].point1 - Goal[Goal_Number].point2, position - Goal[Goal_Number].point2) >= 0);
}
float Pedestrian::ClosePoint(Vector3<float>dir, Vector3<float> p_g) {
	
	float t = dot(p_g, dir) / dot(dir, dir);
	
	if (t < 0)
		t = 0;
	if (t > 1)
		t = 1;
	
	return t ;
}
float Pedestrian::isNegative(float dis) {

	return dis >= 0 ? dis : 0;
}
float Pedestrian::CalculateRepulsiveForce(float rij, float distance)
{
	return Ai * exp((rij - distance) / Bi) + K * isNegative(rij - distance);
	
}
float Pedestrian::Weight(Vector3<float> Force) {
	if (dot(edge, force) >= force.norm() * cos(100 * 3.14159 / 180))
		return 1;
	else
		return 1;
}
