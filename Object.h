#pragma once

#include <fstream>
#include <sstream>
#include <stdexcept>>
#include <string>
#include <vector>

#include "Triangle.h"

/* --------------- Public Structs/Enums --------------- */
typedef struct materialInfoRGBStruct
{
	double r;
	double g;
	double b;
};

struct textureInfoStruct
{
	std::string Name;
	int offset;
	int textureHeight;
	int textureWidth;
	short bitsPerPixel;
};

typedef struct materialInfoStruct
{
	std::string Name; // Material Name
	std::string map_Kd; // Texture File
	materialInfoRGBStruct Ka; // Ambient color
	materialInfoRGBStruct Kd; // Diffuse color
	materialInfoRGBStruct Ks; // Specular color
	materialInfoRGBStruct Ke; // Emissive Color
	double Tf; // Transmission Filter Color
	union
	{
		double d; // Dissolve
		double Tr; // Transparency (1- d)
	};
	double Ni; // Optical density
	int Ns; // Specular highlights
	int Sharpness; // sharpness of relfections
	int illum; // Illum type

	textureInfoStruct textureInfo;
	std::vector<unsigned char> texture;
};

class Object
{
public:	
	/* --------------- Public Members --------------- */
	std::vector<Triangle> triangles;
	std::vector<materialInfoStruct> materials;

	/* --------------- Public Constructor --------------- */
	Object(std::string objectFile)
	{
		triangles = {};
		materials = {};

		loadObject(objectFile);
	}

	/* --------------- Public Functions --------------- */
	void loadObject(const std::string objectFile)
	{
		if (objectFile == "") throw std::runtime_error("Failed to open object file\n");

		std::vector<Coords> verticies;
		std::vector<Coords> vTextures;
		std::vector<Vector> vNormals;
		std::string curMaterial = "";
		int curMaterialIndex = 0;

		std::ifstream obj(objectFile);
		std::string line;

		while (std::getline(obj, line))
		{
			std::istringstream iss(line);
			std::string type;
			double v1, v2, v3;

			iss >> type;

			if (type == "v")
			{
				iss >> v1 >> v2 >> v3;
				verticies.push_back({ v1, v3, v2 }); // blender defines z as up; I define y as up
			}
			else if (type == "vt")
			{
				iss >> v1 >> v2;
				vTextures.push_back({ v1, v2, -1 });
			}
			else if (type == "vn")
			{
				iss >> v1 >> v2 >> v3;
				vNormals.push_back({ v1, v3, v2 }); // blender defines z as up; I define y as up
			}
			else if (type == "f")
			{
				std::string face;
				int v1, v2, v3;
				int vt1, vt2, vt3;
				int vn1, vn2, vn3;

				iss >> face;
				int itemsRead = std::sscanf(face.c_str(), "%d/%d/%d", &v1, &vt1, &vn1);
				if (itemsRead != 3) throw std::runtime_error("Failed to parse line\n");

				iss >> face;
				itemsRead = std::sscanf(face.c_str(), "%d/%d/%d", &v2, &vt2, &vn2);
				if (itemsRead != 3) throw std::runtime_error("Failed to parse line\n");

				iss >> face;
				itemsRead = std::sscanf(face.c_str(), "%d/%d/%d", &v3, &vt3, &vn3);
				if (itemsRead != 3) throw std::runtime_error("Failed to parse line\n");

				Triangle tri = {
					verticies[v1 - 1], verticies[v2 - 1], verticies[v3 - 1],
					vTextures[vt1 - 1], vTextures[vt2 - 1], vTextures[vt3 - 1],
					vNormals[vn1 - 1], vNormals[vn2 - 1], vNormals[vn3 - 1],
					curMaterialIndex
				};
				triangles.push_back(tri);

				while (iss >> face)
				{
					v2 = v3;
					vt2 = vt3;
					vn2 = vn3;

					iss >> face;
					itemsRead = std::sscanf(face.c_str(), "%d/%d/%d", &v3, &vt3, &vn3);
					if (itemsRead != 3) throw std::runtime_error("Failed to parse line\n");

					tri = {
						verticies[v1 - 1], verticies[v2 - 1], verticies[v3 - 1],
						vTextures[vt1 - 1], vTextures[vt2 - 1], vTextures[vt3 - 1],
						vNormals[vn1 - 1], vNormals[vn2 - 1], vNormals[vn3 - 1],
						curMaterialIndex
					};
					triangles.push_back(tri);
				}
			}
			else if (type == "mtllib")
			{
				std::string mtlFile;
				iss >> mtlFile;
				loadMaterial(mtlFile);
			}
			else if (type == "usemtl")
			{
				iss >> curMaterial;
				for (unsigned long i = 0; i < materials.size(); i++)
				{
					if (curMaterial == materials[i].Name)
					{
						curMaterialIndex = i;
						break;
					}
				}
			}
			else continue;
		}

		obj.close();
		return;
	}

