#pragma once

#include "vector.h"
#include "helper.h"
#include <iostream>
#include <vector>
#include <string>

#include <fstream>
#include <iostream>

using namespace std;


// since the cloud vdb spans the whole scene, we assume only one instance in each scene.

class Cloud {
public:
    Cloud() {}
    Cloud(bool v) { visible = v; }

    string name;
    string voxelPath = "Clouds/StormbirdCloud/TGA/";
    string noisePath = "Clouds/noise/";

    Vec3f grid = Vec3f(512,64,512);
    float size = 0.1;
    bool visible = false;

    Vec44f data = Vec44f(Vec4f(0.0), Vec4f(0.0),
        Vec4f(0.0), Vec4f(0.0, 0.5, 0.5, 0.5));

    void setValue(string n, string val)
    {
        if (n == "name")
        {
            name = val;
        }
        else if (n == "voxel")
        {
            voxelPath = val.substr(1, val.size() - 3);
        }
        else if (n == "noise")
        {
            noisePath = val.substr(1, val.size()-3);
        }
        else if (n == "grid")
        {
            grid = tovec3f(val);
        }
        else if (n == "size")
        {
            size = stof(val);
        }
    }

    void updateColor(Vec3f c) {
        data.w.y = c.x;
        data.w.z = c.y;
        data.w.w = c.z;
    }

    void setWind(int wind) {
        data.x.w = wind;
    }

    void initializeCloud() {
        Vec3f bbmin = -((grid  - 1)* size) / 2;
        Vec3f bbmax = ((grid - 1) * size) / 2;

        data = Vec44f(Vec4f(bbmin.x, bbmin.y, bbmin.z, 0),
            Vec4f(bbmax.x, bbmax.y, bbmax.z, 0),
            Vec4f(grid.x, grid.y, grid.z, 0),
            Vec4f(size, data.w.y, data.w.z, data.w.w));
    }
};
