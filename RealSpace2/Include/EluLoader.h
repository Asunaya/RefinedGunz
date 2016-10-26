#pragma once

#include "XMLFileStructs.h"
#include "XMLParser.h"
#include <vector>
#include "gli/gli.hpp"

namespace RealSpace2
{
struct RLIGHT;
class RSolidBspNode;
}

namespace rsx
{

struct EluHeader
{
	u32 Signature;
	u32 Version;
	u32 matCount;
	u32 meshCount;
};

struct DrawProp
{
	u32 vertexBase;
	u32 indexBase;
	// Triangle count
	u32 count;
	// Index into the material vector in an EluObjectData instance
	int material;
};

struct EluMesh
{
	std::unique_ptr<v3[]> Positions;
	std::unique_ptr<v3[]> Normals;
	std::unique_ptr<v2[]> TexCoords;
	std::unique_ptr<v4[]> Tangents;
	std::unique_ptr<u16[]> Indices;

	u32 VertexCount;
	u32 IndexCount;

	std::vector<DrawProp> DrawProps;

	rmatrix World;

	std::string Name;
};

using TextureType = std::string;//gli::texture;

struct EluMaterial
{
	v4 cDiffuse;
	v4 cAmbient;
	v4 cSpecular;
	v4 cEmissive;

	float shininess;
	float roughness;

	TextureType tDiffuse;
	TextureType tNormal;
	TextureType tSpecular;
	TextureType tOpacity;
	TextureType tEmissive;
};

// The data comprising a particular elu
struct EluObjectData
{
	std::string Name;
	rmatrix World;
	std::vector<EluMesh> Meshes;
	int MaterialStart = -1;
};

// An object in the map.
// Contains a reference to the data of the elu it uses
// and a world transform for describing its situation in space.
struct EluObject
{
	int Data = -1;
	rmatrix World;
};

struct LoaderState
{
	std::vector<std::string> Paths;
	std::vector<EluObjectData> ObjectData;
	std::vector<EluObject> Objects;
	std::vector<EluMaterial> Materials;
	std::unordered_map<std::string, int> EluMap;
};

bool loadTree(LoaderState& State, const char * sceneName, std::vector<RealSpace2::RLIGHT>& Lights);
bool loadPropTree(LoaderState& State, const char * propName);

}