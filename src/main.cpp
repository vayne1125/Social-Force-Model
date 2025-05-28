#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <bits/stdc++.h>

#include "./header/Shader.h"
#include "./header/Graphics.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "./header/Pedestrian.h"
#include "./header/Wall.h"
#include "./header/Control.h"
#include "./header/Vec3.h"

#define _mn first
#define _mx second

int SCR_HEIGHT = 800;
int SCR_WIDTH = 1500;

Shader *shader;

std::vector<Pedestrian*> p_right;
std::vector<Pedestrian*> p_left;
std::vector<Pedestrian*> all_p;
std::vector<Wall* > wall;
int p_height = 8;
int p_width = 4;
int max_height = 8;
int max_width = 4;
int maxp = 8 * (max_width - 1) + max_height;
float radius = 0.2f;

enum Scenario {
    NORMAL,
    BOTTLENECK,
    ATTRACTION
};
enum FriendType {
    WITHOUT_FRIEND,
    WITH_FRIEND
};

int num = 0;

float rear_repulsion_weight_factor = 0.8f; // 後方行人敏感度
float front_repulsion_weight_factor = 1.0f; // 前方行人敏感度

int mode = Scenario::NORMAL;
int friend_mode = FriendType::WITHOUT_FRIEND; // 是否有朋友
bool simu = false;

pair<float,float> r_desired_speed = {100, -1}; // mn, mx
pair<float,float> l_desired_speed = {100, -1}; // mn, mx

