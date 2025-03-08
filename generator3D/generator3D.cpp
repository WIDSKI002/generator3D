#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb-master/stb_image.h"

#include <fstream>
#include <sstream>

// Funkcja do ładowania shaderów
unsigned int loadShaders(const char* vertexPath, const char* fragmentPath) {
    // 1. Pobierz kod źródłowy shaderów z plików
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    // Upewnij się, że strumienie mogą rzucać wyjątkami
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        // Otwórz pliki
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;

        // Wczytaj zawartość plików do strumieni
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        // Zamknij pliki
        vShaderFile.close();
        fShaderFile.close();

        // Konwertuj strumienie na stringi
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 2. Skompiluj shadery
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // 3. Stwórz program shaderowy
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);

    // Sprawdź błędy linkowania
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // 4. Wyczyść shadery, ponieważ są już połączone w program
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return shaderProgram;
}

int main() {
    // Inicjalizacja GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Tworzenie okna
    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Heightmap", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Inicjalizacja GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Wczytanie obrazu mapy wysokości
    int width, height, channels;
    unsigned char* image = stbi_load("Heightmap.png", &width, &height, &channels, 0);
    if (!image) {
        std::cerr << "Failed to load image" << std::endl;
        return -1;
    } else {
        std::cout << "Image loaded successfully! Width: " << width << ", Height: " << height << ", Channels: " << channels << std::endl;
    }

    // Przetworzenie obrazu na siatkę 3D
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

 for (int y = 0; y < height; ++y) {
     for (int x = 0; x < width; ++x) {
         unsigned char heightValue = image[(y * width + x) * channels];
         float height = (heightValue / 255.0f) * 10.0f; // Skalowanie wysokości

         vertices.push_back((float)x - width / 2.0f); // Centrowanie mapy
         vertices.push_back(height);
         vertices.push_back((float)y - height / 2.0f); // Centrowanie mapy

         if (x < width - 1 && y < height - 1) {
             int topLeft = y * width + x;
             int topRight = topLeft + 1;
             int bottomLeft = (y + 1) * width + x;
             int bottomRight = bottomLeft + 1;

             indices.push_back(topLeft);
             indices.push_back(bottomLeft);
             indices.push_back(topRight);

             indices.push_back(topRight);
             indices.push_back(bottomLeft);
             indices.push_back(bottomRight);
         }
     }
 }

    stbi_image_free(image);

    // Inicjalizacja OpenGL
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Ładowanie shaderów
    GLuint shaderProgram = loadShaders("vertex.glsl", "fragment.glsl");

    // Włącz testowanie głębi
    glEnable(GL_DEPTH_TEST);

    // Zmienne do obrotu kamery
    float yaw = 0.0f;
    float pitch = 0.0f;
    float cameraSpeed = 0.05f; // Prędkość ruchu kamery
glm::vec3 cameraPos = glm::vec3(0.0f, 10.0f, 20.0f);// Początkowa pozycja kamery nad mapą

    // Ustawienie callbacków dla myszy
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Zmieniono na normalny kursor

    // Zmiana na sterowanie za pomocą klawiszy W, A, S, D
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {

        float sensitivity = 5.1f; // Wrażliwość

        // Uzyskanie dostępu do zmiennej cameraPos
        glm::vec3* cameraPos = reinterpret_cast<glm::vec3*>(glfwGetWindowUserPointer(window));

        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            if (key == GLFW_KEY_W) {
                cameraPos->x += sensitivity; // Patrz wyżej
 
            }
            if (key == GLFW_KEY_S) {
                cameraPos->x -= sensitivity; // Patrz niżej
        
            }
            if (key == GLFW_KEY_A) {
                cameraPos->z -= sensitivity; // Patrz w lewo
            }
            if (key == GLFW_KEY_D) {
                cameraPos->z += sensitivity; // Patrz w prawo
            }
            // Dodano obsługę klawiszy Z i X do zmiany pozycji kamery w osi Z
            if (key == GLFW_KEY_Z) {
                cameraPos->y += sensitivity; // Przesunięcie kamery w dół
            }
            if (key == GLFW_KEY_X) {
                cameraPos->y -= sensitivity; // Przesunięcie kamery w górę
            }
        }
    });

    // Ustawienie wskaźnika na cameraPos
    glfwSetWindowUserPointer(window, &cameraPos);

    // Zmiana na rozglądanie się przy użyciu prawego przycisku myszy
    bool firstMouse = true;
    double lastX = 0, lastY = 0;
    glm::vec3 direction;

    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {

        static bool firstMouse = true;
        static double lastX = 0, lastY = 0;

        glm::vec3* cameraPos = reinterpret_cast<glm::vec3*>(glfwGetWindowUserPointer(window));
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            if (firstMouse) {
                lastX = xpos;
                lastY = ypos;
                firstMouse = false;
            }

            float xoffset = xpos - lastX;
            float yoffset = lastY - ypos; // Odwrócenie kierunku y
            lastX = xpos;
            lastY = ypos;

            float sensitivity = 1.1f; // Wrażliwość
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            if (xoffset != 0) {
              /*  glm::vec3 direction = glm::vec3(cos(glm::radians(yaw)), sin(glm::radians(pitch)), sin(glm::radians(yaw)));
                zmiana = true;*/
                cameraPos->z += xoffset;
                cameraPos->y += yoffset;
            }
            
       
        } else {
            firstMouse = true; // Resetuj, gdy nie jest wciśnięty prawy przycisk myszy
        }
    });

    // Główna pętla renderowania
    while (!glfwWindowShouldClose(window)) {
        // Wyczyść bufory koloru i głębi
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Użyj programu shaderowego
        glUseProgram(shaderProgram);

        // Macierze kamery
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(
            cameraPos,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            (float)800 / (float)600,
            0.1f, 100.0f
        );
        glDepthFunc(GL_LESS);
        
        // Przekaż macierze do shaderów
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        //std::cout << "zmiana" << zmiana << std::endl;
        //if (zmiana) {
        //    glm::vec3 direction = glm::vec3(cos(glm::radians(yaw)), sin(glm::radians(pitch)), sin(glm::radians(yaw))); // Z ustawione na sin(pitch)
        //    cameraPos.y += glm::normalize(direction).y; // Zmiana tylko y
        //    cameraPos.z += glm::normalize(direction).z; // Zmiana tylko z
        //    std::cout << "myszka:" << cameraPos.y << ",z: " << cameraPos.z << std::endl;
        //}
        //// Ruch kamery
        //if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        //    cameraPos += cameraSpeed * glm::vec3(cos(glm::radians(yaw)), 0.0f, sin(glm::radians(yaw)));
        //}
        //if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        //    cameraPos -= cameraSpeed * glm::vec3(cos(glm::radians(yaw)), 0.0f, sin(glm::radians(yaw)));
        //}
        //if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        //    cameraPos += glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(cos(glm::radians(yaw)), 0.0f, sin(glm::radians(yaw))))) * cameraSpeed;
        //}
        //if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        //    cameraPos -= glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(cos(glm::radians(yaw)), 0.0f, sin(glm::radians(yaw))))) * cameraSpeed;
        //}
        // std::cout << "camera: " << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << std::endl;
        // Narysuj trójkąt
        //glBegin(GL_TRIANGLES);
        //// Definiuje wierzchołek trójkąta w punkcie (0.0, 0.0, 0.0)
        //glVertex3f(0.0f, 0.0f, 0.0f);
        //// Definiuje wierzchołek trójkąta w punkcie (-0.5, -0.5, 0.0)
        //glVertex3f(-0.5f, -0.5f, 0.0f);
        //// Definiuje wierzchołek trójkąta w punkcie (0.5, -0.5, 0.0) w osi współrzędnych x, y, z
        //glVertex3f(0.5f, -0.5f, 1.0f);
        //glEnd();

        // Narysuj siatkę
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Zamień bufory
        glfwSwapBuffers(window);

        // Obsłuż zdarzenia
        glfwPollEvents();
     
    }
   
    // Zwolnienie zasobów
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}