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

};

class Environment : public Material {
public:
	string name;
	string type = "env";
	string vertShader = "vert.spv";
	string fragShader = "env-frag.spv";
	vector<int> objInstance;

	string normalMap = "[0.5,0.5,1]"; // default value
	string displacementMap = "[0,0,0]";

	string radiance;

	void setValue(string n, string val) override {
		if (n == "name")
		{
			name = val.substr(1, val.size() - 3);
		}
		else if (n == "radiance")
		{
			if (val[0] == '[' || val[val.size() - 1] == ']') {
				radiance = val;
			}
			else {
				size_t start = val.find(':') + 2;
				size_t end = val.find('"', start);
				radiance = val.substr(start, end - start);
			}
		}
		else if (n == "normalMap")
		{
			if (val.size() != 0) {
				size_t start = val.find(':') + 2;
				size_t end = val.find('"', start);
				normalMap = val.substr(start, end - start);
			}
		}
		else if (n == "displacementMap")
		{
			if (val.size() != 0) {
				size_t start = val.find(':') + 2;
				size_t end = val.find('"', start);
				displacementMap = val.substr(start, end - start);
			}
		}
	}
	string getVertshader() override { return vertShader; }
	string getFragshader() override { return fragShader; }
	vector<int> getObjList() override { return  objInstance; }
	string getType() override{ return type; };
	string getBaseColor() override{ return radiance; };
	string getDisplacement() override { return displacementMap; }
	string getNormal() override { return normalMap; }

	void addObj(int obj) { objInstance.push_back(obj);  }
};

class Mirror : public Material {
public:
	string name;
	string type = "mirror";
	string vertShader = "vert.spv";
	string fragShader = "mirror-frag.spv";
	vector<int> objInstance;

	string normalMap = "[0.5,0.5,1]"; // default value
	string displacementMap = "[0,0,0]";

	void setValue(string n, string val) override {
		if (n == "name")
		{
			name = val.substr(1, val.size() - 3);
		}
		else if (n == "normalMap")
		{
			if (val.size() != 0) {
				size_t start = val.find(':') + 2;
				size_t end = val.find('"', start);
				normalMap = val.substr(start, end - start);
			}
		}
		else if (n == "displacementMap")
		{
			if (val.size() != 0) {
				size_t start = val.find(':') + 2;
				size_t end = val.find('"', start);
				displacementMap = val.substr(start, end - start);
			}
		}

	}
	string getVertshader() override { return vertShader; }
	string getFragshader() override { return fragShader; }
	vector<int> getObjList() override { return  objInstance; }
	string getType()  override { return type; }
	string getDisplacement() override { return displacementMap; }
	string getNormal() override { return normalMap; }

	void addObj(int obj) { objInstance.push_back(obj); }

};

class PBR : public Material {
public:
	string name;
	string type = "pbr";
	string vertShader = "vert.spv";
	string fragShader = "pbr-frag.spv";

	string albedo;
	string roughness;
	string metalness;

	string normalMap = "[0.5,0.5,1]"; // default value
	string displacementMap = "[0,0,0]";
	vector<int> objInstance;

	string getVertshader() override { return vertShader; }
	string getFragshader() override { return fragShader; }
	vector<int> getObjList() override { return  objInstance; }
	string getType() override { return type; };

	string getBaseColor() override { return albedo; };
	string getRoughness() override { return roughness; };
	string getMetalness() override { return metalness; };

	string getDisplacement() override { return displacementMap; }
	string getNormal() override { return normalMap; }

