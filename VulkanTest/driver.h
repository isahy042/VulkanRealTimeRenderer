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
            val = val.substr(1, val.size() - 3);
            
            if (val == "translation") channel = 0;
            else if (val == "rotation") channel = 1;
            else if (val == "scale") channel = 2;
            else  cout << "translation channel not supported. " << val;

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
            if (val == "STEP") mode = 0;
            else if (val == "LINEAR") mode = 1;
            else if (val == "SLERP") mode = 2;
            else cout << "interpolation method not supported. " << val;
        }
    }
    
    void initializeData() {
        intervals = times.size();
        int s = values.size();
        if (channel == 1) { // rotate
            s = s / 4;
            for (int i = 0; i < s; i++) {
                values4.push_back(Vec4f(values[i*4], values[1 + (i * 4)], values[2 + (i * 4)], values[1 + (i * 4)]));
            }
        } else if (channel == 0 || channel == 2) { // rotate
            s = s / 3;
            for (int i = 0; i < s; i++) {
                values3.push_back(Vec3f(values[i * 3], values[1 + (i * 3)], values[2 + (i * 3)]));
            }
        }
        else {
            printf("\ndriver initialize data failed due to wrong channel value.\n");
        }
    }

    Vec3f getTranslate(int time) {
        if (channel != 0) printf("\nwrong driver channel. Attempting to translate.\n");

        if (mode == 0) return step3(time);
        else if (mode == 1) return linear3(time);
        else if (mode == 2) return slerp3(time);

        printf("get translate failed.");
        return Vec3f(0.f);
    }
    Vec4f getRotate(int time) {
        if (channel != 1) printf("\nwrong driver channel. Attempting to rotate.\n");

        if (mode == 0) return step4(time);
        else if (mode == 1) return linear4(time);
        else if (mode == 2) return slerp4(time);

        printf("get rotation failed.");

        Vec4f v = Vec4f(0.f);
        v[3] = 1;
        return v;

    }
    Vec3f getScale(int time) {
        if (channel != 2) printf("\nwrong driver channel. Attempting to scale.\n");

        if (mode == 0) return step3(time);
        else if (mode == 1) return linear3(time);
        else if (mode == 2) return slerp3(time);
        printf("get scale failed.");

        return Vec3f(1.f);

    }


    int node = -1;
    int channel = -1; // translate, rotate, scale

    vector<float> times;
private:

    int findInterval(int time) {
        float t = static_cast<float>(time) / FPSi;
        if (times[0] >= t) return -1;
        else if (times.back() <= t) return intervals - 1;
        for (int i = 1; i < intervals; i++) {
            if (t <= times[i]) return i - 1;
        }
        // shouldn't reach here
        return -1;
    }

    Vec3f step3(int time) {
        int interval = findInterval(time);
        if (interval == -1) interval = 0;
        return values3[interval];
    }

    Vec4f step4(int time) {
        int interval = findInterval(time);
        if (interval == -1) interval = 0;
        return values4[interval];
    }

    Vec3f linear3(int time) {
        int interval = findInterval(time);
        if (interval == -1) return values3[0];
        else if (interval == intervals - 1) return values3[interval];

        // linearly interpolate
        float intervalSize = times[interval + 1] - times[interval];

        float t = static_cast<float>(time) / FPSi;
        float weight1 = (t - times[interval]) / intervalSize;
        float weight2 = 1 - weight1;

        return (weight1 * values3[interval]) + (weight2 * values3[interval + 1]);
    }

    Vec4f linear4(int time) {
        int interval = findInterval(time);
        if (interval == -1) return values4[0];
        else if (interval == intervals - 1) return values4[interval];

        // linearly interpolate
        float intervalSize = times[interval + 1] - times[interval];

        float t = static_cast<float>(time) / FPSi;
        float weight1 = (t - times[interval]) / intervalSize;
        float weight2 = 1 - weight1;

        return (weight1 * values4[interval]) + (weight2 * values4[interval + 1]);
    }

    Vec3f slerp3(int time) {
        return step3(time);
    }
    Vec4f slerp4(int time) {
        return step4(time);
    }

    vector<float> values;
    vector<Vec3f> values3;
    vector<Vec4f> values4;

    int mode = 1; // step, linear, slerp
    int intervals = 0;

};