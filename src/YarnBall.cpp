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

YarnBall::YarnBall() :
    lineWidth(1)
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
    
    std::vector<float>().swap(interleavedLineVertices);
    for (size_t i = 0; i < lineVertCoords.size(); i += 3) {
        // Positions
        interleavedLineVertices.push_back(lineVertCoords[i]);
        interleavedLineVertices.push_back(lineVertCoords[i+1]);
        interleavedLineVertices.push_back(lineVertCoords[i+2]);
        // Colors
        interleavedLineVertices.push_back(lineVertColors[i]);
        interleavedLineVertices.push_back(lineVertColors[i+1]);
        interleavedLineVertices.push_back(lineVertColors[i+2]);
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

struct TriangleMesh {
    std::vector<Vec3> vertices;
    std::vector<int> indices;
};

// TriangleMesh meshSubdivide(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, int subdivLevel)
// {
//     TriangleMesh mesh;
// //    std::vector<Vec3> subTris;
//     if (subdivLevel > 0) {
//         Vec3 m01 = (v0 + v1)*0.5f;
//         Vec3 m02 = (v0 + v2)*0.5f;
//         Vec3 m12 = (v1 + v2)*0.5f;
//         m01 = m01*(1.0f/sqrt(m01*m01)) * v0.len();
//         m02 = m02*(1.0f/sqrt(m02*m02)) * v2.len();
//         m12 = m12*(1.0f/sqrt(m12*m12)) * v1.len();
//         TriangleMesh sub0 = meshSubdivide(v0, m01, m02, subdivLevel - 1);
//         TriangleMesh sub1 = meshSubdivide(m01, v1, m12, subdivLevel - 1);
//         TriangleMesh sub2 = meshSubdivide(m02, m12, v2, subdivLevel - 1);
//         TriangleMesh sub3 = meshSubdivide(m01, m12, m02, subdivLevel - 1);
//         subTris.insert(subTris.end(), sub0.begin(), sub0.end());
//         subTris.insert(subTris.end(), sub1.begin(), sub1.end());
//         subTris.insert(subTris.end(), sub2.begin(), sub2.end());
//         subTris.insert(subTris.end(), sub3.begin(), sub3.end());
//     } else {
//         subTris.push_back(v0);
//         subTris.push_back(v1);
//         subTris.push_back(v2);
//     }
//     return mesh;
// //    return subTris;
// }

std::vector<Vec3> subdivide(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, int subdivLevel)
{
    std::vector<Vec3> subTris;
    if (subdivLevel > 0) {
        Vec3 m01 = (v0 + v1)*0.5f;
        Vec3 m02 = (v0 + v2)*0.5f;
        Vec3 m12 = (v1 + v2)*0.5f;
        m01 = m01*(1.0f/sqrt(m01*m01)) * v0.len();
        m02 = m02*(1.0f/sqrt(m02*m02)) * v2.len();
        m12 = m12*(1.0f/sqrt(m12*m12)) * v1.len();
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

std::vector<Vec3> subdivideLine(const Vec3 &v0, const Vec3 &v1, int subdivLevel)
{
    std::vector<Vec3> subLines;
    if (subdivLevel > 0) {
        Vec3 m01 = (v0 + v1)*0.5f;
        m01 = m01*(v0.len()/sqrt(m01*m01));
        std::vector<Vec3> sub0 = subdivideLine(v0, m01, subdivLevel - 1);
        std::vector<Vec3> sub1 = subdivideLine(m01, v1, subdivLevel - 1);
        subLines.insert(subLines.end(), sub0.begin(), sub0.end());
        subLines.insert(subLines.end(), sub1.begin(), sub1.end());
    } else {
        subLines.push_back(v0);
        subLines.push_back(v1);
    }
    return subLines;
}

std::vector<Vec3> createWideLine(const Vec3 &p0, const Vec3 &p1, float width)
{
    Vec3 yDir = (p1-p0).normalized();
    Vec3 zDir = p0.normalized();
    Vec3 xDir = (yDir^zDir).normalized();

    float halfWidth = width*0.5;
    Vec3 bottomLeft = p0 - xDir*halfWidth;
    Vec3 bottomRight = p0 + xDir*halfWidth;
    Vec3 topLeft = p1 - xDir*halfWidth;
    Vec3 topRight = p1 + xDir*halfWidth;

    return std::vector<Vec3>({bottomLeft, bottomRight, topLeft, bottomRight, topRight, topLeft});
}

YarnBall YarnBall::fromFile(std::string fileName, int subdivLevel, int lineWidth)
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
    // Read colored lines
    std::vector<Vec3> lineVerts;
    std::vector<int> lineColors;
    int numLines = 0;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        float x0, y0, z0, x1, y1, z1;
        int col;
        if (iss >> x0 >> y0 >> z0 >> x1 >> y1 >> z1 >> col) {
            // std::vector<Vec3> wideLine = createWideLine(Vec3(x0,y0, z0)*1.01, Vec3(x1, y1, z1)*1.01, lineWidth);
            // triVerts.insert(triVerts.end(), wideLine.begin(), wideLine.end());
            // triColors.push_back(col);
            // triColors.push_back(col);
            // numTris += 2;
            lineVerts.push_back(Vec3(x0,y0,z0)*1.01);
            lineVerts.push_back(Vec3(x1,y1,z1)*1.01);
            lineColors.push_back(col);
            numLines += 1;
        } else {
            break;
        }
    }

    inFile.close();
    std::cout << "Loaded " << numTris << " triangles and " << numLines << " lines" << std::endl;

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

    for (int curLine = 0; curLine < numLines; curLine++) {
        float r = palette[lineColors[curLine]*3 + 0];
        float g = palette[lineColors[curLine]*3 + 1];
        float b = palette[lineColors[curLine]*3 + 2];

        std::vector<Vec3> subLines = subdivideLine(lineVerts[2*curLine + 0], lineVerts[2*curLine + 1], subdivLevel);
        for (size_t subLine = 0; subLine < subLines.size(); subLine += 2) {
            for (int j = 0; j < 2; ++j) {
                ball.lineVertIndices.push_back(ball.lineVertCoords.size()/3);
                ball.lineVertCoords.push_back(subLines[subLine + j].x);
                ball.lineVertCoords.push_back(subLines[subLine + j].y);
                ball.lineVertCoords.push_back(subLines[subLine + j].z);
                ball.lineVertColors.push_back(r);
                ball.lineVertColors.push_back(g);
                ball.lineVertColors.push_back(b);
            }
        }
    }
    ball.lineWidth = lineWidth;
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

    glDisable(GL_LIGHTING);

    glLineWidth(lineWidth);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 24, &interleavedLineVertices[0]);
    glColorPointer(3, GL_FLOAT, 24, &interleavedLineVertices[3]);

    glDrawElements(GL_LINES, (unsigned int)lineVertIndices.size(), GL_UNSIGNED_INT, lineVertIndices.data());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

//    glEnable(GL_LIGHTING);
}