	void setValue(string n, string val) override {
		if (n == "name")
		{
			name = val.substr(1, val.size() - 3);
		}
		else if (n == "albedo")
		{
			if (val[0] == '[' || val[val.size() - 1] == ']') {
				albedo = val;
			}
			else {
				size_t start = val.find(':') + 2;
				size_t end = val.find('"', start);
				albedo = val.substr(start, end - start);
			}
		}
		else if (n == "roughness")
		{
			if (val[0] == '{' || val[1] == '{' || val[val.size() - 1] == '}') {
				size_t start = val.find(':') + 2;
				size_t end = val.find('"', start);
				roughness = val.substr(start, end - start);
			}
			else {
				string r = to_string(stof(val));
				roughness = "[" + r + "," + r + "," + r + "]";
			}
		}
		else if (n == "metalness")
		{
			if (val[0] == '{' || val[1] == '{' || val[val.size() - 1] == '}') {
				size_t start = val.find(':') + 2;
				size_t end = val.find('"', start);
				metalness = val.substr(start, end - start);
			}
			else {
				string m = to_string(stof(val));
				metalness = "[" + m + "," + m + "," + m + "]";
			}

		}
		else if (n == "normalMap")
		{
			if (val.size() != 0) {
				size_t start = val.find(':') + 2;
				size_t end = val.find('"', start);
				normalMap = val.substr(start, end - start);
			}
		}
		else if (n == "displacementMap")
		{
			if (val.size() != 0) {
				size_t start = val.find(':') + 2;
				size_t end = val.find('"', start);
				displacementMap = val.substr(start, end - start);
			}
		}

	}

	void addObj(int obj) { objInstance.push_back(obj); }

};

class Lambertian :public Material {
public:
	string name;
	string type = "lamb";
	string vertShader = "vert.spv";
	string fragShader = "lamb-frag.spv";
	string albedo;

	string normalMap = "[0.5,0.5,1]"; // default value
	string displacementMap = "[0,0,0]";
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
		else if (n == "normalMap")
		{
			if (val.size() != 0) {
				size_t start = val.find(':') + 2;
				size_t end = val.find('"', start);
				normalMap = val.substr(start, end - start);
			}
		}
		else if (n == "displacementMap")
		{
			if (val.size() != 0) {
				size_t start = val.find(':') + 2;
				size_t end = val.find('"', start);
				displacementMap = val.substr(start, end - start);
			}
		}
	}

	string getVertshader() override { return vertShader; }
	string getFragshader() override { return fragShader; }
	vector<int> getObjList() override { return  objInstance; }
	string getType()  override { return type; };
	string getBaseColor() override { return albedo; };

	string getDisplacement() override { return displacementMap; }
	string getNormal() override { return normalMap; }

	void addObj(int obj) { objInstance.push_back(obj); }

};

class Simple : public Material {
public:
	string name;
	string type = "simple";
	string vertShader = "vert.spv";
	string fragShader = "simple-frag.spv";

	string normalMap = "[0.5,0.5,1]"; // default value
	string displacementMap = "[0,0,0]";
	vector<int> objInstance;

	string getVertshader() override { return vertShader; }
	string getFragshader() override { return fragShader; }
	vector<int> getObjList() override { return  objInstance; }
	string getType()  override { return type; };

	string getDisplacement() override { return displacementMap; }
	string getNormal() override { return normalMap; }

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
void makeLambertianCubeMap(string inFilename) {
	int inWidth, inHeight, texChannels;
	string in = "s72-main/examples/" + inFilename;
	string outFilename = "lambertian-map-" + inFilename;

	const char* inc = in.c_str();
	stbi_uc* pixels = stbi_load(inc, &inWidth, &inHeight, &texChannels, STBI_rgb_alpha);
	cout << inHeight << " " << inWidth;

	if (inHeight / inWidth != 6) cerr << "please input image with correct dimension (wx6w)";
	
	// store in Vec4f, 2D vector
	vector<vector<Vec4f>> inImg(inHeight, vector<Vec4f>(inWidth, Vec4f(0.f)));

	int p = 0;
	cout << "\n generating cube map for file " << inFilename;

	for (int h = 0; h < inHeight; h++) {
		for (int w = 0; w < inWidth; w++) {
			inImg[h][w] = Vec4f((float)(pixels[(p * 4)]) / 255.f, (float)(pixels[(p * 4) + 1]) / 255.f, (float)(pixels[(p * 4) + 2]) / 255.f, (float)(pixels[(p * 4) + 3]) / 255.f);
			p++;
		}
	}

	//  Vec4f, 2D vector to store new map
	const int outWidth = 40;
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
			int validNum = 2000;
			
			for (int s = 0; s < 2000; s++) {
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
			outImg[h][w].w = 0.502;
		}
	}


