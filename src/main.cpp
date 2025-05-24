#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <bits/stdc++.h>

#include "./header/Shader.h"
#include "./header/Object.h"
#include "./header/Graphics.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "./header/camera.h"
#include "./header/Pedestrian.h"
#include "./header/Wall.h"
#include "./header/Control.h"
#include "./header/Vec3.h"

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
int mode = Scenario::NORMAL;
int friend_mode = FriendType::WITHOUT_FRIEND; // 是否有朋友
bool simu = false;


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
    // if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
    //     mode = (mode + 1) % 2;
    //     Reset();
    // }
}
void All_Clear() {
    p_right.clear();
    p_left.clear();
    all_p.clear();
    wall.clear();
    vertices.clear();
    VBO_Wall.clear();
}
void Reset() {
    All_Clear();
    cout << "Reset" << endl;
    num = 0;
    for (int j = 0; j < p_width; j++) {
        for (int i = 0; i < p_height; i++) {
            if ((8 * j + i + 1) > maxp)
                break;
            // unsigned int tempVBO;
            // glGenBuffers(1, &tempVBO);
            Pedestrian* tempp;
            int index = j * p_height + i;
            tempp = new Pedestrian(0.2, Vector3<float>(-15 - 1.5 * j, -5 + 1.5 * i, 0), 'r', num/*, tempVBO*/);
            p_right.push_back(tempp);
            all_p.push_back(p_right[index]);

            // 中間的通道
            if (mode == Scenario::BOTTLENECK)
                p_right[index]->Goal.push_back(Goal(Vector3<float>(0, 0.85, 0), Vector3<float>(0, -0.85, 0)));

            p_right[index]->Goal.push_back(Goal(Vector3<float>(22, 9, 0), Vector3<float>(22, -9, 0)));

            num++;
            tempp = new Pedestrian(0.2, Vector3<float>(15 + 1.5 * j, -5 + 1.5 * i, 0), 'l', num/*, tempVBO*/);
            p_left.push_back(tempp);
            all_p.push_back(p_left[index]);

            if (mode == Scenario::BOTTLENECK)
                p_left[index]->Goal.push_back(Goal(Vector3<float>(0, 0.85, 0), Vector3<float>(0, -0.85, 0)));
            
            p_left[index]->Goal.push_back(Goal(Vector3<float>(-22, 9, 0), Vector3<float>(-22, -9, 0)));
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

    const char* modes[] = {
        "Normal",
        "Bottleneck",
        "Attraction-based",
    };

    static int current_mode = Scenario::NORMAL;
    // static bool with_friend = false;

    // Combo 選單
    ImGui::Text("Select Scenario:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    if (ImGui::Combo("##Select Scenario", &current_mode, modes, IM_ARRAYSIZE(modes))) {
        if (current_mode != mode) {
            mode = current_mode;
            simu = false;
            Reset();
        }
    }

    if (ImGui::Button(friend_mode ? "Enable 'with friend'" : "Disable 'with friend'")) {
        friend_mode ^= 1;
        simu = false;
        Reset();
    }

    // 顯示目前的組合模式
    ImGui::Text("Current Mode: %s Scenario %s",
        modes[current_mode],
        friend_mode == FriendType::WITHOUT_FRIEND ? "without friend" : "with friend");
        
    
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
        if(p_right[j]->friend_number!=NULL || p_right[j]->attractive_wall.size()>0)
            color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        else
            color = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f);
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, p_right[j]->get_position());
        
        shader->set_uniform("color", color);
        shader->set_uniform("model", model);
        graphics->draw_circle();
    }

    for (int j = 0; j < p_left.size(); j++) {
        if (p_left[j]->friend_number != NULL || p_left[j]->attractive_wall.size() > 0)
            color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        else
            color = glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, p_left[j]->get_position());

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
        int deleteindex;
        if (simu) {
            deleteindex = ControllPedestrian(p_right, all_p, wall, deltaTime);
            deleteindex = ControllPedestrian(p_left, all_p, wall, deltaTime);
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
