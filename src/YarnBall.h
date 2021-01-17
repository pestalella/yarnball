#pragma once

#include <string>
#include <vector>

class YarnBall
{
public:
    YarnBall();
    static YarnBall fromFile(std::string fileName, int subdivLevel);
    void draw();

private:
    void setupRender();

private:
    std::vector<float> interleavedVertices;
    std::vector<float> vertCoords;
    std::vector<float> vertColors;
    std::vector<unsigned int> vertIndices;
};

