/**
 * Camera.
 */

#pragma once

#include "vector.h"
#include <iostream>
#include <vector>
#include <string>

#include "helper.h"

using namespace std;

class Camera
{
public:
    Camera() {}

    void setValue(string n, string val)
    {
        if (n == "name")
        {
            name = val.substr(1, val.size()-2);
        }
        else if (n == "aspect")
        {
            aspect = stof(val);
        }
        else if (n == "vfov")
        {
            vfov = stof(val);
        }
        else if (n == "near")
        {
            near = stof(val);
        }
        else if (n == "far")
        {
            far = stof(val);
        }
    }

    std::string printCamera()
    {
        std::string s = "camera specs: aspect: " + std::to_string(aspect) + "vfrov: " + std::to_string(vfov) + "near: " + std::to_string(near) + "far:" + std::to_string(far);
        return s;
    }

private:
    string name = "";
    Vec2f resolution = Vec2f(480, 600);
    float aspect = 1.25f;
    float vfov = 1.f;
    float near = 0.1f;
    float far = 100.f; // infinity
    int type = 0;
    Vec2f size = Vec2f(480, 600);
    Vec3f initTrans = Vec3f(0, 0, 0);
    Vec4f initRot = Vec4f(0, 0, 0, 0);

};