Graphics* graphics;
std::vector<float> vertices;
std::vector<unsigned int> VBO_Wall;
void Reset();
void framebuffer_size_callback(GLFWwindow* window, int width, int heigth) {
    glViewport(0, 0, width, heigth);
    
}
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        Reset();
}
void All_Clear() {
    // 釋放 p_right 中所有 Pedestrian 物件的記憶體
    for (Pedestrian* p : p_right) {
        if(p!=nullptr) delete p;
    }
    p_right.clear();

    // 釋放 p_left 中所有 Pedestrian 物件的記憶mory
    for (Pedestrian* p : p_left) {
        if(p!=nullptr) delete p;
    }
    p_left.clear();

    all_p.clear();

    // 釋放 wall 中所有 Wall 物件的記憶體
    for (Wall* w : wall) {
        delete w;
    }
    wall.clear();
    vertices.clear();
    VBO_Wall.clear();

    r_desired_speed = {100, -1}; // 重置右側行人的期望速度範圍
    l_desired_speed = {100, -1}; // 重置左側行人的期望速度範圍
}
void Reset() {
    All_Clear();
    cout << "Reset" << endl;
    simu = false;
    num = 0;
    for (int j = 0; j < p_width; j++) {
        for (int i = 0; i < p_height; i++) {
            if ((8 * j + i + 1) > maxp)
                break;
            // unsigned int tempVBO;
            // glGenBuffers(1, &tempVBO);
            Pedestrian* tempp;
            int index = j * p_height + i;
            tempp = new Pedestrian(radius, Vector3<float>(-15 - 1.5 * j, -5 + 1.5 * i, 0), 'r', num/*, tempVBO*/);
            tempp->set_rear_repulsion_weight_factor(rear_repulsion_weight_factor);
            tempp->set_front_repulsion_weight_factor(front_repulsion_weight_factor);
            p_right.push_back(tempp);
            all_p.push_back(p_right[index]);
            
            // 中間的通道
            if (mode == Scenario::BOTTLENECK)
                p_right[index]->Goal.push_back(Goal(Vector3<float>(0, 0.85, 0), Vector3<float>(0, -0.85, 0)));

            p_right[index]->Goal.push_back(Goal(Vector3<float>(22, 9, 0), Vector3<float>(22, -9, 0)));
            r_desired_speed._mn = std::min(r_desired_speed._mn, p_right[index]->get_desired_speed());
            r_desired_speed._mx = std::max(r_desired_speed._mx, p_right[index]->get_desired_speed());

            num++;
            tempp = new Pedestrian(radius, Vector3<float>(15 + 1.5 * j, -5 + 1.5 * i, 0), 'l', num/*, tempVBO*/);
            tempp->set_rear_repulsion_weight_factor(rear_repulsion_weight_factor);
            tempp->set_front_repulsion_weight_factor(front_repulsion_weight_factor);
            p_left.push_back(tempp);
            all_p.push_back(p_left[index]);

            if (mode == Scenario::BOTTLENECK)
                p_left[index]->Goal.push_back(Goal(Vector3<float>(0, 0.85, 0), Vector3<float>(0, -0.85, 0)));
            
            p_left[index]->Goal.push_back(Goal(Vector3<float>(-22, 9, 0), Vector3<float>(-22, -9, 0)));
            l_desired_speed._mn = std::min(l_desired_speed._mn, p_left[index]->get_desired_speed());
            l_desired_speed._mx = std::max(l_desired_speed._mx, p_left[index]->get_desired_speed());
            num++;
        }
    }// pedestrian

    for (float i = 0; i < 2; i++) {
        wall.push_back(new Wall(Vector3<float>(-22, 9 - 18 * i, 0), Vector3<float>(22, 9 - 18 * i, 0)));
    } //wall

    if (mode == Scenario::BOTTLENECK) {
        for (float i = 0; i < 2; i++) {
            wall.push_back(new Wall(Vector3<float>(0, -1 + 10 * i, 0), Vector3<float>(0, -9 + 10 * i, 0)));
        }
    }
    if (mode == Scenario::ATTRACTION) {
        wall.push_back(new Wall(Vector3<float>(-2, -9, 0), Vector3<float>(2, -9 , 0)));
        all_p[0]->attractive_wall.push_back(wall[wall.size()-1]);
        if(friend_mode == FriendType::WITH_FRIEND) 
            p_right[1]->attractive_wall.push_back(wall[wall.size()-1]);

    }
    if(friend_mode == FriendType::WITH_FRIEND) {
        p_right[0]->friend_number = p_right[1];
        p_right[1]->friend_number = p_right[0];
        float f_v = p_right[0]->get_desired_speed();
        f_v += p_right[1]->get_desired_speed();
        f_v /= 2.0f;
        p_right[0]->set_desired_speed(1.05 * f_v);
        p_right[1]->set_desired_speed(0.98 * f_v);
    }
}
void init(){
    // Initialize the shader program
    string SHADER_DIR = "../../src/shaders/";
    string v = SHADER_DIR + "easy.vert";
    string f = SHADER_DIR + "easy.frag";
    shader = new Shader(v.c_str(),f.c_str());
    
    graphics = new Graphics();

    Reset();
}
void render_GUI(){
    ImGui::NewFrame();
    ImGui::Begin("Controller");
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    if (ImGui::Button("simulate")) 
        simu = true;
    ImGui::SameLine();
    if (ImGui::Button("reset")) 
        Reset();
    ImGui::SameLine();
    ImGui::Text("Avg. speed: %0.3f", getAvgSpeed(all_p));
    const char* modes[] = {
        "Normal",
        "Bottleneck",
        "Attraction-based",
    };

    static int current_mode = Scenario::NORMAL;

    // Combo 選單
    ImGui::Text("Select Scenario:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    if (ImGui::Combo("##Select Scenario", &current_mode, modes, IM_ARRAYSIZE(modes))) {
        if (current_mode != mode) {
            mode = current_mode;
            Reset();
        }
    }

    if (ImGui::Button(friend_mode ? "Enable 'with friend'" : "Disable 'with friend'")) {
        friend_mode ^= 1;
        Reset();
    }

    // 顯示目前的組合模式
    ImGui::Text("Current Mode: %s Scenario %s",
        modes[current_mode],
        friend_mode == FriendType::WITHOUT_FRIEND ? "without friend" : "with friend");
    
    ImGui::NewLine();
    ImGui::Text("Sensitivity to forces from front (0.0 = no reaction, 1.0 = full reaction):");
    ImGui::SetNextItemWidth(150);
    if(ImGui::SliderFloat("##Rear Repulsion Weight (front)", &front_repulsion_weight_factor, 0.0f, 1.0f, "%.2f")){
        for(auto& p : all_p) {
            if (p->is_live()) {
                p->set_front_repulsion_weight_factor(front_repulsion_weight_factor);
            }
        }
    }

    ImGui::Text("Sensitivity to forces from behind (0.0 = no reaction, 1.0 = full reaction):");
    ImGui::SetNextItemWidth(150);
    if(ImGui::SliderFloat("##Rear Repulsion Weight (behind)", &rear_repulsion_weight_factor, 0.0f, 1.0f, "%.2f")){
        for(auto& p : all_p) {
            if (p->is_live()) {
                p->set_rear_repulsion_weight_factor(rear_repulsion_weight_factor);
            }
        }
    }
    
    ImGui::NewLine();
    ImGui::Text("People number:");
    ImGui::SameLine();
    if (ImGui::Button("-")) {
        max_height--;
        if (max_height < 1) {
            max_height = 8;
            max_width --;
        }
        if (max_width < 1) {
            max_width = 1;
            max_height = 1;
        }
        maxp = 8 * (max_width - 1) + max_height;
        Reset();
    }
    ImGui::SameLine();
    if (ImGui::Button("+")) {
        max_height++;
        if (max_height >8) {
            max_height = 1;
            max_width++;
        }
        if (max_width > 4) {
            max_width = 4;
            max_height = 8;
        }
        maxp = 8 * (max_width - 1) + max_height;
        Reset();
        }
        ImGui::End();
}
void render_scene() {
    float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
    float orthoSize = 11.0f;

    float left = -orthoSize * aspect;
    float right = orthoSize * aspect;
    float bottom = -orthoSize;
    float top = orthoSize;
    float near = -1.0f;
    float far = 1.0f;

    glm::mat4 projection = glm::ortho(left, right, bottom, top, near, far);
    shader->set_uniform("projection", projection);

    glm::vec4 color;
    for (int j = 0; j < p_right.size(); j++) {
        if(p_right[j]->is_live() == false) continue;
        if(p_right[j]->friend_number!=NULL || p_right[j]->attractive_wall.size()>0)
            color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        else
            color = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f);

        if(p_right[j]->is_fall()) {
            color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        }else{
            float v = p_right[j]->get_desired_speed();
            float speed_normalized = (v - (r_desired_speed._mn) ) / (r_desired_speed._mx - (r_desired_speed._mn)) + 0.3f;
            color = (float)speed_normalized * color;
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, p_right[j]->get_position());
        model = glm::scale(model, glm::vec3(radius, radius, radius));
        
        shader->set_uniform("color", color);
        shader->set_uniform("model", model);
        graphics->draw_circle();
    }

    for (int j = 0; j < p_left.size(); j++) {
        if(p_left[j]->is_live() == false) continue;
        if (p_left[j]->friend_number != NULL || p_left[j]->attractive_wall.size() > 0)
            color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        else
            color = glm::vec4( 0.0f, 1.0f, 1.0f, 1.0f);
        
        if(p_left[j]->is_fall()) {
            color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        }else{
            float v = p_left[j]->get_desired_speed();
            float speed_normalized = (v - (l_desired_speed._mn) ) / (l_desired_speed._mx - (l_desired_speed._mn)) + 0.3f;
            color = (float)speed_normalized * color;
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, p_left[j]->get_position());
        model = glm::scale(model, glm::vec3(radius, radius, radius));

        shader->set_uniform("color", color);
        shader->set_uniform("model", model);
        graphics->draw_circle();
    }

    
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    for (int i = 0; i < wall.size(); i++) {
        if(mode == Scenario::ATTRACTION && i==wall.size()-1)
            color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        else
            color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        shader->set_uniform("color", color);
        shader->set_uniform("model", glm::mat4(1.0f));

        std::vector<float> vertices = wall[i]->Spawn();
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glLineWidth(50);
        glDrawArrays(GL_LINES, 0, 24);
    }
}
int main(void)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 330";
    ImGui_ImplOpenGL3_Init(glsl_version);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    init();
    double lastTime = glfwGetTime();
    double deltaTime = 0.0;

    while (!glfwWindowShouldClose(window)) {
        
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        double currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        // RENDER
        shader->use();
        render_scene();
        
        // SIMULATION
        if (simu) {
            ControllPedestrian(p_right, all_p, wall, deltaTime);
            ControllPedestrian(p_left, all_p, wall, deltaTime);
            handleCollision(all_p, radius);
        }
        
        // IMGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        render_GUI();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        processInput(window);
        glfwPollEvents();
    }

    return 0;
    glfwTerminate();

}