	stbi_uc outImgArr[outWidth * outWidth * 4 * 6];

	p = 0;
	for (int h = 0; h < outWidth * 6; h++) {
		for (int w = 0; w < outWidth; w++) {
			// if not edge, blur.
			if (w != 0 && w!= outWidth-1 && h % outWidth != 0 && (h+1) % outWidth != 0) {

				Vec4f t = outImg[h][w];
				/*(outImg[h - 1][w - 1] + outImg[h - 1][w] + outImg[h - 1][w + 1]
						+ outImg[h][w - 1] + outImg[h][w] + outImg[h][w + 1] +
						outImg[h + 1][w-1] + outImg[h + 1][w] + outImg[h+1][w + 1])/9.f;*/

				outImgArr[(p * 4)] = clip((int)floor(t.x * 255.f), 0, 255);
				outImgArr[(p * 4) + 1] = clip((int)floor(t.y * 255.f), 0, 255);
				outImgArr[(p * 4) + 2] = clip((int)floor(t.z * 255.f), 0, 255);
				outImgArr[(p * 4) + 3] = clip((int)floor(t.w * 255.f), 0, 255);
			}
			else {
				outImgArr[(p * 4)] = clip((int)floor((outImg[h][w].x * 255.f)), 0, 255);
				outImgArr[(p * 4) + 1] = clip((int)floor((outImg[h][w].y * 255.f)), 0, 255);
				outImgArr[(p * 4) + 2] = clip((int)floor((outImg[h][w].z * 255.f)), 0, 255);
				outImgArr[(p * 4) + 3] = clip((int)floor((outImg[h][w].w * 255.f)), 0, 255);
			}
			p++;
		}
	}

	string on = "s72-main/examples/" + outFilename;
	const char* onc = on.c_str();
	//stbi__create_png_image();
	stbi_write_png(onc, outWidth, outWidth * 6, 4, outImgArr, 4 * outWidth);

}


// Make mipmap levels for pbr
// combine mipmap with shaders to achieve the effect
// support normal maps
// tone mapping
// complete rest of normal map support
// create scene
// write report
// analyze run time
// 

//https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
Vec3f ImportanceSampleGGX(Vec2f Xi, float Roughness, Vec3f N)
{
	float a = Roughness * Roughness;
	float Phi = 2 * 3.1415926 * Xi.x;
	float CosTheta = sqrt((1 - Xi.y) / (1 + (a * a - 1) * Xi.y));
	float SinTheta = sqrt(1 - CosTheta * CosTheta);
	Vec3f H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;
	Vec3f UpVector = abs(N.z) < 0.999 ? Vec3f(0, 0, 1) : Vec3f(1, 0, 0);
	Vec3f TangentX = normalize(cross(UpVector, N));
	Vec3f TangentY = cross(N, TangentX);
	return TangentX * H.x + TangentY * H.y + N * H.z;
}

