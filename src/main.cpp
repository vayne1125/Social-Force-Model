#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <bits/stdc++.h>

#include "./header/Shader.h"
#include "./header/Object.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "./header/camera.h"
#include "./header/Pedestrian.h"
#include "./header/Wall.h"
#include "./header/Control.h"
#include "./header/Vec3.h"



const char* vertexShaderSource =  "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 projection;\n"
    "uniform vec4 co;\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = projection * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "    color=co;\n"
    "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec4 color;\n"
"void main()\n"
"{\n"
"FragColor =color;\n"
"}\0";



std::vector<Pedestrian*> p_right;
std::vector<Pedestrian*> p_left;
std::vector<Pedestrian*> all_p;
std::vector<Wall* > wall;
int p_height = 8;
int p_width = 4;
int max_height = 8;
int max_width = 4;
int maxp = 8 * (max_width - 1) + max_height;

int num = 0;
int mode = 0;
bool simu = false;
std::vector<float> vertices;
std::vector<unsigned int> VBO_Wall;
std::vector<unsigned int> VBO_Goal_For_Right;
std::vector<unsigned int> VBO_Goal_For_Left;
void Reset();
void framebuffer_size_callback(GLFWwindow* window, int width, int heigth) {
    glViewport(0, 0, width, heigth);
    
}
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        Reset();
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        mode = (mode + 1) % 2;
        Reset();
    }
}
void All_Clear() {
    p_right.clear();
    p_left.clear();
    all_p.clear();
    wall.clear();
    vertices.clear();
    VBO_Wall.clear();
    VBO_Goal_For_Left.clear();
    VBO_Goal_For_Right.clear();
}
void Reset() {
    All_Clear();
    num = 0;
    for (int j = 0; j < p_width; j++) {
        for (int i = 0; i < p_height; i++) {
            if ((8 * j + i + 1) > maxp)
                break;
            unsigned int tempVBO;
            Pedestrian* tempp;
            int index = j * p_height + i;
            glGenBuffers(1, &tempVBO);
            tempp = new Pedestrian(0.2, Vector3<float>(-15 - 1.5 * j, -5 + 1.5 * i, 0), 'r', num, tempVBO);
            p_right.push_back(tempp);
            all_p.push_back(p_right[index]);

            if (mode == 0)
                p_right[index]->Goal.push_back(Goal(Vector3<float>(0, 0.85, 0), Vector3<float>(0, -0.85, 0)));

            p_right[index]->Goal.push_back(Goal(Vector3<float>(22, 9, 0), Vector3<float>(22, -9, 0)));
            vertices = p_right[index]->DrawCircle();

            glBindBuffer(GL_ARRAY_BUFFER, p_right[index]->VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            num++;
            glGenBuffers(1, &tempVBO);
            tempp = new Pedestrian(0.2, Vector3<float>(15 + 1.5 * j, -5 + 1.5 * i, 0), 'l', num, tempVBO);
            p_left.push_back(tempp);
            all_p.push_back(p_left[index]);

            if (mode == 0)
                p_left[index]->Goal.push_back(Goal(Vector3<float>(0, 0.85, 0), Vector3<float>(0, -0.85, 0)));
            else {
                if (num == 1)
                    all_p[num]->friend_number = 0;
            }
            p_left[index]->Goal.push_back(Goal(Vector3<float>(-22, 9, 0), Vector3<float>(-22, -9, 0)));

            vertices = p_left[index]->DrawCircle();

            glBindBuffer(GL_ARRAY_BUFFER, p_right[index]->VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            num++;
            
        }
    }// pedestrian

    for (float i = 0; i < 2; i++) {

        unsigned int tempVBO;

        wall.push_back(new Wall(Vector3<float>(-22, 9 - 18 * i, 0), Vector3<float>(22, 9 - 18 * i, 0)));
        glGenBuffers(1, &tempVBO);

        VBO_Wall.push_back(tempVBO);

        vertices = wall[i]->Spawn();

        glBindBuffer(GL_ARRAY_BUFFER, VBO_Wall[i]);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    } //wall

    if (mode == 0) {
        for (float i = 0; i < 2; i++) {

            unsigned int tempVBO;
            wall.push_back(new Wall(Vector3<float>(0, -1 + 10 * i, 0), Vector3<float>(0, -9 + 10 * i, 0)));
            glGenBuffers(1, &tempVBO);

            VBO_Wall.push_back(tempVBO);

            vertices = wall[i]->Spawn();
            for (int j = 0; j < vertices.size(); j++)
                glBindBuffer(GL_ARRAY_BUFFER, VBO_Wall[i]);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
        }
    }
    if (mode == 1) {
        unsigned int tempVBO;
        wall.push_back(new Wall(Vector3<float>(-2, -9, 0), Vector3<float>(2, -9 , 0)));
        glGenBuffers(1, &tempVBO);

        VBO_Wall.push_back(tempVBO);
        std::cout << wall.size() << std::endl;
        all_p[0]->attractive_wall.push_back(wall[wall.size()-1]);
        vertices = wall[0]->Spawn();
        for (int j = 0; j < vertices.size(); j++)
            glBindBuffer(GL_ARRAY_BUFFER, VBO_Wall[0]);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }
}

int main(void)
{
    int SCC_HIGHT = 800;
    int SCC_WEIGHT = 1500;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCC_WEIGHT, SCC_HIGHT, "LearnOpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, SCC_WEIGHT, SCC_HIGHT);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 330";
    ImGui_ImplOpenGL3_Init(glsl_version);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    
    double lastTime = glfwGetTime();
    double deltaTime = 0.0;
    unsigned int vertexShader;
    unsigned int fragmentShader;
    unsigned int fragmentShader2;
    unsigned int fragmentShader3;
    unsigned int shaderProgram;
    unsigned int shaderProgram2;
    unsigned int shaderProgram3;
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
  
    Reset();
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
   
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    
    while (!glfwWindowShouldClose(window)) {

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Controller");
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        if (ImGui::Button("simulate")) 
            simu = true;
        
        if (ImGui::Button("Change mode")) {
            simu = false;
            mode =(mode + 1) % 2;
            Reset();
        }
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

        double currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = (float)width / (float)height;
        float orthoSize = 11.0f;


        float left = -orthoSize * aspect;
        float right = orthoSize * aspect;
        float bottom = -orthoSize;
        float top = orthoSize;
        float near = -1.0f;
        float far = 1.0f;

        float projection[16] = {
            2.0f / (right - left), 0.0f, 0.0f, -(right + left) / (right - left),
            0.0f, 2.0f / (top - bottom), 0.0f, -(top + bottom) / (top - bottom),
            0.0f, 0.0f, -2.0f / (far - near), -(far + near) / (far - near),
            0.0f, 0.0f, 0.0f, 1.0f
        };


        int projectionLoc;
        int colorLoc;

        for (int j = 0; j < p_right.size(); j++) {
            glUseProgram(shaderProgram);
            projectionLoc = glGetUniformLocation(shaderProgram, "projection");
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection);
            colorLoc = glGetUniformLocation(shaderProgram, "co");
            if(p_right[j]->friend_number!=NULL || p_right[j]->attractive_wall.size()>0)
                glUniform4f(colorLoc, 0.0f, 0.0f, 1.0f, 1.0f);
            else
                glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
            vertices = p_right[j]->DrawCircle();
            glBindBuffer(GL_ARRAY_BUFFER, p_right[j]->VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 102);

        }
        for (int j = 0; j < p_left.size(); j++) {
            glUseProgram(shaderProgram);
            projectionLoc = glGetUniformLocation(shaderProgram, "projection");
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection);
            colorLoc = glGetUniformLocation(shaderProgram, "co");
            if (p_left[j]->friend_number != NULL || p_left[j]->attractive_wall.size() > 0)
                glUniform4f(colorLoc, 0.0f, 0.0f, 1.0f, 1.0f);
            else
                glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f);
            vertices = p_left[j]->DrawCircle();
            glBindBuffer(GL_ARRAY_BUFFER, p_left[j]->VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 102);
        }

        glUseProgram(shaderProgram);
        projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection);
        colorLoc = glGetUniformLocation(shaderProgram, "co");
        
        for (int i = 0; i < wall.size(); i++) {
            if(mode==1 && i==wall.size()-1)
                glUniform4f(colorLoc, 1.0f, 1.0f, 0.0f, 1.0f);
            else
                glUniform4f(colorLoc, 0.5f, 0.5f, 0.5f, 1.0f);
            std::vector<float> vertices = wall[i]->Spawn();
            glBindBuffer(GL_ARRAY_BUFFER, VBO_Wall[i]);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glLineWidth(50);
            glDrawArrays(GL_LINES, 0, 24);
        }
        int deleteindex;
        if (simu) {
            deleteindex = ControllPedestrian(p_right, all_p, wall, deltaTime);
            deleteindex = ControllPedestrian(p_left, all_p, wall, deltaTime);
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        processInput(window);
        glfwPollEvents();
    }


    return 0;
    glfwTerminate();

}
