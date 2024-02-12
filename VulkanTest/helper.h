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
    return Vec3f(0.f);
}

inline Vec4f tovec4f(string v)
{
    return Vec4f(0.f);
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
#pragma once
