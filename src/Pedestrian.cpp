#include "./header/Pedestrian.h"
#include <random>
#include <math.h>
#include <algorithm>
# define M_PI 3.14159265358979323846

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
		else{
			live = false;
			return true;
		}
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
		this -> edge = actual_velocity / actual_velocity.norm();
	Vector3<float> dir = Goal[Goal_Number].point2 - Goal[Goal_Number].point1;
	Vector3<float> p_g = position - Goal[Goal_Number].point1;
	Vector3<float> close = Goal[Goal_Number].point1 + ClosePoint(dir, p_g) * dir;

	// === 1 === //
	// Social force model for pedestrian dynamics p4. (1) vec_{e_alpha}(t)
	desired_direction = (close - position) / (close - position).norm();
	
	// Social force model for pedestrian dynamics p4. (2) vec_{F}
	force = force + ((desired_direction * desired_speed - actual_velocity) / relaxation_time);//point1

	// === 2 === //
	for (int i = 0; i < all_p.size(); i++) {
		if(all_p[i] == this) continue;
		if(all_p[i]->is_live() == false) continue;
		if(all_p[i]->is_fall() == true) continue; // 跌倒直接踩過去w

		Vector3<float> diff = position - all_p[i]->position;
		Vector3<float> rel_vab = all_p[i]->velocity - velocity;

		float distance = diff.norm();

		// Specification of a Microscopic Pedestrian Model by Evolutionary p8. (12) 2b
		// Calculate b (collision distance) 
		// b 可以想成 "兩人快要撞上了嗎" 的衡量程度, b 越小表示他們 "即將撞上"
		Vector3<float> future_diff = diff - rel_vab * (float)time;
		float fab = pow((distance + future_diff.norm()),2)  - dot((rel_vab * (float)time), (rel_vab * (float)time));
		float b = 0.5 * std::sqrt(fab);

		// Specification of a Microscopic Pedestrian Model by Evolutionary p7. (11) vec_{g_ab}
		// 他這邊把公式的 vec_{e_b} 換成 vec_{v_a} - vec_{v_b}
		// Calculate repulsive force
		Vector3<float> repulsive_force = A * exp(-b / B) * ((distance + future_diff.norm()) / (2 * b)) * (0.5f) * ((diff / diff.norm()) + (future_diff / future_diff.norm()));
		if (friend_number != NULL && friend_number == all_p[i])
			force = force + Weight(Vector3f(0,0,0) - repulsive_force) * repulsive_force * 0.5f;
		else
			force = force + Weight(Vector3f(0,0,0) - repulsive_force) * repulsive_force;
		
		
	}//pedestrian
	
	// === 3 === //
	for (int i = 0; i < wall.size(); i++)
	{
		Vector3<float> dir = wall[i]->point2 - wall[i]->point1;
		Vector3<float> p_g = position - wall[i]->point1;
		Vector3<float> close = wall[i]->point1 + ClosePoint(dir, p_g) * dir;
		Vector3<float> diff = position - close;
		float distance = diff.norm();
		
		// 和 === 2 === 類似，把 b 想成人和牆壁的最近距離, future_diff 是下一個時間點兩人的距離
		// Calculate b (collision distance)
		// Vector3<float> future_diff = diff + velocity.norm() * (float)time*edge;
		Vector3<float> future_diff = diff + velocity * (float)time;
		float b = distance;

		// Calculate repulsive force
		Vector3<float> repulsive_force = A * exp(-b / B) * ((distance + future_diff.norm()) / (2 * b)) * (0.5f) * ((diff / diff.norm()) + (future_diff / future_diff.norm()));
		force = force + Weight(Vector3f(0,0,0) - repulsive_force) * repulsive_force;
	} //Obstacle
	//attractive
	for (int i = 0; i < all_p.size(); i++) {
		if(all_p[i]->is_live() == false) continue;
		
		// 沒朋友
		if (friend_number == NULL)
			break;
		if (friend_number != all_p[i])
			continue;

		// 指向朋友的方向 -> 吸引力, 希望往朋友靠近
		Vector3<float> diff = all_p[i]->position - position;

		float distance = diff.norm();
		if (distance  > 5)
			continue;

		Vector3<float> future_diff = diff + velocity * (float)time;
		float b = future_diff.norm();
		Vector3<float> repulsive_force = Ai * exp(-b / Bi) * ((distance + future_diff.norm()) / (2 * b)) * (0.5f) * ((diff / diff.norm()) + (future_diff / future_diff.norm()));
		
		// 和朋友的前進方向一致(都往右or都往左), 並且朋友在前方 -> 不用特別加速, 就跟著朋友走就好
		if (direction == all_p[i]->direction && friend_type == 'p' && (dot(desired_direction, diff)  < 0 && distance >= 1.8f * 2 * radius)) {
			actual_velocity = desired_direction;
		}
		// 離朋友太遠 -> 施加吸引力
		if (distance <= 2 * radius + 5 )
			force = force + repulsive_force;
	}//pedestrian
	
	for (int i = 0; i < attractive_wall.size(); i++) {
		Vector3<float> dir = attractive_wall[i]->point2 - attractive_wall[i]->point1;
		Vector3<float> p_g = position - attractive_wall[i]->point1;
		Vector3<float> close = attractive_wall[i]->point1 + ClosePoint(dir, p_g) * dir;
		Vector3<float> diff = close - position;
		float distance = diff.norm();
		if (distance < 5 && attractive_time>=0.0001f) {

			// Vector3<float> future_diff = diff + velocity.norm() * (float)time * edge;
			Vector3<float> future_diff = diff + velocity * (float)time;

			float b = distance;

			Vector3<float> repulsive_force = A * exp(-b / attractive_time) * ((distance + future_diff.norm()) / (2 * b)) * (0.5f) * ((diff / diff.norm()) + (future_diff / future_diff.norm()));
			force = force + Weight(repulsive_force) * repulsive_force;
			if (distance < radius + 0.5f) {
				// 朋友到了嗎
				if(friend_number != nullptr){
					p_g = friend_number->position - attractive_wall[i]->point1;
					close = attractive_wall[i]->point1 + ClosePoint(dir, p_g) * dir;
					diff = close - friend_number->position;
					distance = diff.norm();
					if(distance < radius + 0.5f){
						actual_velocity = Vector3<float>(0, 0, 0);
						attractive_time -= time;
					}else{
						float norm_val = actual_velocity.norm();
						actual_velocity = Vector3f(actual_velocity.x() / norm_val, 
													actual_velocity.y() / norm_val,
													actual_velocity.z() / norm_val);
					}
				}
				// 停留
				else{
					actual_velocity = Vector3<float>(0, 0, 0);
					attractive_time -= time;
				}
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
	velocity = actual_velocity;
}

bool Pedestrian::Comput_Goal_Distance() {
	Vector3<float>dir=Goal[Goal_Number].point2 - Goal[Goal_Number].point1;
	Vector3<float> p_g = position - Goal[Goal_Number].point1;
	return ((p_g - (dot(p_g, dir) / dot(dir, dir)) * dir).norm()<0.05f && dot(dir,p_g)>=0 && dot(Goal[Goal_Number].point1 - Goal[Goal_Number].point2, position - Goal[Goal_Number].point2) >= 0);
}

float Pedestrian::ClosePoint(Vector3<float>dir, Vector3<float> p_g) {
	
	float t = dot(p_g, dir) / dot(dir, dir);
	
	if (t < 0) t = 0;
	else if (t > 1) t = 1;
	
	return t ;
}

float Pedestrian::Weight(Vector3<float> repulsive_force) {
	// Social force model for pedestrian dynamics p6. (7) w_{vec_{e}, vec_{f}}
	// 期望去的方向和"施加的力的方向的反向(a->b)"角度, 越正面權重越大
	// 可以想成, 如果今天施力者在你的前方, 那你會抵抗它 (front_repulsion_weight_factor = 1.0f)
	// 如果施力者在你的後方, 那你會不太抵抗它 (rear_repulsion_weight_factor = 0.8f)
	// a --> 
	//   \ 
	//    b
	// desired_direction: 行人期望移動的單位方向向量 (e)
	// repulsive_force: 施加在行人身上的排斥力向量 (f)
	// threshold_angle_degrees: 100 度，排斥力權重的角度閾值

	// 檢查排斥力是否為零向量，避免除以零
	if (repulsive_force.norm() < 0.0001f) {
		return 1.0f; // 如果沒有排斥力，權重為1
	}

	// 計算期望方向和排斥力之間的餘弦值
	float cos_angle = dot(desired_direction, repulsive_force) / repulsive_force.norm();

	// 由於浮點數精度問題，cos_angle 可能會略微超出 [-1, 1] 範圍，需要限制
	cos_angle = std::max(-1.0f, std::min(1.0f, cos_angle));

	// 計算實際夾角（弧度）
	float angle_radians = acos(cos_angle);

	// 將閾值角度轉換為弧度
	const float THRESHOLD_ANGLE_RADIANS = 100.0f * M_PI / 180.0f; // M_PI 是 PI 的常數，通常定義在 <cmath> 或 <math.h>

	// 如果夾角小於或等於閾值角度 (100度)，則權重為 1
	if (angle_radians <= THRESHOLD_ANGLE_RADIANS) {
		return front_repulsion_weight_factor;
	} else {
		// 對後方行人的敏感度 (應該可以改值當作報告的一部分)
		// 可能可以加個跌倒變成紅色 模擬踩踏事件
		return rear_repulsion_weight_factor;
	}
	// if (dot(desired_direction, repulsive_force) >= repulsive_force.norm() * cos(100 * 3.14159 / 180))
	// 	return 1;
	// else
	// 	return 1;
}
