#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

#include<chrono>

#include "tiny_obj_loader.h"

const unsigned int sirka = 800;
const unsigned int vyska = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height); //function prototype

static int KompilujShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        std::cout << "nepovedlo se zkompilovat shadery" << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

static unsigned int VytvorShader(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vShader = KompilujShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fShader = KompilujShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vShader);
    glDeleteShader(fShader);

    return program;
}

static std::string parseFile(const std::string filePath) {
    std::ifstream file(filePath);
    std::string str;
    std::string content;
    while (std::getline(file, str)) {
        content.append(str + "\n");
    }
    std::cout << content;
    return content;
}

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(sirka, vyska, "OpenGL", NULL, NULL);
    if (!window)
    {
        std::cout << "Nepovedlo se vytvorit okno!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Nepovedlo se nacist OpenGL function pointers!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Load OBJ model
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "models/colored_cube.obj");

    if (!warn.empty()) {
        std::cout << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        std::cerr << "Failed to load/parse .obj file." << std::endl;
        return -1;
    }

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    //animace otácení
    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                // Vertex position
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);

                // Vertex color
                if (!attrib.colors.empty()) {
                    vertices.push_back(attrib.colors[3 * idx.vertex_index + 0]);
                    vertices.push_back(attrib.colors[3 * idx.vertex_index + 1]);
                    vertices.push_back(attrib.colors[3 * idx.vertex_index + 2]);
                }
                else {
                    // Default color if not available
                    vertices.push_back(1.0f);
                    vertices.push_back(1.0f);
                    vertices.push_back(1.0f);
                }

                indices.push_back(index_offset + v);
            }
            index_offset += fv;
        }
    }

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    //atribut pozice
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //atribut barvy
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    std::string vShader = parseFile("shaders/vertex.shader");
    std::string fShader = parseFile("shaders/fragment.shader");

    unsigned int shader = VytvorShader(vShader, fShader);

    float rotation = 0.0f;
    double prevTime = glfwGetTime();

    glEnable(GL_DEPTH_TEST);

    int sirkaObr, vyskaObr, pocBarev;
    unsigned char* obrazek = stbi_load("Textury/textura.png", &sirkaObr, &vyskaObr, &pocBarev, 0);

    unsigned int textura;
    glGenTextures(1, &textura);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textura);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sirkaObr, vyskaObr, 0, GL_RGBA, GL_UNSIGNED_BYTE, obrazek);

    stbi_image_free(obrazek);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLuint tex0Uniform = glGetUniformLocation(shader, "tex0");
    glUniform1i(tex0Uniform, 0);

    const GLubyte* renderer = glGetString(GL_RENDERER); // Get renderer string
    const GLubyte* vendor = glGetString(GL_VENDOR);     // Get vendor string
    const GLubyte* version = glGetString(GL_VERSION);   // Version as a string

    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "Vendor: " << vendor << std::endl;
    std::cout << "OpenGL version supported: " << version << std::endl;

    //vypsat prezentacní mód
    int presentMode = glfwGetWindowAttrib(window, GLFW_DOUBLEBUFFER);
    if (presentMode == GLFW_TRUE) {
        std::cout << "Present mode: Double buffered (V-Sync enabled)" << std::endl;
    }
    else {
        std::cout << "Present mode: Single buffered (V-Sync disabled)" << std::endl;
    }

    /*
    //PREDCYKLUS
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    //glClearColor(0.0f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader);
    glBindTexture(GL_TEXTURE_2D, textura);

    double curTime = glfwGetTime();
    if (curTime - prevTime >= 1 / 60)
    {
        rotation += 0.1f;
        prevTime = curTime;
    }

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    //glm::mat4 model = glm::mat4(1.0f); // Identity matrix

    const float c3 = glm::cos(rotation / 80);
    const float s3 = glm::sin(rotation / 80);
    const float c2 = glm::cos(rotation / 80);
    const float s2 = glm::sin(rotation / 80);
    const float c1 = glm::cos(rotation / 80);
    const float s1 = glm::sin(rotation / 80);

    // Construct the rotation matrix based on your custom formula
    model[0][0] = c1 * c3 + s1 * s2 * s3;
    model[0][1] = c2 * s3;
    model[0][2] = c1 * s2 * s3 - c3 * s1;
    model[1][0] = c3 * s1 * s2 - c1 * s3;
    model[1][1] = c2 * c3;
    model[1][2] = c1 * c3 * s2 + s1 * s3;
    model[2][0] = c2 * s1;
    model[2][1] = -s2;
    model[2][2] = c1 * c2;
    //view = glm::translate(view, glm::vec3(0.0f, -0.5f, -2.0f));
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f));
    projection = glm::perspective(glm::radians(45.0f), (float)sirka / vyska, 0.1f, 100.0f);

    int modelLocation = glGetUniformLocation(shader, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
    int viewLocation = glGetUniformLocation(shader, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    int projectionLocation = glGetUniformLocation(shader, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
    //KONEC PREDCYKLU
    */

    auto startTime = std::chrono::high_resolution_clock::now();
    int frameCount = 0;

    while (!glfwWindowShouldClose(window) && frameCount < 5000)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        glClearColor(0.0f, 0.2f, 0.2f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);
        glBindTexture(GL_TEXTURE_2D, textura);

        double curTime = glfwGetTime();
        if (curTime - prevTime >= 1 / 60)
        {
            rotation += 0.1f;
            prevTime = curTime;
        }

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        //glm::mat4 model = glm::mat4(1.0f); // Identity matrix

        const float c3 = glm::cos(rotation/80);
        const float s3 = glm::sin(rotation/80);
        const float c2 = glm::cos(rotation/80);
        const float s2 = glm::sin(rotation/80);
        const float c1 = glm::cos(rotation/80);
        const float s1 = glm::sin(rotation/80);

        // Construct the rotation matrix based on your custom formula
        model[0][0] = c1 * c3 + s1 * s2 * s3;
        model[0][1] = c2 * s3;
        model[0][2] = c1 * s2 * s3 - c3 * s1;
        model[1][0] = c3 * s1 * s2 - c1 * s3;
        model[1][1] = c2 * c3;
        model[1][2] = c1 * c3 * s2 + s1 * s3;
        model[2][0] = c2 * s1;
        model[2][1] = -s2;
        model[2][2] = c1 * c2;
        //view = glm::translate(view, glm::vec3(0.0f, -0.5f, -2.0f));
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f));
        projection = glm::perspective(glm::radians(45.0f), (float)sirka / vyska, 0.1f, 100.0f);

        int modelLocation = glGetUniformLocation(shader, "model");
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        int viewLocation = glGetUniformLocation(shader, "view");
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        int projectionLocation = glGetUniformLocation(shader, "projection");
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
        frameCount++;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    float totalTime = std::chrono::duration<float, std::chrono::seconds::period>(endTime - startTime).count();

    // Output time taken
    std::cout << "Vykreslovani trvalo " << totalTime << " vterin." << std::endl;

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glDeleteTextures(1, &textura);
    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}