#include "./header/Graphics.hpp"
Graphics::Graphics(){
    cout << "Graphics.cpp" << endl;
    
    make_circle(); 
    make_wall();
}
void Graphics::make_wall(){


}
void Graphics::make_circle(){
    // Create a circle
    vector<float> vertices;
	float aspect = 2000 / 1000; 
    glm::vec3 pos(0,0,0);
    int segement = 36;
    float radius = 0.2f;
	vertices.push_back(pos.x);
	vertices.push_back(pos.y);
	vertices.push_back(pos.z);
	for (int i = 0; i <= segement; i++) {
		float theta = 2.0f * 3.1415926f * float(i) / float(segement);

		float x = radius * cosf(theta);
		float y = radius * sinf(theta);
		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(0.0f);
	}

    unsigned int VBO;

    glGenVertexArrays(1, &this->circleVAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(this->circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    circle_vertexCnt = vertices.size()/3;
    cout << "circle_vertexCnt: " << circle_vertexCnt << endl;
    vertices.clear();
    // glBindVertexArray(0);
    // glDeleteBuffers(1, &VBO);
}

void Graphics::draw_circle(){
    glBindVertexArray(this->circleVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, circle_vertexCnt);
    // glBindVertexArray(0);
}

// void Graphics::change_color(int r, int g, int b){
//     this->color = glm::vec3(r/255.0f,g/255.0f,b/255.0f);
//     glUniform3fv(glGetUniformLocation(shader->ID, "co"), 1, glm::value_ptr(this->color));
// }