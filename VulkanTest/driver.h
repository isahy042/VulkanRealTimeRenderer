#pragma once

#include "vector.h"
#include "helper.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;


class Driver
{
public:
    Driver() {}

    void setValue(string n, string val)
    {
        if (n == "node")
        {
            node = stoi(val);
        }
        else if (n == "channel")
        {
            val = val.substr(1, val.size() - 2);
            if (val == "translation") channel = 0;
            else if (val == "rotation") channel = 1;
            else if (val == "scale") channel = 2;
            else printf("translation channel not supported.");
        }
        else if (n == "times")
        {
            times = tovectorf(val);
        }
        else if (n == "values")
        {
            values = tovectorf(val);
        }
        else if (n == "interpolation")
        {
            val = val.substr(1, val.size() - 2);
            if (val == "STEP") channel = 0;
            else if (val == "LINEAR") channel = 1;
            else if (val == "SLERP") channel = 2;
            else printf("translation mpde not supported.");
        }
    }
    

private:
    int node = -1;
    int channel = -1; // translate, rotate, scale
    vector<float> times;
    vector<float> values;
    int mode = 1; // step, linear, slerp

};