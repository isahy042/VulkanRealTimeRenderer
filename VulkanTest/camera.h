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
        for (int i = 0; i < 6; i++) {
            normals.push_back(Vec3f(0.f));
        }
    }

    void setValue(string n, string val)
    {
        if (n == "name")
        {
            name = val.substr(1, val.size()-3);
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
    void initializeProjection() {
        transformMatrix = identity44();

        Vec44f proj = Vec44f(Vec4f(0.f));
        float scaleVfov = 1 / std::tan(vfov / 2.0f);
        proj[0][0] = scaleVfov / aspect;
        proj[1][1] = scaleVfov;
        proj[2][2] = -far / (far - near);
        proj[2][3] = -1.0f;
        proj[3][2] = - (far * near) / (far - near);
        projectionMatrix = proj;

        // fill in frustum data
        float halfHeightNear = near * tan(vfov / 2.0f);
        float halfWidthNear = halfHeightNear * aspect;

        // Coordinates of the four corners of the viewing plane at the near distance
        nearfrustum[0] = Vec3f(-halfWidthNear, halfHeightNear, -near); // clockwise
        nearfrustum[1] = Vec3f(halfWidthNear, halfHeightNear, -near);
        nearfrustum[2] = Vec3f(halfWidthNear, -halfHeightNear, -near);
        nearfrustum[3] = Vec3f(-halfWidthNear, -halfHeightNear, -near);

        float halfHeightFar = far * tan(vfov / 2.0f);
        float halfWidthFar = halfHeightFar * aspect;

        // Coordinates of the four corners of the viewing plane at the near distance
        farfrustum[0] = Vec3f(-halfWidthFar, halfHeightFar, -far);
        farfrustum[1] = Vec3f(halfWidthFar, halfHeightFar, -far);
        farfrustum[2] = Vec3f(halfWidthFar, -halfHeightFar, -far);
        farfrustum[3] = Vec3f(-halfWidthFar, -halfHeightFar, -far);
    }

    void updateNormal() {
        normals[0] = cross((farfrustum[0] - farfrustum[1]),(farfrustum[2] - farfrustum[1])); // far plane
        normals[1] = cross((nearfrustum[2] - nearfrustum[1]), (nearfrustum[0] - nearfrustum[1])); // near plane
        normals[2] = cross((nearfrustum[0] - nearfrustum[3]),(farfrustum[1] - nearfrustum[3])); // top plane
        normals[3] = cross((farfrustum[3] - nearfrustum[3]), (nearfrustum[0] - nearfrustum[3])); // left plane
        normals[4] = cross((farfrustum[3] - farfrustum[2]), (nearfrustum[2] - farfrustum[2])); // bottom plane
        normals[5] = cross((farfrustum[1] - nearfrustum[3]), (nearfrustum[2] - nearfrustum[3])); // right plane
    }
    
    void applyTrasformation() {
        // update frustum
        for (int f = 0; f < 4; f++) {
            farfrustum[f] = transformPos(transformMatrix, farfrustum[f]);
            nearfrustum[f] = transformPos(transformMatrix, nearfrustum[f]);
        }
        // update normal
        updateNormal();
    }

    Vec44f projectionMatrix = Vec44f(Vec4f(0.f));
    Vec44f viewMatrix = Vec44f(Vec4f(0.f));
    Vec44f transformMatrix = Vec44f(Vec4f(0.f));
    string name = "";
    bool testIntersect(Vec3f bbmax, Vec3f bbmin) {
        vector<Vec3f> bbcorners;
        bbcorners.push_back(Vec3f(bbmin.x, bbmin.y, bbmin.z)); // Min corner (x, y, z)
        bbcorners.push_back(Vec3f(bbmin.x, bbmin.y, bbmax.z));
        bbcorners.push_back(Vec3f(bbmin.x, bbmax.y, bbmin.z ));
        bbcorners.push_back(Vec3f(bbmin.x, bbmax.y, bbmax.z));
        bbcorners.push_back(Vec3f(bbmax.x, bbmin.y, bbmin.z));
        bbcorners.push_back(Vec3f(bbmax.x, bbmin.y, bbmax.z ));
        bbcorners.push_back(Vec3f(bbmax.x, bbmax.y, bbmin.z ));
        bbcorners.push_back(Vec3f(bbmax.x, bbmax.y, bbmax.z ));

        // either one of the bb corner is inside the furstum or one of the frustum corner is in the bb.
        // bb corner inside frustum
        for (int corner = 0; corner < 8; corner++) {
            if ((dot(bbcorners[corner] - farfrustum[0], normals[0]) < 0)) {
                continue;
            }
            if ((dot(bbcorners[corner] - nearfrustum[0], normals[1]) < 0)) {
                continue;
            }
            if ((dot(bbcorners[corner] - farfrustum[0], normals[2]) < 0)) {
                continue;
            }
            if ((dot(bbcorners[corner] - farfrustum[0], normals[3]) < 0)) {
                continue;
            }
            if ((dot(bbcorners[corner] - farfrustum[2], normals[4]) < 0)) {
                continue;
            }
            if ((dot(bbcorners[corner] - farfrustum[2], normals[5]) < 0)) {
                continue;
            }
            return true;
        }
        
        // furstum corner inside bb
        for (int corner = 0; corner < 4; corner++) {
            if ((farfrustum[corner].x >= bbmin.x) && (farfrustum[corner].x <= bbmax.x) &&
                (farfrustum[corner].y >= bbmin.y) && (farfrustum[corner].y <= bbmax.y) &&
                (farfrustum[corner].z >= bbmin.z) && (farfrustum[corner].z <= bbmax.z)) {
                return true;
            }
            if ((nearfrustum[corner].x >= bbmin.x) && (nearfrustum[corner].x <= bbmax.x) &&
                (nearfrustum[corner].y >= bbmin.y) && (nearfrustum[corner].y <= bbmax.y) &&
                (nearfrustum[corner].z >= bbmin.z) && (nearfrustum[corner].z <= bbmax.z)) {
                return true;
            }
        }
        // otherwise, false
        return false;
    }

private:
    
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
    vector<Vec3f> normals;


};