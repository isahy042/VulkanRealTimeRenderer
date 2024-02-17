/***
 * This file contains any additional helper functions that may be needed.
 */

#pragma once

#include <iostream>
#include <fstream> // for std::ifstream

#include <string>
#include <vector>

#include "vector.h"

#include <glm/glm.hpp>

using namespace std;

const string FOLDER = "C:/Users/Sasa/Desktop/Spring2024/672Graphics/";
const int FPS = 50;

// https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
// https://github.com/MonoGame/MonoGame/blob/develop/MonoGame.Framework/Matrix.cs

/**
Transformation Conversion 
*/
Vec33f quaternionToMatrix(Vec4f q) {
    Vec33f m(Vec3f(0.f));
    float qx = q[0];
    float qy = q[1];
    float qz = q[2];
    float qw = q[3];

    m[0][0] = 1 - (2 * qy * qy) - (2 * qz * qz);
    m[0][1] = (2 * qx * qy) - (2 * qz * qw);
    m[0][2] = (2 * qx * qz) + (2 * qy * qw);

    m[1][0] = (2 * qx * qy) + (2 * qz * qw);
    m[1][1] = 1 - (2 * qx * qx) - (2 * qz * qz);
    m[1][2] = (2 * qy * qz) - (2 * qx * qw);

    m[2][0] = (2 * qx * qz) - (2 * qy * qw);
    m[2][1] = (2 * qz * qy) + (2 * qx * qw);
    m[2][2] = 1 - (2 * qx * qx) - (2 * qy * qy);

    return m;
}

Vec44f quaternionToMatrix4(Vec4f q) {
    Vec44f m(Vec4f(0.f));
    float qx = q[0];
    float qy = q[1];
    float qz = q[2];
    float qw = q[3];

    m[0][0] = 1 - (2 * qy * qy) - (2 * qz * qz);
    m[0][1] = (2 * qx * qy) - (2 * qz * qw);
    m[0][2] = (2 * qx * qz) + (2 * qy * qw);

    m[1][0] = (2 * qx * qy) + (2 * qz * qw);
    m[1][1] = 1 - (2 * qx * qx) - (2 * qz * qz);
    m[1][2] = (2 * qy * qz) - (2 * qx * qw);

    m[2][0] = (2 * qx * qz) - (2 * qy * qw);
    m[2][1] = (2 * qz * qy) + (2 * qx * qw);
    m[2][2] = 1 - (2 * qx * qx) - (2 * qy * qy);

    m[3][3] = 1.f;

    return m;
}

 Vec44f transToMatrix4(Vec3f t) {
    Vec44f m(Vec4f(0.f));
    m[0][0] = 1.f;
    m[1][1] = 1.f;
    m[2][2] = 1.f;
    m[3][3] = 1.f;
    m[0][3] = t.x;
    m[1][3] = t.y;
    m[2][3] = t.z;
    return m;
}

 Vec44f scaleToMatrix4(Vec3f s) {
    Vec44f m(Vec4f(0.f));
    m[0][0] = s.x;
    m[1][1] = s.y;
    m[2][2] = s.z;
    m[3][3] = 1.f;
    return m;
}

 /**
Matrix Multiplication
*/
inline Vec3f matmul333(Vec33f m, Vec3f v) {
    Vec3f r = Vec3f(0.f);

    r.x = v.x * m[0].x + v.y * m[0].y + v.z * m[0].z;
    r.y = v.x * m[1].x + v.y * m[1].y + v.z * m[1].z;
    r.z = v.x * m[2].x + v.y * m[2].y + v.z * m[2].z;

    return r;
}

inline Vec33f matmul3333(Vec33f m1, Vec33f m2) {
    Vec33f m(Vec3f(0.f));
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            m[i][j] = 0.0f;
            for (int k = 0; k < 3; ++k) {
                m[i][j] += m1[i][k] * m2[k][j];
            }
        }
    }
    return m;
}

inline Vec4f matmul444(Vec44f m, Vec4f v) {

    Vec4f r = Vec4f(0.f);

    r[0] = v[0] * m[0][0] + v[1] * m[0][1] + v[2] * m[0][2] + v[3] * m[0][3];
    r[1] = v[0] * m[1][0] + v[1] * m[1][1] + v[2] * m[1][2] + v[3] * m[1][3];
    r[2] = v[0] * m[2][0] + v[1] * m[2][1] + v[2] * m[2][2] + v[3] * m[2][3];
    r[3] = v[0] * m[3][0] + v[1] * m[3][1] + v[2] * m[3][2] + v[3] * m[3][3];

    return r;
}

inline Vec44f matmul4444(Vec44f m1, Vec44f m2) {
    Vec44f m(Vec4f(0.f));
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m[i][j] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                m[i][j] += m1[i][k] * m2[k][j];
            }
        }
    }

    return m;
}

Vec3f transformPos(Vec44f t, Vec3f pos) {

    Vec4f newp = matmul444(t, Vec4f(pos.x, pos.y, pos.z, 1.f));
    return Vec3f(newp[0], newp[1], newp[2]) / newp[3];

}

/**
Matrix Operation
*/

Vec33f identity33() {
    Vec33f t = Vec33f(Vec3f(0.f));
    for (int i = 0; i < 3; ++i) {
        t[i][i] = 1.f;
    }
    return t;
}

