#include <iostream>
#include <math.h>
#include"Goal.h"
#include "Wall.h"
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#pragma once
class Pedestrian {
public:
	std::vector<float> vertices;
	  Pedestrian(float radius, Vector3<float> position,char d,int number) {
		  this->radius = radius;
		  segement = 100;
		  this->position = position;
		  this->direction = d;
		  desired_speed = generategaussian(mean, standard_deviation);
		  max_speed = 1.3 * desired_speed ;
		  this->number = number;
		  friend_number = nullptr;
		  prePosition.push_back(position);
	  }
	  std::vector<Goal> Goal;
	  std::vector<Vector3<float>> prePosition;
	  float generategaussian(float mean, float standard_deviation);
	  bool ComputeForce(std::vector<Pedestrian*>& p,std::vector<Pedestrian*>& all_p, std::vector<Wall* >& wall,float time);
	  void ApplyForce(double time);
	  int Goal_Number = 0;
	  int number;
	  Pedestrian* friend_number;
	  std::vector<Wall* >attractive_wall;
	  char friend_type = 'f';
	  glm::vec3 get_position() {
		  glm::vec3 rt;
		  rt.x = position.x();
		  rt.y = position.y();
		  rt.z = position.z();
		  return rt;
	  }
	  float get_desired_speed() {
		return desired_speed;
	  }
	  glm::vec3 get_current_velocity() {
		  glm::vec3 rt;
		  rt.x = actual_velocity.x();
		  rt.y = actual_velocity.y();
		  rt.z = actual_velocity.z();
		  return rt;
	  }
	  bool is_live() {
		  return live;
	  }
	  void set_rear_repulsion_weight_factor(float factor) {
		  rear_repulsion_weight_factor = factor;
	  }
	  void set_front_repulsion_weight_factor(float factor) {
		  front_repulsion_weight_factor = factor;
	  }
	  bool is_fall() {
		  return fall;
	  }
	  void set_fall() {
		  fall = true;
	  }
private:
	bool live = true;
	bool fall = false; // 是否摔倒
	bool destroy=false;
	char direction;
	bool Comput_Goal_Distance();
	// float CalculateRepulsiveForce(float rij, float distance);
	// float isNegative(float dis);
	float radius;
	float relaxation_time = 0.5f;
	float max_speed;
	float mean = 1.34f;
	float standard_deviation = 0.26f;
	float desired_speed = 0;

	// Specification of a Microscopic Pedestrian Model by Evolutionary p11, Table 1.
	float A = 25; // 力的強度
	float B = 1.07-0.55;
	float Ai = 3;
	float Bi = 5 - 0.55;
	float sigma = 0.3f;
	float attractive_time = 10;
	int segement;
	float rear_repulsion_weight_factor = 0.8f;
	float front_repulsion_weight_factor = 1.0f;
	Vector3<float>position;
	Vector3<float> actual_velocity = Vector3<float>(0, 0, 0);
	Vector3<float> edge;
	Vector3<float> force = Vector3<float>(0, 0, 0);
	Vector3<float> desired_direction;
	float ClosePoint(Vector3<float>dir, Vector3<float> p_g);
	float Weight(Vector3<float> Force);
	Vector3<float> velocity= Vector3<float>(0,0,0);
};
