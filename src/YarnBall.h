#pragma once

#include <string>
#include <vector>

class YarnBall
{
public:
    YarnBall();
    static YarnBall fromFile(std::string fileName, int subdivLevel, int lineWidth);
    void draw();

private:
    void setupRender();

private:
    // Lines
    int lineWidth;
    std::vector<float> interleavedLineVertices;
    std::vector<float> lineVertCoords;
    std::vector<float> lineVertColors;
    std::vector<unsigned int> lineVertIndices;
    // Triangles
    std::vector<float> interleavedVertices;
    std::vector<float> vertCoords;
    std::vector<float> vertColors;
    std::vector<unsigned int> vertIndices;
};