	void loadMaterial(const std::string materialFile)
	{
		if (materialFile == "") throw std::runtime_error("Failed to open material file");

		long index = materials.size() - 1;
		std::ifstream material(materialFile);
		std::string line;

		while (std::getline(material, line))
		{
			std::istringstream iss(line);
			std::string type;

			iss >> type;

			if (type == "newmtl")
			{
				index++;
				materials.push_back({});
				iss >> materials[index].Name;
			}
			else if (type == "d")
			{
				double d;
				iss >> d;

				materials[index].d = d;
			}
			else if (type == "Ns")
			{
				double Ns;
				iss >> Ns;

				materials[index].Ns = Ns;
			}
			else if (type == "Ni")
			{
				double Ni;
				iss >> Ni;
				materials[index].Ni = Ni;
			}
			else if (type == "Tr")
			{
				double Tr;
				iss >> Tr;

				materials[index].Tr = Tr;
			}
			else if (type == "Ka")
			{
				double r, g, b;
				iss >> r >> g >> b;

				materials[index].Ka.r = r;
				materials[index].Ka.g = g;
				materials[index].Ka.b = b;
			}
			else if (type == "Ks")
			{
				double r, g, b;
				iss >> r >> g >> b;

				materials[index].Ks.r = r;
				materials[index].Ks.g = g;
				materials[index].Ks.b = b;
			}
			else if (type == "Ke")
			{
				double r, g, b;
				iss >> r >> g >> b;

				materials[index].Ke.r = r;
				materials[index].Ke.g = g;
				materials[index].Ke.b = b;
			}
			else if (type == "Kd")
			{
				double r, g, b;
				iss >> r >> g >> b;

				materials[index].Kd.r = r;
				materials[index].Kd.g = g;
				materials[index].Kd.b = b;
			}
			else if (type == "illum")
			{
				int illum;
				iss >> illum;
				materials[index].illum = illum;
			}
			else if (type == "map_Kd")
			{
				iss >> materials[index].map_Kd;
				loadTexture(materials[index]);
			}
			else continue;
		}
	}

	void loadTexture(materialInfoStruct& materialStruct)
	{
		std::string textureFile = materialStruct.map_Kd;
		if (textureFile.substr(textureFile.find('.')) != ".bmp") throw std::runtime_error("Not a bmp file");

		std::ifstream texture(textureFile , std::ios::binary);
		if (!texture) throw std::runtime_error("Failed to open texture file\n");

		int offset = 0; // offset to move straight to pixel data
		texture.seekg(10, std::ios::beg); // move file pointer to place where data points to pixel data
		texture.read((char*)(&offset), 4);

		int textureWidth = 0;
		texture.seekg(18, std::ios::beg); // move file pointer to place where data is about width
		texture.read((char*)(&textureWidth), 4);

		int textureHeight = 0;
		texture.seekg(22, std::ios::beg); // move file pointer to place where data is about height
		texture.read((char*)(&textureHeight), 4);

		short bitsPerPixel = 0;
		texture.seekg(28, std::ios::beg); // move file pointer to place where data talks about number of bits for a pixel
		texture.read((char*)(&bitsPerPixel), 2);

		int bytes = bitsPerPixel / 8;
		std::vector<unsigned char>& textureVec = materialStruct.texture;
		textureVec.resize(textureWidth * textureHeight * 3);

		texture.seekg(offset);
		for (unsigned long i = 0; i < textureVec.size(); i += 3)
		{
			std::vector<uint8_t> bgra(bytes);
			texture.read(reinterpret_cast<char*>(bgra.data()), bytes);

			textureVec[i] = bgra[0];
			textureVec[i + 1] = bgra[1];
			textureVec[i + 2] = bgra[2];
		}

		texture.close();

		materialStruct.textureInfo.offset = offset;
		materialStruct.textureInfo.textureHeight = textureHeight;
		materialStruct.textureInfo.textureWidth = textureWidth;
		materialStruct.textureInfo.bitsPerPixel = bitsPerPixel;

		return;
	}
};