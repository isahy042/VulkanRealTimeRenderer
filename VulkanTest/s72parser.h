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
#include <cstdlib> 
#include "camera.h"
#include "mesh.h"
#include "driver.h"
#include "materials.h"

#include "node.h"

using namespace std;


inline string getName(string line)
{
    std::size_t start = line.find('"');
    std::size_t end = line.find('"', start + 1);
    return line.substr(start + 1, end - start - 1);
}

inline string getValue(string line)
{
    if (line.size() == 0) return "";
    std::size_t start = line.find(':');
    std::size_t end = line.size() - 1;
    while (line[end] == ',' || line[end] == '\n') {
        end--;
    }
    return line.substr(start + 1, end - start + 1);
}

class Scene {
public:
    vector<pair<int, int>> s72map; // (type, index in type vector)
    int sceneInd = -1;
    vector<int> roots;

    vector<Camera> cameras;
    Camera *currCam;
    Vec2i env = Vec2i(-1,-1); // index of node, index in the material vector
    Vec3f cameraMovement = Vec3f(0.f);
    bool updateFrustum = true;

    vector<Node> nodes;
    vector<Mesh> meshes;
    vector<Driver> drivers;
    vector<Object> objects;
    vector<shared_ptr<Material>> materials;
    shared_ptr<Environment> envMat;
    
    map<int, int> nodeToObj;

    int totalFrames = 0;

    Scene() {}

