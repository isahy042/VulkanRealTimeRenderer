/***
 * This file parses the s72 files, and contains any additional helper functions that may be needed.
 */
 // scene, node, mesh, camea, driver
#pragma once

#include <iostream>
#include <fstream> // for std::ifstream

#include <string>
#include <vector>

#include "camera.h"
#include "mesh.h"
#include "driver.h"
#include "node.h"
#include "object.h"


using namespace std;

vector<tuple<int, int>> s72map; // (type, index in type vector)
int sceneInd = 0;
vector<int> roots;

vector<Camera> cameras;
vector<Node> nodes;
vector<Mesh> meshes;
vector<Driver> drivers;
vector<Object> objects;

inline string getName(string line)
{
    std::size_t start = line.find('"');
    std::size_t end = line.find('"', start + 1);
    return line.substr(start + 1, end - start - 1);
}

inline string getValue(string line)
{
    std::size_t start = line.find(':');
    std::size_t end = line.size() - 1;
    while (line[end] == ',' || line[end] == '\n') {
        end--;
    }
    return line.substr(start + 1, end - start + 1);
}

//inline float extractFloat(string line)
//{
//    std::size_t start = line.find(':');
//    std::size_t end = line.find(',', start + 1);
//    if (start == string::npos || end == string::npos)
//    {
//        // error.
//    }
//    return std::stof(line.substr(start + 1, end - start - 2));
//}
//
//inline string extractstring(string line)
//{
//    std::size_t start = line.find(':');
//    std::size_t end = line.find('"', start + 2);
//    if (start == string::npos || end == string::npos)
//    {
//        // error.
//    }
//    return line.substr(start + 2, end - start - 3);
//}


void parseJson(string filepath)
{
    string json;
    // ReadFile(filepath, json);
    // std::cout << json;

    std::ifstream file(filepath);
    std::string line;

    while (std::getline(file, line))
    {

        if (line[0] == '{')
        {
            // new object declared.
            std::getline(file, line);
            if (line.substr(2, 4) == "type")
            {
                string objName = line.substr(9, 5);
                // scene, node, mesh, camera, driver
                if (objName == "SCENE") // 0 - scene
                {
                    std::getline(file, line); // name
                        std::getline(file, line); // roots
                        s72map.push_back(make_tuple(0,0));
                        sceneInd = s72map.size() - 1;
                        roots = tovector(getValue(line));
                  
                }
                else if (objName == "MESH\"") // 1 - mesh
                {
                    Mesh mesh = Mesh();
                    std::getline(file, line); // name
                    while (line[0] != '}')
                    {
                        mesh.setValue(getName(line), getValue(line));
                        std::getline(file, line);
                    }
                    mesh.fillAttribute();
                    meshes.push_back(mesh);
                    s72map.push_back(make_tuple(1, meshes.size()-1));
                }
                else if (objName == "NODE\"") // 2 - node
                {
                    Node node = Node();
                    std::getline(file, line); // name
                    while (line[0] != '}')
                    {
                        node.setValue(getName(line), getValue(line));
                        std::getline(file, line);
                    }
                    nodes.push_back(node);
                    s72map.push_back(make_tuple(2, nodes.size() - 1));
                }
                else if (objName == "CAMER") // 3 - camera
                {           
                    Camera camera = Camera();
                    std::getline(file, line); // name
                    camera.setValue(getName(line), getValue(line));
                    std::getline(file, line); // perspective
                    std::getline(file, line);
                    while (line[1] != '}')
                    {
                        camera.setValue(getName(line), getValue(line));
                        std::getline(file, line);
                    }
                    cameras.push_back(camera);
                    s72map.push_back(make_tuple(3, cameras.size()-1));
                }
                else if (objName == "DRIVE")
                {
                    Driver driver = Driver();
                    std::getline(file, line); // name
                    while (line[0] != '}')
                    {
                        driver.setValue(getName(line), getValue(line));
                        std::getline(file, line);
                    }
                }
        
            }
        }
    }
}
