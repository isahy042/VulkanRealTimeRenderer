/**
 * Mesh.
 */

#pragma once

#include "vector.h"
#include "helper.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

/// currently assuming all attributes files are the same.
string folder = "C:/Users/Sasa/Desktop/Spring2024/672Graphics/s72-main/s72-main/examples/";

class Mesh
{
public:
    Mesh() {}

    void setValue(string n, string val)
    {
        if (n == "name")
        {
            name = val;
        }
        else if (n == "topology")
        {
            // default is "TRIANGLE_LIST"
        }
        else if (n == "count")
        {
            count = stoi(val);
            cout << "\nmesh " << name << "is set to " << count;
        }
        else if (n == "indicies")
        {
            // index
            inputIndices = true;
            std::size_t start = val.find(':');
            std::size_t end = val.find('"', start + 2);
            indexFile = folder +  val.substr(start + 2, end - start - 2);
            start = val.find(':', end + 1);
            end = val.find(',', start + 1);
            indexoffset = stoi(val.substr(start + 1, end - start));
            start = val.find(':', end + 1);
            end = val.find('"', start + 2);
            string format = val.substr(start + 2, end - start - 2);
            if (!support(indexoffset, 0, format, indexoffset, 0, "UINT32")) {
                cout << "data is " << indexoffset << format << "UINT32";
                cout << "unsupported indices encountered for a mesh!";
            }
        }
        else if (n == "POSITION")
        {
            std::size_t start = val.find(':');
            std::size_t end = val.find('"', start + 2);
            string filename = val.substr(start + 2, end - start - 2);
            start = val.find(':', end + 1);
            end = val.find(',', start + 1);
            int offset = stoi(val.substr(start + 1, end - start));
            start = val.find(':', end + 1);
            end = val.find(',', start + 1);
            int stride = stoi(val.substr(start + 1, end - start));
            start = val.find(':', end + 1);
            end = val.find('"', start + 2);
            string format = val.substr(start + 2, end - start - 2);
            if (!support(offset, stride, format, 0, 28, "R32G32B32_SFLOAT")) {
                cout << "data is " << offset << stride << format << 0 << 28 << "R32G32B32_SFLOAT";
                cout << "unsupported attributes encountered for a mesh!";
            }

        }
        else if (n == "NORMAL")
        {
            std::size_t start = val.find(':');
            std::size_t end = val.find('"', start + 2);
            string filename = val.substr(start + 2, end - start - 2);
            start = val.find(':', end + 1);
            end = val.find(',', start + 1);
            int offset = stoi(val.substr(start + 1, end - start));
            start = val.find(':', end + 1);
            end = val.find(',', start + 1);
            int stride = stoi(val.substr(start + 1, end - start));
            start = val.find(':', end + 1);
            end = val.find('"', start + 2);
            string format = val.substr(start + 2, end - start - 2);
            if (!support(offset, stride, format, 12, 28, "R32G32B32_SFLOAT")) {
                cout << "data is " << offset << stride << format << 12 << 28 << "R32G32B32_SFLOAT";
                cout << "unsupported attributes encountered for a mesh!";
            }

        }
        else if (n == "COLOR")
        {
            std::size_t start = val.find(':');
            std::size_t end = val.find('"', start + 2);
            string filename = val.substr(start + 2, end - start - 2);
            attributesFile = folder + filename;
            start = val.find(':', end + 1);
            end = val.find(',', start + 1);
            int offset = stoi(val.substr(start + 1, end - start));
            start = val.find(':', end + 1);
            end = val.find(',', start + 1);
            int stride = stoi(val.substr(start + 1, end - start));
            start = val.find(':', end + 1);
            end = val.find('"', start + 2);
            string format = val.substr(start + 2, end - start - 2);
            if (!support(offset, stride, format, 24, 28, "R8G8B8A8_UNORM")) {
                cout << "data is " << offset << stride << format << 24 << 28 << "R8G8B8A8_UNORM";
                cout << "unsupported attributes encountered for a mesh!";
            }
        }
    }

    void fillAttribute() {
        ifstream infile(attributesFile, ifstream::binary);
        float f1;
        float f2;
        float f3;
        unsigned char c1;
        unsigned char c2;
        unsigned char c3;
        unsigned char c4;
        size_t v = 0;

        if (!infile.is_open()) {
            std::cerr << " failed to open attribute file" << std::endl;
        }

        while (infile.read(reinterpret_cast<char*>(&f1), sizeof(f1))) {
           
            infile.read(reinterpret_cast<char*>(&f2), sizeof(f2));
            infile.read(reinterpret_cast<char*>(&f3), sizeof(f3));
            position.push_back(Vec3f(f1, f2, f3));
            bbmax.x = max(bbmax.x, f1);
            bbmax.y = max(bbmax.y, f2);
            bbmax.z = max(bbmax.z, f3);
            bbmin.x = min(bbmin.x, f1);
            bbmin.y = min(bbmin.y, f2);
            bbmin.z = min(bbmin.z, f3);
            infile.read(reinterpret_cast<char*>(&f1), sizeof(f1));
            infile.read(reinterpret_cast<char*>(&f2), sizeof(f2));
            infile.read(reinterpret_cast<char*>(&f3), sizeof(f3));
            normal.push_back(Vec3f(f1, f2, f3));
            infile.read(reinterpret_cast<char*>(&c1), sizeof(c1));
            infile.read(reinterpret_cast<char*>(&c2), sizeof(c2));
            infile.read(reinterpret_cast<char*>(&c3), sizeof(c3));
            infile.read(reinterpret_cast<char*>(&c4), sizeof(c4));
            color.push_back(Vec4uc(c1, c2, c3, c4));
            v++;
        }

        infile.close();
       
        if (v != count) { 
            cout << "Something is wrong in mesh vertices count. v: " << v << " count: " << count << "\n";}

        if (inputIndices) {
            ifstream infile(indexFile, ifstream::binary);
            uint32_t i;
            while (!infile.eof()) {
                infile.read(reinterpret_cast<char*>(&i), sizeof(i));
                indices.push_back(i); // this is hopefully not a shallow copy.
            }
            infile.close();
        }
        else {
            for (uint32_t i = 0; i < v; i++) {
                indices.push_back(i);
            }
        }

    }

    string name = "";
    int topology = 0; // default is "TRIANGLE_LIST"
    int count = 0;

    vector<uint32_t> indices;
    vector<Vec3f> position;
    vector<Vec3f> normal;
    vector<Vec4uc> color;

    Vec3f bbmax = Vec3f(-INFINITY);
    Vec3f bbmin = Vec3f(INFINITY);

private:
        bool inputIndices = false;

        string indexFile = "";
        int indexoffset = 0;
        string attributesFile = "";

    bool support(int o, int s, string f, int oo, int ss, string ff) {
        if ((o != oo) || (s != ss) || (f != ff)) return false;
        return true;
    }
};