#pragma once
#include "vector.h"
#include <iostream>
#include <vector>
#include <string>

#include "helper.h"

// texture
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"



class Material {
public:

	virtual ~Material() {}

	virtual void setValue(string n, string val) {};
	virtual string getVertshader() { return ""; };
	virtual string getFragshader() { return ""; };
	virtual string getType() { return "generic"; };
	virtual string getDisplacement() { return ""; };
	virtual string getNormal() { return ""; };
	virtual string getBaseColor() { return ""; };
	virtual string getRoughness() { return ""; };
	virtual string getMetalness() { return ""; };



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
			if (val[0] == '[' || val[val.size()-1] == ']') {
				radiance = val;
			}
			else {
			size_t start = val.find(':') + 2;
			size_t end = val.find('"', start);
			radiance = val.substr(start, end - start);
			}
		}
	}
	string getVertshader() override { return vertShader; }
	string getFragshader() override { return fragShader; }
	vector<int> getObjList() override { return  objInstance; }
	string getType() override{ return type; };
	string getBaseColor() override{ return radiance; };

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
	string getType()  override { return type; }

	void addObj(int obj) { objInstance.push_back(obj); }

};

class PBR : public Material {
public:
	string name;
	string type = "pbr";
	string vertShader = "vert.spv";
	string fragShader;

	string albedo;
	string rougness;
	string metalness;

	string normalMap;
	string displacementMap;
	vector<int> objInstance;

	string getVertshader() override { return vertShader; }
	string getFragshader() override { return fragShader; }
	vector<int> getObjList() override { return  objInstance; }
	string getType() override { return type; };

	string getBaseColor() override { return albedo; };
	string getRoughness() override { return rougness; };
	string getMetalness() override { return metalness; };

	void addObj(int obj) { objInstance.push_back(obj); }

};

class Lambertian :public  Material {
public:
	string name;
	string type = "lamb";
	string vertShader = "vert.spv";
	string fragShader = "lamb-frag.spv";
	string albedo;

	string normalMap;
	string displacementMap;
	vector<int> objInstance;

	void setValue(string n, string val) override {
		if (n == "name")
		{
			name = val.substr(1, val.size() - 3);
		}
		else if (n == "albedo")
		{
			if (val[0] == '[' || val[val.size()-1] == ']') {
				albedo = val;
			}
			else {
				size_t start = val.find(':') + 2;
				size_t end = val.find('"', start);
				albedo = val.substr(start, end - start);
			}
			cout << albedo << "\n";
		}
	}

	string getVertshader() override { return vertShader; }
	string getFragshader() override { return fragShader; }
	vector<int> getObjList() override { return  objInstance; }
	string getType()  override { return type; };
	string getBaseColor() override { return albedo; };

	void addObj(int obj) { objInstance.push_back(obj); }

};

class Simple : public Material {
public:
	string name;
	string type = "simple";
	string vertShader = "vert.spv";
	string fragShader;

	string normalMap;
	string displacementMap;
	vector<int> objInstance;

	string getVertshader() override { return vertShader; }
	string getFragshader() override { return fragShader; }
	vector<int> getObjList() override { return  objInstance; }
	string getType()  override { return type; };

	void addObj(int obj) { objInstance.push_back(obj); }

};