Vec44f invert44(Vec44f mat) {

    Vec44f r = Vec44f(Vec4f(0.f));

    float m[16];
    int cnt = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m[cnt] = mat[i][j];
            cnt++;
        }
    }
    float inv[16], det;
    int i;

    inv[0] = m[5] * m[10] * m[15] -m[5] * m[11] * m[14] -m[9] * m[6] * m[15] +
        m[9] * m[7] * m[14] + m[13] * m[6] * m[11] -m[13] * m[7] * m[10];

    inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] +m[8] * m[6] * m[15] -
        m[8] * m[7] * m[14] - m[12] * m[6] * m[11] +m[12] * m[7] * m[10];

    inv[8] = m[4] * m[9] * m[15] -m[4] * m[11] * m[13] -m[8] * m[5] * m[15] +
        m[8] * m[7] * m[13] +m[12] * m[5] * m[11] - m[12] * m[7] * m[9];

    inv[12] = -m[4] * m[9] * m[14] +m[4] * m[10] * m[13] +m[8] * m[5] * m[14] -
        m[8] * m[6] * m[13] -m[12] * m[5] * m[10] +m[12] * m[6] * m[9];

    inv[1] = -m[1] * m[10] * m[15] +m[1] * m[11] * m[14] +m[9] * m[2] * m[15] -
        m[9] * m[3] * m[14] -m[13] * m[2] * m[11] + m[13] * m[3] * m[10];

    inv[5] = m[0] * m[10] * m[15] -m[0] * m[11] * m[14] -m[8] * m[2] * m[15] +
        m[8] * m[3] * m[14] +m[12] * m[2] * m[11] -m[12] * m[3] * m[10];

    inv[9] = -m[0] * m[9] * m[15] +m[0] * m[11] * m[13] + m[8] * m[1] * m[15] -
        m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];

    inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] +
        m[8] * m[2] * m[13] +m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

    inv[2] = m[1] * m[6] * m[15] -m[1] * m[7] * m[14] - m[5] * m[2] * m[15] +
        m[5] * m[3] * m[14] + m[13] * m[2] * m[7] -m[13] * m[3] * m[6];

    inv[6] = -m[0] * m[6] * m[15] +m[0] * m[7] * m[14] + m[4] * m[2] * m[15] -
        m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];

    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] +
        m[4] * m[3] * m[13] +m[12] * m[1] * m[7] - m[12] * m[3] * m[5];

    inv[14] = -m[0] * m[5] * m[14] +m[0] * m[6] * m[13] + m[4] * m[1] * m[14] -
        m[4] * m[2] * m[13] -m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] +m[5] * m[2] * m[11] -
        m[5] * m[3] * m[10] - m[9] * m[2] * m[7] +m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] +
        m[4] * m[3] * m[10] + m[8] * m[2] * m[7] -m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +m[0] * m[7] * m[9] +m[4] * m[1] * m[11] -
        m[4] * m[3] * m[9] -m[8] * m[1] * m[7] +m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -m[0] * m[6] * m[9] - m[4] * m[1] * m[10] +
        m[4] * m[2] * m[9] + m[8] * m[1] * m[6] -m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0) {
        cout << "matrix not invertible.";
        return r;
    }

    det = 1.0 / det;

    cnt = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            r[i][j] = inv[cnt] * det;
            cnt++;
        }
    }
    return r;
}

Vec44f transpose44(Vec44f mat) {
    Vec44f trans;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            trans[i][j] = mat[j][i];
        }
    }
    return trans;
}

Vec44f identity44() {
    Vec44f t = Vec44f(Vec4f(0.f));
    for (int i = 0; i < 4; ++i) {
        t[i][i] = 1.f;
    }
    return t;
}



/**
Data type Conversion
*/

inline vector<int> tovector(string line)
{
    std::size_t start = line.find('[');
    vector<int> intvector;
    string currNum = "";
    while (true)
    {
        start++;
        if (line[start] == ',')
        {
            intvector.push_back(std::stoi(currNum));
            currNum = "";
        }
        else if (line[start] == ']')
        {
            intvector.push_back(std::stoi(currNum));
            break;
        }
        else
        {
            currNum += line[start];
        }
    }
    return intvector;
}

inline vector<float> tovectorf(string line)
{
    std::size_t start = line.find('[');
    vector<float> floatvector;
    string currNum = "";
    while (true)
    {
        start++;
        if (line[start] == ',')
        {
            floatvector.push_back(std::stof(currNum));
            currNum = "";
        }
        else if (line[start] == ']')
        {
            floatvector.push_back(std::stof(currNum));
            break;
        }
        else
        {
            currNum += line[start];
        }
    }

    return floatvector;
}

inline Vec3f tovec3f(string v)
{
    vector<float> f = tovectorf(v);
    return Vec3f(f[0], f[1], f[2]);
}

inline Vec4f tovec4f(string v)
{
    vector<float> f = tovectorf(v);
    return Vec4f(f[0], f[1], f[2], f[3]);
}

float* makeUboMatrix(Vec44f t) {

    float p[16] = { t[0][0], t[0][1], t[0][2], t[0][3],
            t[1][0], t[1][1], t[1][2], t[1][3],
            t[2][0], t[2][1], t[2][2], t[2][3],
            t[3][0], t[3][1], t[3][2], t[3][3]
    };

    return p;
}

/**
Quick to-string debugging functions
*/
inline void view44(Vec44f matrix, string s = "") {
    cout << "\n printing matirx " << s;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << matrix[i][j] << "\t";
        }
        std::cout << std::endl;
    }
}

inline void message(string m, bool clear = false)
{
    std::ofstream outfile;

    if (clear)
    {
        outfile.open(FOLDER + "A1sceneViewer/output.txt");
        outfile << m;
        outfile << "\n ---------------- \n";
    }
    else
    {
        outfile.open(FOLDER + "A1sceneViewer/output.txt", std::ios_base::app); // append instead of overwrite
        outfile << m;
        outfile << "\n ---------------- \n";
    }
}

