#pragma once
#include "vector.h"
#include <iostream>
#include <vector>
#include <string>

#include "helper.h"


class Material {
public:

	virtual ~Material() {}

	virtual void setValue(string n, string val) {};
	virtual string getVertshader() { return ""; };
	virtual string getFragshader() { return ""; };
	virtual vector<int> getObjList() { return vector<int>(); };
	virtual void addObj(int obj) {};



	//string name;
	//string type;
	//string vertShader = "vert.spv";
	//string fragShader;

	//string normalMap;
	//string displacementMap;
	//vector<int> objInstance;

};

class Environment : public Material {
public:
	string name;
	string type = "env";
	string vertShader = "vert.spv";
	string fragShader = "env-frag.spv";
	vector<int> objInstance;

	string normalMap;
	string displacementMap;

	string radiance;

	void setValue(string n, string val) override {
		if (n == "name")
		{
			name = val.substr(1, val.size() - 3);
		}
		else if (n == "radiance")
		{
			size_t start = val.find(':') + 2;
			size_t end = val.find('"', start);
			radiance = val.substr(start, end-start);
			cout << radiance;
		}
	}
	string getVertshader() override { return vertShader; }
	string getFragshader() override { return fragShader; }
	vector<int> getObjList() override { return  objInstance; }
	void addObj(int obj) { objInstance.push_back(obj);  }
};

class Mirror : public Material {
public:
	string name;
	string type = "mirror";
	string vertShader = "vert.spv";
	string fragShader = "mirror-frag.spv";
	vector<int> objInstance;

	string normalMap;
	string displacementMap;

	void setValue(string n, string val) override {
		if (n == "name")
		{
			name = val.substr(1, val.size() - 3);
		}
	}
	string getVertshader() override { return vertShader; }
	string getFragshader() override { return fragShader; }
	vector<int> getObjList() override { return  objInstance; }
	void addObj(int obj) { objInstance.push_back(obj); }


};

class PBR : public Material {
public:
	string name;
string type = "pbr";
string vertShader = "vert.spv";
string fragShader;

string normalMap;
string displacementMap;
vector<int> objInstance;

};

class Lambertian :public  Material {
public:
	Vec3f baseColor = Vec3f(0.f);
	string name;
	string type;
	string vertShader = "vert.spv";
	string fragShader;

	string normalMap;
	string displacementMap;
	vector<int> objInstance;

};

class Simple : public Material {
public:
	string name;
	string type;
	string vertShader = "vert.spv";
	string fragShader;

	string normalMap;
	string displacementMap;
	vector<int> objInstance;

};