Vec2f getIndexForCubeMap(Vec3f normal) {
	int face = -1;
	float absX = abs(normal.x);
	float absY = abs(normal.y);
	float absZ = abs(normal.z);

	float maxAxis = std::max(absX, std::max(absY, absZ));

	if (maxAxis == absX) {
		face = normal.x > 0.f ? 0 : 1;
	}
	else if (maxAxis == absY) {
		face = normal.y > 0.f ? 2 : 3;
	}
	else {
		face = normal.z > 0.f ? 4 : 5;
	}

	float s, t;
	switch (face) {
	case 0:
		s = -normal.z / absX;
		t = -normal.y / absX;
		break;
	case 1:
		s = normal.z / absX;
		t = -normal.y / absX;
		break;
	case 2:
		s = normal.x / absY;
		t = normal.z / absY;
		break;
	case 3:
		s = normal.x / absY;
		t = -normal.z / absY;
		break;
	case 4:
		s = normal.x / absZ;
		t = -normal.y / absZ;
		break;
	case 5:
		s = -normal.x / absZ;
		t = -normal.y / absZ;
		break;
	}

	// Normalize texture coordinates to [0, 1] range
	s = (s + 1.0f) / 2.0f;
	t = ((t + 1.0f) / 12.0f) + (face * (1.f/6.f));

	return Vec2f(t, s);
}

// From Chapter 8 of PBR Textbook
class OrthonormalBasis {
public:
	OrthonormalBasis() {
		for (int i = 0; i < 3; i++) {
			axis.push_back(Vec3f(0.f));
		}
	}

	Vec3f operator[](int i) const { return axis[i]; }
	Vec3f& operator[](int i) { return axis[i]; }

	Vec3f u() const { return axis[0]; }
	Vec3f v() const { return axis[1]; }
	Vec3f w() const { return axis[2]; }

	Vec3f local(float a, float b, float c) const { return a * u() + b * v() + c * w(); }

	Vec3f local(const Vec3f& a) const { return a.x * u() + a.y * v() + a.z * w(); }

	void build_from_w(const Vec3f& w) {
		Vec3f unit_w = normalize(w);
		Vec3f a = (fabs(unit_w.x) > 0.9) ? Vec3f(0, 1, 0) : Vec3f(1, 0, 0);
		Vec3f v = normalize(cross(unit_w, a));
		Vec3f u = cross(unit_w, v);
		axis[0] = u;
		axis[1] = v;
		axis[2] = unit_w;
	}

public:
	vector<Vec3f> axis;
};

// node: this code is from homework2 of PBR (written by me)
Vec3f randomOnUnitHemisphere(Vec3f normal) {
	float theta = 2 * 3.1415926f * randf();
	float cosPhi = randf();
	float sinPhi = sqrt(1 - cosPhi * cosPhi);

	float x = cos(theta) * sinPhi;
	float y = sin(theta) * sinPhi;
	float z = cosPhi;

	OrthonormalBasis obn = OrthonormalBasis();
	obn.build_from_w(normal);

	return normalize(obn.local(Vec3f(x, y, z)));
}

