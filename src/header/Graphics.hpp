#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bits/stdc++.h>
using namespace std;
class Graphics{
public:
    void draw_circle();
    Graphics();
private: 
    unsigned int circleVAO, wallVAO;
    int circle_vertexCnt, wall_vertexCnt;
    void make_circle();
};