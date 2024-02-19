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
    Vec44f projectionMatrix = Vec44f(Vec4f(0.f));
    Vec44f viewMatrix = Vec44f(Vec4f(0.f));
    Vec44f transformMatrix = Vec44f(Vec4f(0.f));
    string name = "";

    vector<Vec3f> initnearfrustum;
    vector<Vec3f> initfarfrustum;
    vector<Vec3f> initnormals;

    vector<Vec3f> nearfrustum;
    vector<Vec3f> farfrustum;
    vector<Vec3f> normals;

    float aspect = 1.25f;

    Camera() {
        for (int i = 0; i < 8; i++) { 
            nearfrustum.push_back(Vec3f(0.f));
            farfrustum.push_back(Vec3f(0.f));

            initnearfrustum.push_back(Vec3f(0.f));
            initfarfrustum.push_back(Vec3f(0.f));
        }
        for (int i = 0; i < 6; i++) {
            normals.push_back(Vec3f(0.f));
            initnormals.push_back(Vec3f(0.f));

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
        initnearfrustum[0] = Vec3f(-halfWidthNear, halfHeightNear, -near); // clockwise
        initnearfrustum[1] = Vec3f(halfWidthNear, halfHeightNear, -near);
        initnearfrustum[2] = Vec3f(halfWidthNear, -halfHeightNear, -near);
        initnearfrustum[3] = Vec3f(-halfWidthNear, -halfHeightNear, -near);

        float halfHeightFar = far * tan(vfov / 2.0f);
        float halfWidthFar = halfHeightFar * aspect;

        // Coordinates of the four corners of the viewing plane at the near distance
        initfarfrustum[0] = Vec3f(-halfWidthFar, halfHeightFar, -far);
        initfarfrustum[1] = Vec3f(halfWidthFar, halfHeightFar, -far);
        initfarfrustum[2] = Vec3f(halfWidthFar, -halfHeightFar, -far);
        initfarfrustum[3] = Vec3f(-halfWidthFar, -halfHeightFar, -far);

    }
    
    void updateNormal() {
        normals[0] = normalize(cross((farfrustum[0] - farfrustum[1]),(farfrustum[2] - farfrustum[1]))); // far plane
        normals[1] = normalize(cross((nearfrustum[2] - nearfrustum[1]), (nearfrustum[0] - nearfrustum[1]))); // near plane
        normals[2] = normalize(cross((nearfrustum[0] - nearfrustum[1]),(farfrustum[1] - nearfrustum[1]))); // top plane
        normals[3] = normalize(cross((farfrustum[3] - nearfrustum[3]), (nearfrustum[0] - nearfrustum[3]))); // left plane
        normals[4] = normalize(cross((farfrustum[3] - farfrustum[2]), (nearfrustum[2] - farfrustum[2]))); // bottom plane
        normals[5] = normalize(cross((farfrustum[1] - nearfrustum[1]), (nearfrustum[2] - nearfrustum[1]))); // right plane
    }
    
    void applyTrasformation() {
        // update frustum
        for (int f = 0; f < 4; f++) {
            farfrustum[f] = transformPos(transformMatrix, initfarfrustum[f]);
            nearfrustum[f] = transformPos(transformMatrix, initnearfrustum[f]);
        }
        // update normal
        updateNormal();
    }


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

        // intersect but no corners inside each other :(
        // test if either 4 lines of the frustum intersects any plane of the bb.
        for (int l = 0; l < 4; l++) {
            Vec3f line = normalize(farfrustum[l] - nearfrustum[l]);

            float mint = (bbmin.x - nearfrustum[l].x) / line.x;
            float maxt = (bbmax.x - nearfrustum[l].x) / line.x;

            if ((((line.y * mint) + nearfrustum[l].y) >= bbmin.y) && (((line.y * mint) + nearfrustum[l].y) <= bbmax.y) && (mint >= 0) &&
                (((line.z * mint) + nearfrustum[l].z) >= bbmin.z) && (((line.z * mint) + nearfrustum[l].z) <= bbmax.z)) return true;

            if ((((line.y * maxt) + nearfrustum[l].y) >= bbmin.y) && (((line.y * maxt) + nearfrustum[l].y) <= bbmax.y) && (maxt >= 0) &&
                (((line.z * maxt) + nearfrustum[l].z) >= bbmin.z) && (((line.z * maxt) + nearfrustum[l].z) <= bbmax.z)) return true;

            mint = (bbmin.y - nearfrustum[l].y) / line.y;
            maxt = (bbmax.y - nearfrustum[l].y) / line.y;

            if ((((line.x * mint) + nearfrustum[l].x) >= bbmin.x) && (((line.x * mint) + nearfrustum[l].x) <= bbmax.x) && (mint >= 0) &&
                (((line.z * mint) + nearfrustum[l].z) >= bbmin.z) && (((line.z * mint) + nearfrustum[l].z) <= bbmax.z)) return true;

            if ((((line.x * maxt) + nearfrustum[l].x) >= bbmin.x) && (((line.x * maxt) + nearfrustum[l].x) <= bbmax.x) && (maxt >= 0) &&
                (((line.z * maxt) + nearfrustum[l].z) >= bbmin.z) && (((line.z * maxt) + nearfrustum[l].z) <= bbmax.z)) return true;

            mint = (bbmin.z - nearfrustum[l].z) / line.z;
            maxt = (bbmax.z - nearfrustum[l].z) / line.z;

            if ((((line.x * mint) + nearfrustum[l].x) >= bbmin.x) && (((line.x * mint) + nearfrustum[l].x) <= bbmax.x) && (mint >= 0) &&
                (((line.y * mint) + nearfrustum[l].y) >= bbmin.y) && (((line.y * mint) + nearfrustum[l].y) <= bbmax.y)) return true;

            if ((((line.x * maxt) + nearfrustum[l].x) >= bbmin.x) && (((line.x * maxt) + nearfrustum[l].x) <= bbmax.x) && (maxt >= 0) &&
                (((line.y * maxt) + nearfrustum[l].y) >= bbmin.y) && (((line.y * maxt) + nearfrustum[l].y) <= bbmax.y)) return true;

            }


        // otherwise, false
        return false;
    }

private:
    float vfov = 1.f;
    float near = 0.1f;
    float far = 1000.f; 
    int type = 0;
    Vec2f size = Vec2f(480, 600);
    Vec3f initTrans = Vec3f(0, 0, 0);
    Vec4f initRot = Vec4f(0, 0, 0, 0);



};