// helper functions for material cubemap parsing.
void makePBRCubeMap(string inFilename, float roughness, int index) {
	cout << "\n generating cube map for file " << inFilename << " at roughness " << roughness;
	int inWidth, inHeight, texChannels;
	string in = "s72-main/examples/" + inFilename;
	string outFilename = "pbr-map-" + to_string(index) + "-" + inFilename;

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
	int outWidth = 32 / int(pow(2, index));

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
			Vec3f viewing = normal;
			float totalWeight = 0;

			for (int s = 0; s < 1000; s++) {

				Vec2f Xi = Vec2f(randf(), randf());
				Vec3f H = ImportanceSampleGGX(Xi, roughness, normal);
				Vec3f scatterDir = 2 * dot(viewing, H) * H - viewing;

				Vec2f index = getIndexForCubeMap(scatterDir);

				int iy = clip((int)floor(index.x * (inHeight - 1)), 0, inHeight - 1);
				int ix = clip((int)floor(index.y * (inWidth - 1)), 0, inWidth - 1);

				Vec4f c = inImg[iy][ix];

				float NoL = clip(dot(normal, scatterDir), 0, 1);

				if (!(NoL <= 1) && !(NoL >= 0)) cout << "\n NoL" << NoL;

				if (NoL > 0)
				{
					sampledColor = sampledColor + c * NoL;
					totalWeight += NoL;
				}
			}

			outImg[h][w] = sampledColor / totalWeight;
			//outImg[h][w].w = 1.f;
		}
	}
	

	stbi_uc outImgArr[32 * 32 * 4 * 6]; // allocate this for now, may not use whole array
	p = 0;
	for (int h = 0; h < outWidth * 6; h++) {
		for (int w = 0; w < outWidth; w++) {

			outImgArr[(p * 4)] = clip((int)floor((outImg[h][w].x * 255.f)), 0, 255);
			outImgArr[(p * 4) + 1] = clip((int)floor((outImg[h][w].y * 255.f)), 0, 255);
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

// is this right???
float G_Smith(float Roughness, float NoV, float NoL) {
	float k = (Roughness + 1) * (Roughness + 1) / 8.0f;
	float G_V = NoV / (NoV * (1.0f - k) + k);
	float G_L = NoL / (NoL * (1.0f - k) + k);
	return G_V * G_L;
}

void makePBRLUT() {
	string outFilename = "pbr-map.png";

	//  Vec4f, 2D vector to store new map
	int outWidth = 32;

	vector<vector<Vec4f>> outImg(outWidth);
	for (int h = 0; h < outWidth; h++) {
		outImg[h].resize(outWidth);
	}

	Vec3f V;
	Vec3f normal = Vec3f(0, 0, 1);
	float roughness = 0.f;
	float roughnessIncrement = (1 / (outWidth - 1));
	float NoV = 0.f;
	float NoVIncrement = (2 * 3.1415926 / (outWidth - 1));

	for (int h = 0; h < outWidth; h++) {
		for (int w = 0; w < outWidth; w++) {
			V.x = sqrt(1.0f - NoV * NoV); // sin
			V.y = 0;
			V.z = NoV; // cos

			float A = 0;
			float B = 0;

			for (int s = 0; s < 1000; s++) {

				Vec2f Xi = Vec2f(randf(), randf());
				Vec3f H = ImportanceSampleGGX(Xi, roughness, normal);
				Vec3f L = 2 * dot(V, H) * H - V;

				float NoL = clamp(L.z,0.f,1.f);
				float NoH = clamp(H.z, 0.f, 1.f);
				float VoH = clamp(dot(V, H), 0.f, 1.f);

				if (NoL > 0)
				{
					float G = G_Smith(roughness, NoV, NoL);
					float G_Vis = G * VoH / (NoH * NoV);
					float Fc = pow(1 - VoH, 5);
					A += (1 - Fc) * G_Vis;
					B += Fc * G_Vis;
				}

			}

			Vec2f finalSamples = Vec2f(A, B) / 1000;
			outImg[h][w] = Vec4f(finalSamples.x, finalSamples.y, 0.f,0.f);
			NoV += NoVIncrement;
		}
		NoV = 0.f;
		roughness += roughnessIncrement;
	}
	

	stbi_uc outImgArr[32 * 4 * 32]; // allocate this for now, may not use whole array
	int p = 0;
	for (int h = 0; h < outWidth; h++) {
		for (int w = 0; w < outWidth; w++) {

			outImgArr[(p * 4)] = clip((int)floor((outImg[h][w].x * 255.f)), 0, 255);
			outImgArr[(p * 4) + 1] = clip((int)floor((outImg[h][w].y * 255.f)), 0, 255);
			outImgArr[(p * 4) + 2] = 0;
			outImgArr[(p * 4) + 3] = 255;
			p++;
		}
	}

	string on = "s72-main/examples/" + outFilename;
	const char* onc = on.c_str();
	//stbi__create_png_image();

	stbi_write_png(onc, outWidth, outWidth, 4, outImgArr, 4 * outWidth);

}