#pragma once

#include "vector.h"
#include "helper.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

// Assuming lights are not scaled.
// Assuming sun light is unique.

class Light
{
public:
    virtual ~Light() {}
    virtual void setValue(string n, string val) {};
    virtual void setTransformationMatrix(Vec44f m) {};
    virtual Vec44f getTransformationMatrix() { return Vec44f(Vec4f(0.f)); };
    virtual Vec44f getProjMatrix() { return Vec44f(Vec4f(0.f)); };
    virtual Vec44f getDataMatrix() { return Vec44f(Vec4f(0.f)); };
    virtual Vec44f getViewMatrix() { return Vec44f(Vec4f(0.f)); };
    virtual int getType() { return -1; };
    virtual int getShadow() { return -1; };
};

class Sun : public Light {
public:
    string name = "";
    Vec3f tint = Vec3f(1.f);
    Vec3f pos = Vec3f(0.f);
    Vec3f direction = Vec3f(0, 0, -1);
    Vec44f projMat = Vec44f(Vec4f(0.f));
    Vec44f transMat = Vec44f(Vec4f(0.f));
    Vec44f viewMat = Vec44f(Vec4f(0.f));

    int type = 2;

    float angle;
    float strength;

    int shadow = 256;

    void setValue(string n, string val) override
    {
        if (n == "name")
        {
            name = val;
        }
        else if (n == "tint")
        {
            tint = tovec3f(val);
        }
        else if (n == "shadow")
        {
            shadow = stoi(val);
        }
        else if (n == "angle")
        {
            angle = stof(val);
        }
        else if (n == "strength")
        {
            strength = stof(val);
        }
    }
    void setTransformationMatrix(Vec44f m) override {
        pos = transformPos(m, Vec3f(0.f));
        direction = transformDir(m, Vec3f(0, 0, -1));
        transMat = m;
        viewMat = transpose44(invert44(m));
    }
    Vec44f getTransformationMatrix() override {
        return transMat;
    }
    Vec44f getProjMatrix() override {
        return projMat;
    }
    Vec44f getViewMatrix() override {
        return viewMat;
    }
    Vec44f getDataMatrix() override {
        return Vec44f(Vec4f(1.f, tint.x, tint.y, tint.z), 
            Vec4f(shadow, angle, strength, 0.f), 
            Vec4f(direction.x, direction.y, direction.z,0.f),
            Vec4f(0.f));
    }
    int getType() override { return type; };
    int getShadow() override { return shadow; };

};

class Sphere : public Light {
public:
    string name = "";
    Vec3f tint = Vec3f(1.f);
    Vec3f pos = Vec3f(0.f);;
    Vec44f transMat = Vec44f(Vec4f(0.f));
    Vec44f projMat = Vec44f(Vec4f(0.f));
    Vec44f viewMat = Vec44f(Vec4f(0.f));

    int type = 0;
    int shadow = 256;

    float radius;
    float power;
    float limit = 1.f;


    void setValue(string n, string val) override
    {
        if (n == "name")
        {
            name = val;
        }
        else if (n == "tint")
        {
            tint = tovec3f(val);
        }
        else if (n == "shadow")
        {
            shadow = stoi(val);
        }
        else if (n == "radius")
        {
            radius = stof(val);
        }
        else if (n == "power")
        {
            power = stof(val);
        }
        else if (n == "limit")
        {
            limit = stof(val);
        }
    }
    void setTransformationMatrix(Vec44f m) override {
        pos = transformPos(m, Vec3f(0.f));
        transMat = m;
        viewMat = transpose44(invert44(m));

    }
    Vec44f getTransformationMatrix() override {
        return transMat;
    }
    Vec44f getProjMatrix() override {
        return projMat;
    }
    Vec44f getViewMatrix() override {
        return viewMat;
    }
    Vec44f getDataMatrix() override {
        return Vec44f(Vec4f(1.f, tint.x, tint.y, tint.z),
            Vec4f(shadow, radius, power, limit),
            Vec4f(0.f),
            Vec4f(0.f));
    }
    int getType() override { return type; };
    int getShadow() override { return shadow; };


};

class Spot : public Light {
public:
    string name = "";
    Vec3f tint = Vec3f(1.f);
    Vec3f pos = Vec3f(0.f);
    Vec3f direction = Vec3f(0,0,-1);
    Vec44f transMat = Vec44f(Vec4f(0.f));
    Vec44f projMat = Vec44f(Vec4f(0.f));
    Vec44f viewMat = Vec44f(Vec4f(0.f));

    int type = 1;
    int shadow = 256;

    float radius;
    float power;
    float limit = 1.f;
    float fov;
    float blend;

    void setValue(string n, string val) override
    {
        if (n == "name")
        {
            name = val;
        }
        else if (n == "tint")
        {
            tint = tovec3f(val);
        }
        else if (n == "shadow")
        {
            shadow = stoi(val);
        }
        else if (n == "radius")
        {
            radius = stof(val);
        }
        else if (n == "power")
        {
            power = stof(val);
        }
        else if (n == "limit")
        {
            limit = stof(val);
        }
        else if (n == "fov")
        {
            fov = stof(val);

            float scaleVfov = 1 / std::tan(fov / 2.0f);
            projMat[0][0] = scaleVfov;
            projMat[1][1] = scaleVfov;
            projMat[2][2] = -(100) / ((100) - 0.01);
            projMat[2][3] = -1.0f;
            projMat[3][2] = -((100) * 0.01) / ((100) - 0.01);
        }
        else if (n == "blend")
        {
            blend = stof(val);
        }
    }
    void setTransformationMatrix(Vec44f m) override {
        pos = transformPos(m, Vec3f(0.f));
        direction = transformDir(m, Vec3f(0, 0, -1));
        transMat = m;
        viewMat = transpose44(invert44(m));

    }
    Vec44f getTransformationMatrix() override {
        return transMat;
    }
    Vec44f getProjMatrix() override {
        return projMat;
    }
    Vec44f getViewMatrix() override {
        return viewMat;
    }
    Vec44f getDataMatrix() override {
        return Vec44f(Vec4f(1.f, tint.x, tint.y, tint.z),
            Vec4f(shadow, radius, power, limit),
            Vec4f(fov, blend, 0.f,0.f),
            Vec4f(direction.x, direction.y, direction.z, 0.0f));
    }

    int getType() override { return type; };
    int getShadow() override { return shadow; };


};