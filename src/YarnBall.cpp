#ifdef _WIN32
#include <Windows.h>
#endif

#include "Point.h"
#include "YarnBall.h"

#include <GL/gl.h>
#include <fstream>
#include <iostream>
#include <cmath> 
#include <sstream>

YarnBall::YarnBall()
{
}

void YarnBall::setupRender()
{
    std::vector<float>().swap(interleavedVertices);

    std::size_t count = vertCoords.size();
    for (size_t i = 0; i < count; i += 3) {
        // Positions
        interleavedVertices.push_back(vertCoords[i]);
        interleavedVertices.push_back(vertCoords[i+1]);
        interleavedVertices.push_back(vertCoords[i+2]);
        // Normals
        interleavedVertices.push_back(vertCoords[i]);
        interleavedVertices.push_back(vertCoords[i+1]);
        interleavedVertices.push_back(vertCoords[i+2]);
        // Colors
        interleavedVertices.push_back(vertColors[i]);
        interleavedVertices.push_back(vertColors[i+1]);
        interleavedVertices.push_back(vertColors[i+2]);
    }
}

std::vector<float> divideTri(
    float x0, float y0,
    float x1, float y1,
    float x2, float y2)
{
    std::vector<float> subTris;
    const float eps = 5.0;
    if (std::abs(x1 - x0) > eps ||
        std::abs(y1 - y0) > eps ||
        std::abs(x2 - x0) > eps ||
        std::abs(y2 - y0) > eps ||
        std::abs(x2 - x1) > eps ||
        std::abs(y2 - y1) > eps)
     {
        float x01 = 0.5*(x0+x1);
        float y01 = 0.5*(y0+y1);
        float x02 = 0.5*(x0+x2);
        float y02 = 0.5*(y0+y2);
        float x12 = 0.5*(x2+x1);
        float y12 = 0.5*(y2+y1);
        std::vector<float> sub0 = divideTri(x0, y0, x01, y01, x02, y02);
        std::vector<float> sub1 = divideTri(x01, y01, x1, y1, x12, y12);
        std::vector<float> sub2 = divideTri(x02, y02, x12, y12, x2, y2);
        std::vector<float> sub3 = divideTri(x01, y01, x12, y12, x02, y02);
        subTris.insert(subTris.end(), sub0.begin(), sub0.end());
        subTris.insert(subTris.end(), sub1.begin(), sub1.end());
        subTris.insert(subTris.end(), sub2.begin(), sub2.end());
        subTris.insert(subTris.end(), sub3.begin(), sub3.end());
    } else {
        subTris.push_back(x0);
        subTris.push_back(y0);
        subTris.push_back(x1);
        subTris.push_back(y1);
        subTris.push_back(x2);
        subTris.push_back(y2);
    }
    return subTris;
}

std::vector<Vec3> subdivide(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, int subdivLevel)
{
    std::vector<Vec3> subTris;
    if (subdivLevel > 0) {
        Vec3 m01 = (v0 + v1)*0.5f;
        Vec3 m02 = (v0 + v2)*0.5f;
        Vec3 m12 = (v1 + v2)*0.5f;
        m01 = m01*(1.0f/sqrt(m01*m01));
        m02 = m02*(1.0f/sqrt(m02*m02));
        m12 = m12*(1.0f/sqrt(m12*m12));
        std::vector<Vec3> sub0 = subdivide(v0, m01, m02, subdivLevel - 1);
        std::vector<Vec3> sub1 = subdivide(m01, v1, m12, subdivLevel - 1);
        std::vector<Vec3> sub2 = subdivide(m02, m12, v2, subdivLevel - 1);
        std::vector<Vec3> sub3 = subdivide(m01, m12, m02, subdivLevel - 1);
        subTris.insert(subTris.end(), sub0.begin(), sub0.end());
        subTris.insert(subTris.end(), sub1.begin(), sub1.end());
        subTris.insert(subTris.end(), sub2.begin(), sub2.end());
        subTris.insert(subTris.end(), sub3.begin(), sub3.end());
    } else {
        subTris.push_back(v0);
        subTris.push_back(v1);
        subTris.push_back(v2);
    }
    return subTris;
}

