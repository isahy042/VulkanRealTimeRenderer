/***
 * This file parses the s72 files, and contains any additional helper functions that may be needed.
 */
 // scene, node, mesh, camea, driver
#pragma once

#include <iostream>
#include <fstream> // for std::ifstream

#include <string>
#include <vector>
#include <map>

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
    Camera *currCam;
    vector<Node> nodes;
    vector<Mesh> meshes;
    vector<Driver> drivers;
    vector<Object> objects;
    map<int, int> nodeToObj;

    int totalFrames = 0;

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
                        camera.initializeProjection();
                        cameras.push_back(camera);
                        s72map.push_back(make_pair(3, cameras.size() - 1));
                    }
                    else if (objName == "DRIVE") // 4 - driver
                    {
                        Driver driver = Driver();
                        std::getline(file, line); // name
                        while (line[0] != '}')
                        {
                            driver.setValue(getName(line), getValue(line));
                            std::getline(file, line);
                        }
                        driver.initializeData();
                        drivers.push_back(driver);
                        s72map.push_back(make_pair(4, drivers.size() - 1));
                    }

                }
            }
        }
        return 1;
    }

    // first traversal of graph - constructing it
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

       // bind drivers
       for (int d = 0; d < drivers.size(); d++) {
           Driver drive = drivers[d];
           int n = drive.node;
           if (s72map[n].first != 2) {
               printf("wants to drive a node, driving something else. \n");
               return 0;
           }

           if (drive.channel == 0) {
               nodes[s72map[n].second].driveTranslate = true;
               nodes[s72map[n].second].transDriver = d;
           }
           else if (drive.channel == 1) {
               nodes[s72map[n].second].driveRotate = true;
               nodes[s72map[n].second].rotateDriver = d;
           }
           else if (drive.channel == 2) { 
               nodes[s72map[n].second].driveScale = true; 
               nodes[s72map[n].second].scaleDriver = d;
           }
           
           else printf("translation channel not supported.");

           totalFrames = std::max(totalFrames, static_cast<int>(ceil(drive.times.back() * FPS)));
       }
       if (drivers.size() == 0) totalFrames = 2;
       currCam = &cameras[0];
    }

    int instantiateNode(int root, vector<Vec3f> scales, vector<Vec3f> trans, vector<Vec4f> rotates){
        cout << "Instantiating node " << root << "\n";
        Node n = nodes[s72map[root].second];

        // push transformations onto vectors
        scales.push_back(n.scale);
        trans.push_back(n.translate);
        rotates.push_back(n.rotate);

        if (n.camera > 0) instantiateCamera(n.camera, scales, trans, rotates);
        if (n.mesh > 0) instantiateMesh(n.mesh, root, scales, trans, rotates);
        if (n.children.size() > 0) {
            for (int& child : n.children) {
                instantiateNode(child, scales, trans, rotates);
            }
        }

        return 1;
    }

    // construct graph!
    int instantiateMesh(int meshAt, int nodeAt, vector<Vec3f> scales, vector<Vec3f> trans, vector<Vec4f> rotates) {
        cout << "Instantiating Mesh " << meshAt << "\n";
        // check if mesh
        if (s72map[meshAt].first != 1) {
            printf("wants to instantiate mesh, but given object %d instead. \n", s72map[meshAt].first);
            return 0;
        }
        Object o = Object(meshes[s72map[meshAt].second]);
        // TODO: condense into a single transformation matrix, and perform a single matmul with homogenous coordinates.
        int transformations = scales.size();
        o.transformMatrix = generateTransformationMatrix(scales, trans, rotates);
        o.updateBoundingBox();
        objects.push_back(o);

        cout << "\n setting" << nodeAt << " " << objects.size() - 1;
        nodeToObj[nodeAt] = objects.size() - 1;
    }

    int instantiateCamera(int at, vector<Vec3f> scales, vector<Vec3f> trans, vector<Vec4f> rotates) {
        // check if camera
        if (s72map[at].first != 3) {
            printf("wants to instantiate camera, but given object %d instead. \n", s72map[at].first);
            return 0;
        }
        // TODO: transform camera.
        cameras[s72map[at].second].transformMatrix = generateTransformationMatrix(scales, trans, rotates);
        cameras[s72map[at].second].viewMatrix = transpose44(invert44(cameras[s72map[at].second].transformMatrix));

        return 1;

    }

    // traverse graph!
    void updateSceneTransformMatrix(int time) {
        // perform graph traversal, assuming no back edge.
        for (int& root : roots) {
            vector<Vec3f> scales;
            vector<Vec3f> trans;
            vector<Vec4f> rotates;
            updateNodeTransformMatrix(root, time, scales, trans, rotates);
        }
    }

    void updateNodeTransformMatrix(int root, int time, vector<Vec3f> scales, vector<Vec3f> trans, vector<Vec4f> rotates) {

        //cout << "updating node. \n";
        Node n = nodes[s72map[root].second];
        //cout << "examing node " << root <<". scale translate rotate: " << n.driveScale <<n.driveTranslate<<n.driveRotate <<"\n";
        // push transformations onto vectors
        if (n.driveScale) {
            scales.push_back(drivers[n.scaleDriver].getScale(time));
        }
        else {
            scales.push_back(n.scale);
        }
        if (n.driveTranslate) {
            trans.push_back(drivers[n.transDriver].getTranslate(time));
        }
        else {
            trans.push_back(n.translate);
        }
        if (n.driveRotate) {
            rotates.push_back(drivers[n.rotateDriver].getRotate(time));
        }
        else {
            rotates.push_back(n.rotate);
        }
       // scales.push_back(n.scale);
       // rotates.push_back(n.rotate);
        //trans.push_back(n.translate);

        if (n.camera > 0) updateCameraTransformMatrix(n.camera, scales, trans, rotates);
        if (n.mesh > 0) updateMeshTransformMatrix(root, scales, trans, rotates);
        if (n.children.size() > 0) {
            for (int& child : n.children) {
                updateNodeTransformMatrix(child, time, scales, trans, rotates);
            }
        }
    }

    void updateCameraTransformMatrix(int at, vector<Vec3f> scales, vector<Vec3f> trans, vector<Vec4f> rotates) {
        cameras[s72map[at].second].transformMatrix = generateTransformationMatrix(scales, trans, rotates);
        cameras[s72map[at].second].viewMatrix = transpose44(invert44(cameras[s72map[at].second].transformMatrix));
    }

    void updateMeshTransformMatrix(int nodeAt, vector<Vec3f> scales, vector<Vec3f> trans, vector<Vec4f> rotates) {
        // get obj index
        int index = nodeToObj[nodeAt];
        objects[index].transformMatrix = generateTransformationMatrix(scales, trans, rotates);
        objects[index].updateBoundingBox();
    }
    
    int cull() {
        for (Object & obj : objects) {
            obj.inFrame = (*currCam).testIntersect(obj.bbmax, obj.bbmin);
        }
    }

    Vec44f generateTransformationMatrix(vector<Vec3f> scales, vector<Vec3f> trans, vector<Vec4f> rotates) {
        Vec44f transformation = identity44();
        Vec44f temp;
        while (!rotates.empty()) {
            // applying its scale, rotation, and translation values (in that order)
            transformation = matmul4444(scaleToMatrix4(scales.back()), transformation);
            temp = matmul4444(transToMatrix4(trans.back()), quaternionToMatrix4(rotates.back()));  
            transformation = matmul4444(temp, transformation);
            scales.pop_back();
            trans.pop_back();
            rotates.pop_back();
        }

        return transformation;
    }
    };