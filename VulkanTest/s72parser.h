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

using namespace std;

/**
To do list (Finish by Wed.):
Json parser (load)
Loading and viewing

// for later
Vector Library (3 hr)
Report (3 hr)
*/



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
class Scene {
public:
    vector<pair<int, int>> s72map; // (type, index in type vector)
    int sceneInd = -1;
    vector<int> roots;

    vector<Camera> cameras;
    Camera currCam;
    vector<Node> nodes;
    vector<Mesh> meshes;
    vector<Driver> drivers;
    vector<Object> objects;

    Scene() {}

    int parseJson(string filepath)
    {
        cout << "Parsing the s72 file..";
        string json;
        // ReadFile(filepath, json);
        // std::cout << json;

        std::ifstream file(filepath);
        std::string line;

        if (!file.is_open()) {
            std::cerr << " failed to open s72 file" << std::endl;
            return 0;
        }

        s72map.push_back(make_pair(-1,-1)); // 0 is invalid

        while (std::getline(file, line))
        {
            if (line[0] == '{')
            {
                // new object declared.
                std::getline(file, line);
                if (line.substr(2, 4) == "type")
                {
                    string objName = line.substr(9, 5);
                    cout << "finding object with name " << objName << "\n";
                    // scene, node, mesh, camera, driver
                    if (objName == "SCENE") // 0 - scene
                    {
                        std::getline(file, line); // name
                        std::getline(file, line); // roots
                        s72map.push_back(make_pair(0, 0));
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
                        s72map.push_back(make_pair(1, meshes.size() - 1));
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
                        s72map.push_back(make_pair(2, nodes.size() - 1));
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
                        camera.buildProjectionAndViewMatrix();
                        cameras.push_back(camera);
                        s72map.push_back(make_pair(3, cameras.size() - 1));
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
        return 1;
    }

    int InstantiateObjects() {
       cout << "Instantiating Objects.. \n";
       if (sceneInd == -1) { 
            throw string("read s72 file before instantiating objects. \n");
            return 0;
       }

       // perform graph traversal, assuming no back edge.
       for (int& root : roots) {
           vector<Vec3f> scales;
           vector<Vec3f> trans;
           vector<Vec4f> rotates;
           instantiateNode(root, scales, trans, rotates);
       }


    }

    int instantiateNode(int root, vector<Vec3f> scales, vector<Vec3f> trans, vector<Vec4f> rotates){
        cout << "Instantiating node " << root << "\n";
        Node n = nodes[s72map[root].second];

        // push transformations onto vectors
        scales.push_back(n.scale);
        trans.push_back(n.translate);
        rotates.push_back(n.rotate);

        if (n.camera > 0) instantiateCamera(n.camera, scales, trans, rotates);
        if (n.mesh > 0) instantiateMesh(n.mesh, scales, trans, rotates);
        if (n.children.size() > 0) {
            for (int& child : n.children) {
                instantiateNode(child, scales, trans, rotates);
            }
        }

        return 1;
    }

    // construct graph!
    int instantiateMesh(int at, vector<Vec3f> scales, vector<Vec3f> trans, vector<Vec4f> rotates) {
        cout << "Instantiating Mesh " << at << "\n";
        // check if mesh
        if (s72map[at].first != 1) {
            printf("wants to instantiate mesh, but given object %d instead. \n", s72map[at].first);
            return 0;
        }
        Object o = Object(meshes[s72map[at].second]);
        // TODO: condense into a single transformation matrix, and perform a single matmul with homogenous coordinates.
        int transformations = scales.size();


        while(!rotates.empty()) {
            // applying its scale, rotation, and translation values (in that order)
            o.applyTransformation(scales.back(), trans.back(),rotates.back());
            scales.pop_back();
            trans.pop_back();
            rotates.pop_back();
        }
        objects.push_back(o);
        

    }

    int instantiateCamera(int at, vector<Vec3f> scales, vector<Vec3f> trans, vector<Vec4f> rotates) {
        // check if camera
        if (s72map[at].first != 3) {
            printf("wants to instantiate camera, but given object %d instead. \n", s72map[at].first);
            return 0;
        }

        Camera c = cameras[s72map[at].second];
        // TODO: transform camera.
        
        return 1;

    }

    int cull() {
        for (Object & obj : objects) {
            obj.culled = currCam.testIntersect(obj.bbmax, obj.bbmin);
        }
    }

    };