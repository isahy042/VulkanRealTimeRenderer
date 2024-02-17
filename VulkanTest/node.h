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
        normalTransformMatrix = identity44();
        normalTransformMatrix[3][3] = 0.f;
    }

    int applyTransformation(Vec3f scale, Vec3f translate,Vec4f rotate){
       

        bbmax = Vec3f(-INFINITY);
        bbmin = Vec3f(INFINITY);



        for (int i = 0; i < mesh.count; i++) {
            Vec3f v = position[i];
            Vec3f n = normal[i];
            Vec4f vhomo = Vec4f(v.x, v.y, v.z, 1.f);

            Vec44f t = scaleToMatrix4(scale);

            t = matmul4444(quaternionToMatrix4(rotate), t);

            t = matmul4444(transToMatrix4(translate), t);

            vhomo = matmul444(t, vhomo);


            // scale vertex
            v = v * scale;
            // rotate vertex and normal
            Vec33f rotmat = quaternionToMatrix(rotate);
            v = matmul333(rotmat, v);
            n = matmul333(rotmat, n);
            // translate vertex
            v += translate;
            position[i] = v;
            normal[i] = n;

            // TODO: divide by w term!


            // update bounding box
            bbmax.x = max(bbmax.x, v.x);
            bbmax.y = max(bbmax.y, v.y);
            bbmax.z = max(bbmax.z, v.z);

            bbmin.x = min(bbmin.x, v.x);
            bbmin.y = min(bbmin.y, v.y);
            bbmin.z = min(bbmin.z, v.z);

        }


        return 1;

    }

    void updateBoundingBox() {
    }

    Mesh mesh;
    vector<Vec3f> position;
    vector<Vec3f> normal;
    Vec3f bbmax;
    Vec3f bbmin;
    bool inFrame = true;
    Vec44f transformMatrix;
    Vec44f normalTransformMatrix;

};