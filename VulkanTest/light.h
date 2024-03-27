#pragma once

#include "vector.h"
#include "helper.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;


class Light
{
public:
    virtual ~Light() {}
    virtual void setValue(string n, string val) {};
};

class Sun : public Light {
public:
    string name = "";
    Vec3f tint = Vec3f(1.f);
    int type = 0;

    float angle;
    float strength;

    int shadow = 0;

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
};

class Sphere : public Light {
public:
    string name = "";
    Vec3f tint = Vec3f(1.f);
    int type = 0;
    int shadow = 0;

    float radius;
    float power;
    float limit;


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
};

class Spot : public Light {
public:
    string name = "";
    Vec3f tint = Vec3f(1.f);
    int type = 0;
    int shadow = 0;

    float radius;
    float power;
    float limit;
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
        }
        else if (n == "blend")
        {
            blend = stof(val);
        }
    }
};