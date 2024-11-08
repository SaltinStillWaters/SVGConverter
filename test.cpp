#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>


void display();
void initBuffers(std::vector<float>& vector);

struct ShaderProgramSource {
    std::string VertexShader;
    std::string FragmentShader;
};

static ShaderProgramSource ParseShader(const std::string& filepath) {
    std::ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1,
    };

    ShaderType shaderType = ShaderType::NONE;
    std::stringstream ss[2];

    std::string line;
    while (getline(stream, line)) {
        if (line.find("shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                shaderType = ShaderType::VERTEX;
            } else if (line.find("fragment") != std::string::npos) {
                shaderType = ShaderType::FRAGMENT;
            }
        } else {
            ss[(int) shaderType] << line << '\n';
        }
    }

    return {ss[0].str(), ss[1].str()};
}

static unsigned int CompileShader(GLuint type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    //error handling
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*) alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);

        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader\n";
        std::cout << message << '\n';
        
        glDeleteShader(id);
        return 0;
    }

    return id;
}

static unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void split(std::string s, char del, std::vector<float>& vector);

void fileToVector(std::vector<float>& vector, const char* filename) {
    std::ifstream file(filename);
    std::string line;

    while(std::getline(file, line)) {
        std::cout << line << '\n';
        split(line, ' ', vector);
    }
}

void split(std::string s, char del, std::vector<float>& vector) {
    std::stringstream ss(s);
    std::string word;
    float val;

    while (!ss.eof()) {
        getline(ss, word, del);
        vector.push_back(std::stof(word));

    }
}

std::vector<float> vector;

int main(int argcp, char** argv) {
    glutInit(&argcp, argv);
    glutCreateWindow("Window");

    if (glewInit() != GLEW_OK) {
        std::cout << "Error initializing glew\n";
    }
    fileToVector(vector, "vertices.txt");

    for (int x = 0; x < vector.size(); ++x) {
        std::cout << vector[x] << '\n';
    }
    
    initBuffers(vector);
    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_QUADS, 0, vector.size());
    glutSwapBuffers();
    glFlush();
}

void initBuffers(std::vector<float>& vector) {
    unsigned int vbo, vao;
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vector.size() * sizeof(float), vector.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, (void *) (2 * sizeof(GLfloat)));

    ShaderProgramSource source = ParseShader("shader.shader");

    unsigned int shader = createShader(source.VertexShader, source.FragmentShader);
    glUseProgram(shader);

    glDeleteProgram(shader);
}