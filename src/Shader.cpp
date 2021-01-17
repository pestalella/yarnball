#ifdef _WIN32
#include <Windows.h>
#endif

#include "Shader.h"

#include <GL/GL.h>

#include <fstream>
#include <iostream>
#include <sstream>

std::string readTextFile(std::string filename)
{
    std::string text;
    std::ifstream infile;
    // ensure ifstream objects can throw exceptions:
    infile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // open file
        infile.open(filename);
        // read file's buffer contents into streams
        std::stringstream textStream;
        textStream << infile.rdbuf();
        // close file handlers
        infile.close();
        // convert stream into string
        text = textStream.str();
    }
    catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    return text;
}

Shader Shader::fromFiles(std::string vertexPath, std::string fragmentPath)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode = readTextFile(vertexPath);
    std::string fragmentCode = readTextFile(fragmentPath);
    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    // print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    // similiar for Fragment Shader
    [...]

    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    // print linking errors if any
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return Shader();
}