// helper functions for material cubemap parsing.
void makeLambertianCubeMap(string inFilename, string outFilename) {
	int inWidth, inHeight, texChannels;
	string in = "s72-main/examples/" + inFilename;
	const char* inc = in.c_str();
	stbi_uc* pixels = stbi_load(inc, &inWidth, &inHeight, &texChannels, STBI_rgb_alpha);

	if (inHeight / inWidth != 6) cerr << "please input image with correct dimension (wx6w)";
	
	// store in Vec4f, 2D vector
	vector<vector<Vec4f>> inImg(inHeight, vector<Vec4f>(inWidth, Vec4f(0.f)));

	int p = 0;

	for (int h = 0; h < inHeight; h++) {
		for (int w = 0; w < inWidth; w++) {
			inImg[h][w] = Vec4f((float)(pixels[(p * 4)]) / 255.f, (float)(pixels[(p * 4) + 1]) / 255.f, (float)(pixels[(p * 4) + 2]) / 255.f, (float)(pixels[(p * 4) + 3]) / 255.f);
			p++;
		}
	}

	//  Vec4f, 2D vector to store new map
	const int outWidth = 32;
	vector<vector<Vec4f>> outImg(outWidth * 6);
	for (int h = 0; h < outWidth * 6; h++) {
		outImg[h].resize(outWidth);
	}

	// for each normal, sample 100 times and average
	float dx = 2.f / (float)outWidth;
	float dy = 2.f / (float)outWidth;
	float dz = 2.f / (float)outWidth;
	Vec3f normal = Vec3f(0.f);
	int cubeIndex = 0;

	for (int h = 0; h < outWidth * 6; h++) {
		// generate normal
		if (h % outWidth == 0) {
			cubeIndex = h / outWidth;
			if (cubeIndex == 0) normal = Vec3f(1.f);
			else if (cubeIndex == 1) normal = Vec3f(-1.f, 1.f, -1.f);
			else if (cubeIndex == 2) normal = Vec3f(-1.f, 1.f, -1.f);
			else if (cubeIndex == 3) normal = Vec3f(-1.f, -1.f, 1.f);
			else if (cubeIndex == 4) normal = Vec3f(-1.f, 1.f, 1.f);
			else if (cubeIndex == 5) normal = Vec3f(1.f, 1.f, -1.f);
		}

		if (cubeIndex == 0) { 
			normal[1] -= dy; 
			normal[2] = 1.f;
		}
		else if (cubeIndex == 1) { 
			normal[1] -= dy; 
			normal[2] = -1.f;
		}
		else if (cubeIndex == 2) {
			normal[2] += dz; 
			normal[0] = -1.f;
		}
		else if (cubeIndex == 3) { 
			normal[2] -= dz; 
			normal[0] = -1.f;
		}
		else if (cubeIndex == 4) { 
			normal[1] -= dy;
			normal[0] = -1.f;
		}
		else if (cubeIndex == 5) { 
			normal[1] -= dy; 
			normal[0] = 1.f;
		}

		for (int w = 0; w < outWidth; w++) {
			// generate normal
			if (cubeIndex == 0) normal[2] -= dz;
			else if (cubeIndex == 1) normal[2] += dz;
			else if (cubeIndex == 2) normal[0] += dx;
			else if (cubeIndex == 3) normal[0] += dx;
			else if (cubeIndex == 4) normal[0] += dx;
			else if (cubeIndex == 5) normal[0] -= dx;

			Vec4f sampledColor = Vec4f(0.f);
			int validNum = 1000;
			
			for (int s = 0; s < 1000; s++) {
				Vec3f scatterDir = randomOnUnitHemisphere(normal);
				float cosineTheta = dot(normalize(scatterDir), normalize(normal));
				if (cosineTheta <= 0) {
					validNum -= 1;
					continue;
				}
				// calculate corresponding pixel in input file, standarizing at 10 away for now.
				Vec2f index = getIndexForCubeMap(scatterDir);
				int iy = clip((int)floor(index.x * (inHeight - 1)), 0, inHeight - 1);
				int ix = clip((int)floor(index.y * (inWidth - 1)), 0, inWidth - 1);
				
				Vec4f c = inImg[iy][ix];
				sampledColor = sampledColor + (c * cosineTheta);
			}	

			outImg[h][w] = 2 * sampledColor / (float)validNum;
			outImg[h][w].w = 1.f;
		}
	}


	stbi_uc outImgArr[outWidth * outWidth * 4 * 6];

	p = 0;
	for (int h = 0; h < outWidth * 6; h++) {
		for (int w = 0; w < outWidth; w++) {
			
			outImgArr[(p * 4)] = clip((int)floor((outImg[h][w].x * 255.f)), 0, 255);
			outImgArr[(p * 4) + 1] =clip((int)floor((outImg[h][w].y * 255.f)), 0, 255);
			outImgArr[(p * 4) + 2] = clip((int)floor((outImg[h][w].z * 255.f)), 0, 255);
			outImgArr[(p * 4) + 3] = clip((int)floor((outImg[h][w].w * 255.f)), 0, 255);
			p++;
		}
	}

	string on = "s72-main/examples/" + outFilename;
	const char* onc = on.c_str();
	//stbi__create_png_image();
	stbi_write_png(onc, outWidth, outWidth * 6, 4, outImgArr, 4 * outWidth);

}