YarnBall YarnBall::fromFile(std::string fileName, int subdivLevel)
{
    std::ifstream inFile(fileName);
    if (!inFile) {
        std::cerr << "Couldn't open file [" << fileName << "]" << std::endl;
        return YarnBall();
    }

    YarnBall ball;
    std::string line;

    // Read color palette
    std::vector<float> palette(256*3);
    int numColors = 0;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        float r, g, b;
        int idx;
        if (iss >> idx >> r >> g >> b) {
            palette[3*idx + 0] = r;
            palette[3*idx + 1] = g;
            palette[3*idx + 2] = b;
            numColors++;
        } else {
            break;
        }
    }
    std::cout << "Loaded " << numColors << " colors" << std::endl;

    // Read colored triangles
    std::vector<Vec3> triVerts;
    std::vector<int> triColors;
    int numTris = 0;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        float x0, y0, z0, x1, y1, z1, x2, y2, z2;
        int col;
        if (iss >> x0 >> y0 >> z0 >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> col) {
            triVerts.push_back(Vec3(x0,y0,z0));
            triVerts.push_back(Vec3(x1,y1,z1));
            triVerts.push_back(Vec3(x2,y2,z2));
            triColors.push_back(col);
            numTris += 1;
        } else {
            break;
        }
    }
    inFile.close();
    std::cout << "Loaded " << triColors.size() << " triangles" << std::endl;

    // Create the geometry from the vertices in spherical coords and the 
    // color palette
    for (int i = 0; i < numTris; ++i) {
        // float r = 1;
        // float g = 0;
        // float b = 1;
        float r = palette[triColors[i]*3 + 0];
        float g = palette[triColors[i]*3 + 1];
        float b = palette[triColors[i]*3 + 2];

        // Sanity checking
        Vec3 v01 = triVerts[3*i + 1] - triVerts[3*i + 0];
        Vec3 v12 = triVerts[3*i + 2] - triVerts[3*i + 1];
        Vec3 v20 = triVerts[3*i + 0] - triVerts[3*i + 2];
        Vec3 n0 = v20^v01;
        Vec3 n1 = v01^v12;
        Vec3 n2 = v12^v20;
        if (triVerts[3*i + 0]*n0 < 0.0 ||
            triVerts[3*i + 1]*n1 < 0.0 ||
            triVerts[3*i + 2]*n2 < 0.0) {
            std::swap(triVerts[3*i + 0], triVerts[3*i + 1]);
        }
        std::vector<Vec3> subTris = subdivide(triVerts[3*i + 0], triVerts[3*i + 1], triVerts[3*i + 2], subdivLevel);
//        std::vector<Vec3> subTris = {triVerts[3*i + 0], triVerts[3*i + 1], triVerts[3*i + 2]};
        for (size_t subTri = 0; subTri < subTris.size(); subTri += 3) {
            for (int j = 0; j < 3; ++j) {
                ball.vertIndices.push_back(ball.vertCoords.size()/3);
                ball.vertCoords.push_back(subTris[subTri + j].x);
                ball.vertCoords.push_back(subTris[subTri + j].y);
                ball.vertCoords.push_back(subTris[subTri + j].z);
                ball.vertColors.push_back(r);
                ball.vertColors.push_back(g);
                ball.vertColors.push_back(b);
            }
        }
    }
    ball.setupRender();
    return ball;
}

void YarnBall::draw()
{
    if (interleavedVertices.empty()) return;
    // interleaved array
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 36, &interleavedVertices[0]);
    glNormalPointer(GL_FLOAT, 36, &interleavedVertices[3]);
    glColorPointer(3, GL_FLOAT, 36, &interleavedVertices[6]);

    glDrawElements(GL_TRIANGLES, (unsigned int)vertIndices.size(), GL_UNSIGNED_INT, vertIndices.data());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}
