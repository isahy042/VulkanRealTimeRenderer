#pragma once

#include "vector.h"
#include "helper.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;


class Node
{
public:
    Node() {}

    Vec3f translate = Vec3f(0, 0, 0);
    Vec4f rotate = Vec4f(0, 0, 0, 1);
    Vec3f scale = Vec3f(1, 1, 1);
    vector<int> children;
    int mesh = -1;
    int light = -1;
    int camera = -1;
    int env = -1;

    bool driveScale = false;
    bool driveTranslate = false;
    bool driveRotate = false;
    int transDriver = -1;
    int rotateDriver = -1;
    int scaleDriver = -1;

    Vec3f bbmax = Vec3f(-INFINITY);
    Vec3f bbmin = Vec3f(INFINITY);

    void setValue(string n, string val)
    {
        if (n == "translation")
        {
            translate = tovec3f(val);
        }
        else if (n == "rotation")
        {
            rotate = tovec4f(val);
        }
        else if (n == "scale")
        {
            scale = tovec3f(val);
        }
        else if (n == "children")
        {
            children = tovector(val);
        }
        else if (n == "camera")
        {
            camera = stoi(val);
        }
        else if (n == "mesh")
        {
            mesh = stoi(val);
        }
        else if (n == "light")
        {
            light = stoi(val);
        }
        else if (n == "enviroment")
        {
            env = stoi(val);
        }
    }


};

class Object {
public:
    Object(Mesh m) {
        mesh = m;
        position = m.position; // this is supposed to be a deep copy
        normal = m.normal;

        tangent = m.tangent;
        texcoord = m.texcoord;
        color = m.color;

        bbmin = m.bbmin;
        bbmax = m.bbmax;

        transformMatrix = identity44();
    }

    void updateBoundingBox() {
        Vec3f temp = transformPos(transformMatrix, mesh.bbcorners[0]);
        bbmin = temp;
        bbmax = temp;

        for (int c = 1; c < 8; c++) {
            temp = transformPos(transformMatrix, mesh.bbcorners[c]);
            for (int i = 0; i < 3; i++) {
                bbmin[i] = min(bbmin[i], temp[i]);
                bbmax[i] = max(bbmax[i], temp[i]);
            }
        }
       
    }

    Mesh mesh;
    vector<Vec3f> position;
    vector<Vec3f> normal;
    vector<Vec4f> tangent;
    vector<Vec2f> texcoord;
    vector<Vec4f> color;


    Vec3f bbmax;
    Vec3f bbmin;
    bool inFrame = true;
    Vec44f transformMatrix;

};