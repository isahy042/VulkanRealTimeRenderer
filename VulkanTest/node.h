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
    int camera = -1;

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
    }


};

class Object {
public:
    Object(Mesh m) {
        mesh = m;
        position = m.position; // this is supposed to be a deep copy
        normal = m.normal;
        bbmin = m.bbmin;
        bbmax = m.bbmax;
        transformMatrix = identity44();
    }

    void updateBoundingBox() {
        bbmin = transformPos(transformMatrix, bbmin);
        bbmax = transformPos(transformMatrix, bbmax);
    }

    Mesh mesh;
    vector<Vec3f> position;
    vector<Vec3f> normal;
    Vec3f bbmax;
    Vec3f bbmin;
    bool inFrame = true;
    Vec44f transformMatrix;


};