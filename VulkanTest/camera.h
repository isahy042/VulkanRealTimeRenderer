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
    Camera() {
        for (int i = 0; i < 8; i++) { 
            nearfrustum.push_back(Vec3f(0.f));
            farfrustum.push_back(Vec3f(0.f));
        }
    }

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
    void buildProjectionAndViewMatrix() {

        Vec44f proj = Vec44f(Vec4f(0.f));
        float scaleVfov = 1 / std::tan(vfov / 2.0f);
        proj[0][0] = scaleVfov / aspect;
        proj[1][1] = scaleVfov;
        proj[2][2] = -far / (far - near);
        proj[2][3] = -1.0f;
        proj[3][2] = - (far * near) / (far - near);
        projectionMatrix = proj;

        Vec44f view = Vec44f(Vec4f(0.f));
        Vec44f q = quaternionToMatrix4(Vec4f(0.336097, -0.0491895, -0.182892, 0.922589));
        Vec44f t = transToMatrix4(Vec3f(-6.41319, -17.984, 24.0639));
        view44(q, "quaternion \n");
        view44(t, "trans \n");

        viewMatrix = transpose44(invert44(matmul4444(t, q)));
        view44(viewMatrix, "view matrix \n ");

        // fill in frustum data
        float halfHeightNear = near * tan(vfov / 2.0f);
        float halfWidthNear = halfHeightNear * aspect;

        // Coordinates of the four corners of the viewing plane at the near distance
        nearfrustum[0] = Vec3f(-halfWidthNear, halfHeightNear, -near);
        nearfrustum[1] = Vec3f(halfWidthNear, halfHeightNear, -near);
        nearfrustum[2] = Vec3f(-halfWidthNear, -halfHeightNear, -near);
        nearfrustum[3] = Vec3f(halfWidthNear, -halfHeightNear, -near);

        float halfHeightFar = far * tan(vfov / 2.0f);
        float halfWidthFar = halfHeightFar * aspect;

        // Coordinates of the four corners of the viewing plane at the near distance
        farfrustum[0] = Vec3f(-halfWidthFar, halfHeightFar, -far);
        farfrustum[1] = Vec3f(halfWidthFar, halfHeightFar, -far);
        farfrustum[2] = Vec3f(-halfWidthFar, -halfHeightFar, -far);
        farfrustum[3] = Vec3f(halfWidthFar, -halfHeightFar, -far);
    }

    void applyTrasformation() {

    }

    Vec44f projectionMatrix = Vec44f(Vec4f(0.f));
    Vec44f viewMatrix = Vec44f(Vec4f(0.f));

    bool testIntersect(Vec3f bbmax, Vec3f bbmin) {
        // either one of the bb corner is inside the furstum or one of the frustum corner is in the bb.
        // otherwise, false
        return true;
    }

private:
    string name = "";
    Vec2f resolution = Vec2f(480, 600);
    float aspect = 1.25f;
    float vfov = 1.f;
    float near = 0.1f;
    float far = 1000.f; 
    int type = 0;
    Vec2f size = Vec2f(480, 600);
    Vec3f initTrans = Vec3f(0, 0, 0);
    Vec4f initRot = Vec4f(0, 0, 0, 0);
    vector<Vec3f> nearfrustum;
    vector<Vec3f> farfrustum;


};