    float parseJson(string filepath, string preferredCamera)
    {
        cout << "Parsing the s72 file..";
        string json;
        // ReadFile(filepath, json);
        // std::cout << json;

        std::ifstream file(filepath);
        std::string line;
        int preferredCameraIndex = 0;

        if (!file.is_open()) {
            std::cerr << " failed to open s72 file" << std::endl;
            return 0;
        }

        s72map.push_back(make_pair(-1,-1)); // 0 is invalid
        envMat = make_shared<Environment>();

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

                        cameras.push_back(camera);
                        s72map.push_back(make_pair(3, cameras.size() - 1));
                        //currCam = &cameras[0];

                        if (cameras.back().name == preferredCamera) { 
                            preferredCameraIndex = cameras.size() - 1;    
                        }
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
                        drivers.push_back(driver);
                        s72map.push_back(make_pair(4, drivers.size() - 1));
                    }
                    else if (objName == "MATER") // 5 - material
                    {
                        string name = "";
                        string normalMap = "";
                        string displacementMap = "";
                        getline(file, line);
                        name = line;
                        getline(file, line);
                        if (getName(line) == "normalMap") {
                            normalMap = line;
                            std::getline(file, line);
                        }
                        if (getName(line) == "displacementMap") {
                            displacementMap = line;
                            std::getline(file, line);
                        }
                        string type = getName(line);
                        if (type == "pbr") {
                            shared_ptr<PBR> mat = make_shared<PBR>();
                            mat->setValue("name", getValue(name));
                            mat->setValue("normalMap", getValue(normalMap));
                            mat->setValue("displacementMap", getValue(displacementMap));
                            while (line[0] != '}')
                            {
                                mat->setValue(getName(line), getValue(line));
                                getline(file, line);
                            }
                            materials.push_back(mat);
                            s72map.push_back(make_pair(5, materials.size() - 1));
                        }
                        else if (type == "lambertian") {
                            shared_ptr<Lambertian> mat = make_shared<Lambertian>();
                            mat->setValue("name", getValue(name));
                            mat->setValue("normalMap", getValue(normalMap));
                            mat->setValue("displacementMap", getValue(displacementMap));
                            while (line[0] != '}')
                            {
                                mat->setValue(getName(line), getValue(line));
                                std::getline(file, line);
                            }
                            materials.push_back(mat);
                            s72map.push_back(make_pair(5, materials.size() - 1));
                        }
                        else if (type == "mirror") {
                            shared_ptr<Mirror> mat = make_shared<Mirror>();
                            mat->setValue("name", getValue(name));
                            mat->setValue("normalMap", getValue(normalMap));
                            mat->setValue("displacementMap", getValue(displacementMap));
                           
                            materials.push_back(mat);

                            s72map.push_back(make_pair(5, materials.size() - 1));
                        }
                        else if (type == "environment") {
                            envMat->setValue("name", getValue(name));
                            envMat->setValue("normalMap", getValue(normalMap));
                            envMat->setValue("displacementMap", getValue(displacementMap));
                            materials.push_back(envMat);
                            s72map.push_back(make_pair(5, materials.size() - 1));

                            env.x = s72map.size() - 1;
                        }
                        else if (type == "simple") {
                            shared_ptr<Simple> mat = make_shared<Simple>();
                            mat->setValue("name", getValue(name));
                            mat->setValue("normalMap", getValue(normalMap));
                            mat->setValue("displacementMap", getValue(displacementMap));
                            materials.push_back(mat);
                            s72map.push_back(make_pair(5, materials.size() - 1));
                        }
                    }
                    else if (objName == "ENVIR") // 6 - enviroment
                    {
                        std::getline(file, line); // name
                        std::getline(file, line); // radiance
                        envMat->setValue("radiance", getValue(line));
                        env.y = s72map.size() - 1;
                        s72map.push_back(make_pair(6, -1));
                    }

                }
            }
        }

        // initialize cameras, drivers, meshes
        for (int i = 0; i<meshes.size(); i++) meshes[i].fillAttribute();
        for (int i = 0; i < cameras.size(); i++) cameras[i].initializeProjection();
        for (int i = 0; i < drivers.size(); i++) drivers[i].initializeData();

        currCam = &cameras[preferredCameraIndex];
        return cameras[preferredCameraIndex].aspect;
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

    }

    int instantiateNode(int root, vector<Vec3f> scales, vector<Vec3f> trans, vector<Vec4f> rotates){
        //cout << "Instantiating node " << root << "\n";
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
        //cout << "Instantiating Mesh " << meshAt << "\n";
        // check if mesh
        if (s72map[meshAt].first != 1) {
            printf("wants to instantiate mesh, but given object %d instead. \n", s72map[meshAt].first);
            return 0;
        }
        Object o = Object(meshes[s72map[meshAt].second]);
        
        // transformation matrix
        int transformations = scales.size();
        o.transformMatrix = generateTransformationMatrix(scales, trans, rotates);

        // bounding box
        o.updateBoundingBox();
        objects.push_back(o);
        
        // put current object into the corresponding material's list.
        if (o.mesh.material == -1 || s72map[o.mesh.material].first != 5) cerr << "\n mesh contains invalid material.";
        materials[s72map[o.mesh.material].second]->addObj(objects.size() - 1);

        // map the current object to its node
        nodeToObj[nodeAt] = objects.size() - 1;
    }

    int instantiateCamera(int at, vector<Vec3f> scales, vector<Vec3f> trans, vector<Vec4f> rotates) {
        // check if camera
        if (s72map[at].first != 3) {
            printf("wants to instantiate camera, but given object %d instead. \n", s72map[at].first);
            return 0;
        }
        // TODO: transform camera.
        Vec44f m = generateTransformationMatrix(scales, trans, rotates);
        cameras[s72map[at].second].transformMatrix = m;
        cameras[s72map[at].second].viewMatrix = transpose44(invert44(m));

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
        //scales.push_back(n.scale);
        //rotates.push_back(n.rotate);
        //trans.push_back(n.translate);

        if (n.camera > 0) updateCameraTransformMatrix(n.camera, scales, trans, rotates);
        if (n.mesh > 0) { 
            updateMeshTransformMatrix(root, time, scales, trans, rotates); 

            nodes[s72map[root].second].bbmin.x = min(nodes[s72map[root].second].bbmin.x, objects[nodeToObj[root]].bbmin.x);
            nodes[s72map[root].second].bbmin.y = min(nodes[s72map[root].second].bbmin.y, objects[nodeToObj[root]].bbmin.y);
            nodes[s72map[root].second].bbmin.z = min(nodes[s72map[root].second].bbmin.z, objects[nodeToObj[root]].bbmin.z);

            nodes[s72map[root].second].bbmax.x = max(nodes[s72map[root].second].bbmax.x, objects[nodeToObj[root]].bbmax.x);
            nodes[s72map[root].second].bbmax.y = max(nodes[s72map[root].second].bbmax.y, objects[nodeToObj[root]].bbmax.y);
            nodes[s72map[root].second].bbmax.z = max(nodes[s72map[root].second].bbmax.z, objects[nodeToObj[root]].bbmax.z);
           
        
        }
        if (n.children.size() > 0) {
            for (int& child : n.children) {
                updateNodeTransformMatrix(child, time, scales, trans, rotates);

                nodes[s72map[root].second].bbmin.x = min(nodes[s72map[root].second].bbmin.x, nodes[s72map[child].second].bbmin.x);
                nodes[s72map[root].second].bbmin.y = min(nodes[s72map[root].second].bbmin.y, nodes[s72map[child].second].bbmin.y);
                nodes[s72map[root].second].bbmin.z = min(nodes[s72map[root].second].bbmin.z, nodes[s72map[child].second].bbmin.z);

                nodes[s72map[root].second].bbmax.x = max(nodes[s72map[root].second].bbmax.x, nodes[s72map[child].second].bbmax.x);
                nodes[s72map[root].second].bbmax.y = max(nodes[s72map[root].second].bbmax.y, nodes[s72map[child].second].bbmax.y);
                nodes[s72map[root].second].bbmax.z = max(nodes[s72map[root].second].bbmax.z, nodes[s72map[child].second].bbmax.z);
            }
        }

        
    }

    void updateCameraTransformMatrix(int at, vector<Vec3f> scales, vector<Vec3f> trans, vector<Vec4f> rotates) {
        Vec44f m = generateTransformationMatrix(scales, trans, rotates);
        cameras[s72map[at].second].transformMatrix = matmul4444(transToMatrix4(cameraMovement), m);
        cameras[s72map[at].second].viewMatrix = transpose44(invert44(cameras[s72map[at].second].transformMatrix));
        if (updateFrustum) cameras[s72map[at].second].applyTrasformation();
    }

    void updateMeshTransformMatrix(int nodeAt, int time, vector<Vec3f> scales, vector<Vec3f> trans, vector<Vec4f> rotates) {
        // get obj index
        int index = nodeToObj[nodeAt];
        objects[index].transformMatrix = generateTransformationMatrix(scales, trans, rotates);
        objects[index].updateBoundingBox();
    }
    
    int cull() {
        for (int& root : roots) {
            cullNode(root, true);
        }

        return 1;
    }

    void cullNode(int at, bool intersect) {
        //cout << "updating node. \n";
        Node n = nodes[s72map[at].second];
        if (intersect) intersect = (*currCam).testIntersect(n.bbmax, n.bbmin);

        if (n.mesh > 0) {
            int index = nodeToObj[at];
            if (intersect) objects[index].inFrame = (*currCam).testIntersect(objects[index].bbmax, objects[index].bbmin);
            else objects[index].inFrame = false;
        }
        if (n.children.size() > 0) {
            for (int& child : n.children) {
                cullNode(child, intersect);
            }
        }

       
    }

    Vec44f generateTransformationMatrix(vector<Vec3f> scales, vector<Vec3f> trans, vector<Vec4f> rotates) {
        Vec44f transformation = identity44();
        Vec44f temp;
        if (scales.size() != trans.size() || rotates.size() != trans.size()) cout << "transformation matrix error.";
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