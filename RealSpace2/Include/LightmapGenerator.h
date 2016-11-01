#pragma once

#include "RNameSpace.h"
#include "GlobalTypes.h"

_NAMESPACE_REALSPACE2_BEGIN

struct RCONVEXPOLYGONINFO;
struct RLightList;
class RBspObject;
struct RLIGHT;
struct RLIGHTMAPTEXTURE;

using RFREEBLOCKLIST = std::list<POINT>;

class RBspLightmapManager {
public:
	RBspLightmapManager();

	int GetSize() const { return m_nSize; }
	auto * GetData() { return m_pData.get(); }

	void SetSize(int nSize) { m_nSize = nSize; }
	void SetData(std::unique_ptr<u32[]> pData) { m_pData = std::move(pData); }

	bool Add(const u32 * data, int nSize, POINT * retpoint);
	bool GetFreeRect(int nLevel, POINT *pt);

	void Save(const char *filename);

	float CalcUnused();
	float m_fUnused;

protected:
	std::unique_ptr<RFREEBLOCKLIST[]> m_pFreeList;
	std::unique_ptr<u32[]> m_pData;
	int m_nSize;
};

struct RLIGHTMAPTEXTURE {
	int nSize;
	std::unique_ptr<u32> data;
	bool bLoaded;
	POINT position;
	int	nLightmapIndex;
};

using RGENERATELIGHTMAPCALLBACK = bool(*)(float fProgress);

struct LightmapGenerator
{
	const char* filename{};
	int MaxLightmapSize{}, MinLightmapSize{};
	int Supersample{};
	float Tolerance{};
	v3 AmbientLight{ 0, 0, 0 };

	RGENERATELIGHTMAPCALLBACK pProgressFn{};

	RBspObject& bsp;

	LightmapGenerator(RBspObject& bsp) : bsp(bsp) {}

	bool Generate();

private:
	void Init();
	bool ProcessConvexPolygon(const RCONVEXPOLYGONINFO* poly, int PolyIndex, int& lightmapsize);
	void InsertLightmap(int lightmapsize, int PolyIndex);
	v3 CalcDiffuse(const rboundingbox& bbox,
		const RCONVEXPOLYGONINFO* poly,
		const v3& polynormal, const v3& diff,
		const RLIGHT* plight,
		int lightmapsize,
		int j, int k,
		int au, int av, int ax);
	bool CheckShadow(const RLIGHT* plight,
		const RCONVEXPOLYGONINFO* poly,
		const rboundingbox& bbox,
		const v3& pnormal, const v3& diff,
		int lightmapsize,
		int j, int k,
		int au, int av, int ax);
	bool SaveToFile();

	float MaximumArea{};
	int ConstCount{};
	int LightIndex{};

	std::unique_ptr<RLIGHT*[]> pplight;
	std::unique_ptr<rvector[]> lightmap;
	std::unique_ptr<u32[]> lightmapdata;
	std::unique_ptr<bool[]> isshadow;
	std::unique_ptr<int[]> SourceLightmap;
	std::map<u32, int> ConstmapTable;

	std::vector<RLIGHTMAPTEXTURE> sourcelightmaplist;
	std::vector<RBspLightmapManager> LightmapList;
};

v3 GetNormal(const RCONVEXPOLYGONINFO *poly, const rvector &position,
	int au, int av);

_NAMESPACE_REALSPACE2_END
