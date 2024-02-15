/***
 * This file contains any additional helper functions that may be needed.
 */

#pragma once

#include <iostream>
#include <fstream> // for std::ifstream

#include <string>
#include <vector>

#include "vector.h"

using namespace std;

const string FOLDER = "C:/Users/Sasa/Desktop/Spring2024/672Graphics/";

inline Vec33f quaternionToMatrix(Vec4f q) {
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

inline Vec3f matmul333(Vec33f m, Vec3f v) {
    Vec3f r = Vec3f(0.f);

    r.x = v.x * m[0].x + v.y * m[0].y + v.z * m[0].z;
    r.y = v.x * m[1].x + v.y * m[1].y + v.z * m[1].z;
    r.z = v.x * m[2].x + v.y * m[2].y + v.z * m[2].z;

    return r;
}

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

