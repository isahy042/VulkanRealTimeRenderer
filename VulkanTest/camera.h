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

    // built from https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix.html
    void buildProjectionMatrix() {
        Vec44f proj = Vec44f(Vec4f(0.f));

        float tanHalfFOV = std::tan(vfov / 2.0f);
        float fovX = std::atan(tanHalfFOV * aspect);
        float fovY = std::atan(tanHalfFOV);

        proj[0][0] = 1.0f / (std::tan(fovX));
        proj[1][1] = 1.0f / (std::tan(fovY));
        proj[2][2] = -(far + near) / (far - near);
        proj[2][3] = -1.0f;
        proj[3][2] = -(2.0f * far * near) / (far - near);

        projectionMatrix = proj;
    }

    void applyTrasformation() {
    }

    Vec44f projectionMatrix = Vec44f(Vec4f(0.f));
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