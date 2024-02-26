#pragma once
#include "vector.h"
#include <iostream>
#include <vector>
#include <string>

#include "helper.h"


class CubeMap {
	int n = -1;
	vector<vector<Vec3f>> cmap;

	CubeMap(Vec3f color) {
	}

	CubeMap(string file) {
	}

	Vec3f query(Vec3f) {
		return Vec3f(0.f);

	}
};
class Material {
public:

	virtual ~Material() {}

	virtual void setValue(string n, string val) {};

	string name;
	string type;
	string normalMap;
	string displacementMap;

};

class Environment : public Material {
public:
	string name;
	string type = "env";
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
	};
};

class Mirror : public Material {
public:


};

class PBR : public Material {
public:
	string type = "pbr";

};

class Lambertian :public  Material {
public:
	Vec3f baseColor = Vec3f(0.f);


};

class Simple : public Material {
public:


};