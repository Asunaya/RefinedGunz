#include "stdafx.h"
#include "LightmapGenerator.h"
#include "RBspObject.h"
#include "RVersions.h"
#include "RealSpace2.h"

#define MAX_LIGHTMAP_SIZE		1024
#define MAX_LEVEL_COUNT			10

_NAMESPACE_REALSPACE2_BEGIN

bool SaveMemoryBmp(int x, int y, void *data, void **retmemory, int *nsize);

using RFREEBLOCKLIST = std::list<POINT>;

struct RLIGHTMAPTEXTURE {
	int nSize;
	std::unique_ptr<u32> data;
	bool bLoaded;
	POINT position;
	int	nLightmapIndex;
};

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

RBspLightmapManager::RBspLightmapManager()
	: m_pData{ new u32[MAX_LIGHTMAP_SIZE*MAX_LIGHTMAP_SIZE] },
	m_pFreeList{ new RFREEBLOCKLIST[MAX_LEVEL_COUNT + 1] }
{
	m_nSize = MAX_LIGHTMAP_SIZE;

	POINT p = { 0,0 };
	m_pFreeList[MAX_LEVEL_COUNT].push_back(p);
}

float RBspLightmapManager::CalcUnused()
{
	float fUnused = 0.f;

	for (int i = 0; i <= MAX_LEVEL_COUNT; i++) {
		float fThisLevelSize = pow(0.25, (MAX_LEVEL_COUNT - i));
		fUnused += (float)m_pFreeList[i].size()*fThisLevelSize;
	}

	return fUnused;
}

bool RBspLightmapManager::GetFreeRect(int nLevel, POINT *pt)
{
	if (nLevel > MAX_LEVEL_COUNT) return false;

	if (!m_pFreeList[nLevel].size())
	{
		POINT point;
		if (!GetFreeRect(nLevel + 1, &point))
			return false;

		int nSize = 1 << nLevel;

		POINT newpoint;

		newpoint.x = point.x + nSize; newpoint.y = point.y;
		m_pFreeList[nLevel].push_back(newpoint);

		newpoint.x = point.x; newpoint.y = point.y + nSize;
		m_pFreeList[nLevel].push_back(newpoint);

		newpoint.x = point.x + nSize; newpoint.y = point.y + nSize;
		m_pFreeList[nLevel].push_back(newpoint);

		*pt = point;

	}
	else
	{
		*pt = *m_pFreeList[nLevel].begin();
		m_pFreeList[nLevel].erase(m_pFreeList[nLevel].begin());
	}

	return true;
}

bool RBspLightmapManager::Add(const u32 *data, int nSize, POINT *retpoint)
{
	int nLevel = 0, nTemp = 1;
	while (nSize > nTemp)
	{
		nTemp = nTemp << 1;
		nLevel++;
	}
	_ASSERT(nSize == nTemp);

	POINT pt;
	if (!GetFreeRect(nLevel, &pt))
		return false;

	for (int y = 0; y < nSize; y++)
	{
		for (int x = 0; x < nSize; x++)
		{
			m_pData[(y + pt.y)*GetSize() + (x + pt.x)] = data[y*nSize + x];
		}
	}
	*retpoint = pt;
	return true;
}

void RBspLightmapManager::Save(const char *filename)
{
	RSaveAsBmp(GetSize(), GetSize(), m_pData.get(), filename);
}

void GetNormal(RCONVEXPOLYGONINFO *poly, const rvector &position,
	rvector *normal, int au, int av)
{
	int nSelPolygon = -1, nSelEdge = -1;
	float fMinDist = FLT_MAX;

	if (poly->nVertices == 3)
		nSelPolygon = 0;
	else
	{
		rvector pnormal(poly->plane.a, poly->plane.b, poly->plane.c);

		for (int i = 0; i < poly->nVertices - 2; i++)
		{
			float t;
			rvector *a = &poly->pVertices[0];
			rvector *b = &poly->pVertices[i + 1];
			rvector *c = &poly->pVertices[i + 2];

			if (IntersectTriangle(*a, *b, *c, position + pnormal, -pnormal, &t))
			{
				nSelPolygon = i;
				nSelEdge = -1;
				break;
			}
			else
			{
				float dist = GetDistance(position, *a, *b);
				if (dist < fMinDist) { fMinDist = dist; nSelPolygon = i; nSelEdge = 0; }
				dist = GetDistance(position, *b, *c);
				if (dist < fMinDist) { fMinDist = dist; nSelPolygon = i; nSelEdge = 1; }
				dist = GetDistance(position, *c, *a);
				if (dist < fMinDist) { fMinDist = dist; nSelPolygon = i; nSelEdge = 2; }
			}
		}
	}

	rvector *v0 = &poly->pVertices[0];
	rvector *v1 = &poly->pVertices[nSelPolygon + 1];
	rvector *v2 = &poly->pVertices[nSelPolygon + 2];

	rvector *n0 = &poly->pNormals[0];
	rvector *n1 = &poly->pNormals[nSelPolygon + 1];
	rvector *n2 = &poly->pNormals[nSelPolygon + 2];

	rvector pos;
	if (nSelEdge != -1)
	{
		rvector *e0 = nSelEdge == 0 ? v0 : nSelEdge == 1 ? v1 : v2;
		rvector *e1 = nSelEdge == 0 ? v1 : nSelEdge == 1 ? v2 : v0;

		rvector dir = *e1 - *e0;
		Normalize(dir);

		pos = *e0 + DotProduct(dir, position - *e0)*dir;
	}
	else
		pos = position;

	rvector a, b, x, tem;

	a = *v1 - *v0;
	b = *v2 - *v1;
	x = pos - *v0;

	float f = b[au] * x[av] - b[av] * x[au];
	if (IS_ZERO(f))
	{
		*normal = *n0;
		return;
	}
	float t = (a[av] * x[au] - a[au] * x[av]) / f;

	tem = Slerp(*n1, *n2, t);

	rvector inter = a + t*b;

	int axisfors;
	if (fabs(inter.x) > fabs(inter.y) && fabs(inter.x) > fabs(inter.z))
		axisfors = 0;
	else if (fabs(inter.y) > fabs(inter.z))
		axisfors = 1;
	else
		axisfors = 2;

	float s = x[axisfors] / inter[axisfors];
	*normal = Slerp(*n0, tem, s);
}

static void CalcLightmapUV(
	RSBspNode * pNode,
	const int * pSourceLightmap,
	const std::vector<RLIGHTMAPTEXTURE*>& SourceLightmaps,
	std::vector<RBspLightmapManager>& LightmapList,
	const std::vector<RCONVEXPOLYGONINFO>& ConvexPolygons)
{
	if (pNode->nPolygon)
	{
		int i, j, k;
		for (i = 0; i < pNode->nPolygon; i++)
		{
			int is = pNode->pInfo[i].nConvexPolygon;
			int nSI = pSourceLightmap[is];

			auto* poly = &ConvexPolygons[is];

			rboundingbox bbox;

			bbox.vmin = bbox.vmax = poly->pVertices[0];
			for (j = 1; j < poly->nVertices; j++)
			{
				for (k = 0; k < 3; k++)
				{
					bbox.vmin[k] = min(bbox.vmin[k], poly->pVertices[j][k]);
					bbox.vmax[k] = max(bbox.vmax[k], poly->pVertices[j][k]);
				}
			}

			RLIGHTMAPTEXTURE* pDestLightmap = SourceLightmaps[nSI];

			int lightmapsize = pDestLightmap->nSize;

			rvector diff = float(lightmapsize) / float(lightmapsize - 1)*(bbox.vmax - bbox.vmin);

			for (k = 0; k<3; k++)
			{
				bbox.vmin[k] -= .5f / float(lightmapsize)*diff[k];
				bbox.vmax[k] += .5f / float(lightmapsize)*diff[k];
			}

			int au, av, ax;

			if (fabs(poly->plane.a)>fabs(poly->plane.b) && fabs(poly->plane.a) > fabs(poly->plane.c))
				ax = 0; // yz
			else if (fabs(poly->plane.b) > fabs(poly->plane.c))
				ax = 1;	// xz
			else
				ax = 2;	// xy

			au = (ax + 1) % 3;
			av = (ax + 2) % 3;

			RPOLYGONINFO *pInfo = &pNode->pInfo[i];
			for (j = 0; j < pInfo->nVertices; j++)
			{
				pInfo->pVertices[j].tu2 = ((*pInfo->pVertices[j].Coord())[au] - bbox.vmin[au]) / diff[au];
				pInfo->pVertices[j].tv2 = ((*pInfo->pVertices[j].Coord())[av] - bbox.vmin[av]) / diff[av];
			}

			auto* CurrentLightmap = LightmapList.size() ? &LightmapList[LightmapList.size() - 1] : NULL;

			if (!pDestLightmap->bLoaded)
			{
				POINT pt;

				while (!CurrentLightmap || !CurrentLightmap->Add(pDestLightmap->data.get(), pDestLightmap->nSize, &pt))
				{
					LightmapList.emplace_back();
					CurrentLightmap = &LightmapList.back();
				}
				pDestLightmap->bLoaded = true;
				pDestLightmap->position = pt;
				pDestLightmap->nLightmapIndex = LightmapList.size() - 1;
			}

			pNode->pInfo[i].nLightmapTexture = pDestLightmap->nLightmapIndex;

			float fScaleFactor = (float)pDestLightmap->nSize / (float)CurrentLightmap->GetSize();
			for (j = 0; j < pInfo->nVertices; j++)
			{
				pInfo->pVertices[j].tu2 =
					pInfo->pVertices[j].tu2 * fScaleFactor +
					(float)pDestLightmap->position.x / (float)CurrentLightmap->GetSize();
				pInfo->pVertices[j].tv2 =
					pInfo->pVertices[j].tv2 * fScaleFactor +
					(float)pDestLightmap->position.y / (float)CurrentLightmap->GetSize();
			}
		}
	}

	if (pNode->m_pPositive) CalcLightmapUV(pNode->m_pPositive, pSourceLightmap, SourceLightmaps, LightmapList, ConvexPolygons);
	if (pNode->m_pNegative) CalcLightmapUV(pNode->m_pNegative, pSourceLightmap, SourceLightmaps, LightmapList, ConvexPolygons);
}

bool LightmapGenerator::Generate()
{
	float fMaximumArea = 0;

	for (size_t i = 0; i < bsp.ConvexPolygons.size(); i++)
	{
		fMaximumArea = max(fMaximumArea, bsp.ConvexPolygons[i].fArea);
	}

	int nConstCount = 0;
	int nLight;
	std::unique_ptr<RLIGHT*[]> pplight{ new RLIGHT*[bsp.StaticMapLightList.size()] };
	std::unique_ptr<rvector[]> lightmap{ new rvector[Square(MaxLightmapSize)] };
	std::unique_ptr<u32[]> lightmapdata{ new u32[Square(MaxLightmapSize)] };
	std::unique_ptr<bool[]> isshadow{ new bool[Square(MaxLightmapSize + 1)] };
	std::unique_ptr<int[]> SourceLightmap{ new int[bsp.ConvexPolygons.size()] };
	std::map<u32, int> ConstmapTable;

	std::vector<RLIGHTMAPTEXTURE*> sourcelightmaplist;
	std::vector<RBspLightmapManager> LightmapList;

	RHEADER header(R_LM_ID, R_LM_VERSION);

	for (size_t i = 0; i < bsp.ConvexPolygons.size(); i++)
	{
		RCONVEXPOLYGONINFO *poly = &bsp.ConvexPolygons[i];

		if (pProgressFn)
		{
			bool bContinue = pProgressFn((float)i / (float)bsp.ConvexPolygons.size());
			if (!bContinue)
			{
				return false;
			}
		}

		rboundingbox bbox;

		bbox.vmin = bbox.vmax = poly->pVertices[0];
		for (int j = 1; j < poly->nVertices; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				bbox.vmin[k] = min(bbox.vmin[k], poly->pVertices[j][k]);
				bbox.vmax[k] = max(bbox.vmax[k], poly->pVertices[j][k]);
			}
		}

		int lightmapsize;

		{
			lightmapsize = MaxLightmapSize;

			float targetarea = fMaximumArea / 4.f;
			while (poly->fArea < targetarea && lightmapsize > MinLightmapSize)
			{
				targetarea /= 4.f;
				lightmapsize /= 2;
			}

			rvector diff = float(lightmapsize) / float(lightmapsize - 1)*(bbox.vmax - bbox.vmin);

			// 1 texel
			for (int k = 0; k < 3; k++)
			{
				bbox.vmin[k] -= .5f / float(lightmapsize)*diff[k];
				bbox.vmax[k] += .5f / float(lightmapsize)*diff[k];
			}

			rvector pnormal = rvector(poly->plane.a, poly->plane.b, poly->plane.c);

			RBSPMATERIAL *pMaterial = &bsp.Materials[bsp.ConvexPolygons[i].nMaterial];

			rvector ambient = pMaterial->Ambient;

			nLight = 0;

			for (auto& Light : bsp.StaticMapLightList)
			{
				if (GetDistance(Light.Position, poly->plane) > Light.fAttnEnd) continue;

				for (int iv = 0; iv < poly->nVertices; iv++)
				{
					if (DotProduct(Light.Position - poly->pVertices[iv], poly->pNormals[iv])>0) {
						pplight[nLight++] = &Light;
						break;
					}

				}
			}

			int au, av, ax;

			if (fabs(poly->plane.a) > fabs(poly->plane.b) && fabs(poly->plane.a) > fabs(poly->plane.c))
				ax = 0; // yz
			else if (fabs(poly->plane.b) > fabs(poly->plane.c))
				ax = 1;	// xz
			else
				ax = 2;	// xy

			au = (ax + 1) % 3;
			av = (ax + 2) % 3;

			for (int j = 0; j < lightmapsize; j++)			// v 
			{
				for (int k = 0; k < lightmapsize; k++)		// u
				{
					lightmap[j*lightmapsize + k] = AmbientLight;
				}
			}

			for (int l = 0; l < nLight; l++)
			{
				RLIGHT *plight = pplight[l];

				for (int j = 0; j < lightmapsize + 1; j++)			// v 
				{
					for (int k = 0; k < lightmapsize + 1; k++)		// u
					{
						isshadow[k*(lightmapsize + 1) + j] = false;
						if ((plight->dwFlags & RM_FLAG_CASTSHADOW) == 0 ||
							(poly->dwFlags & RM_FLAG_RECEIVESHADOW) == 0) continue;
						_ASSERT(plight->dwFlags == 16);

						rvector position;
						position[au] = bbox.vmin[au] + ((float)k / (float)lightmapsize)*diff[au];
						position[av] = bbox.vmin[av] + ((float)j / (float)lightmapsize)*diff[av];
						position[ax] = (-poly->plane.d - pnormal[au] * position[au] - pnormal[av] * position[av]) / pnormal[ax];

						float fDistanceToPolygon = Magnitude(position - plight->Position);

						RBSPPICKINFO bpi;
						if (bsp.PickShadow(plight->Position, position, &bpi))
						{
							float fDistanceToPickPos = Magnitude(bpi.PickPos - plight->Position);

							if (fDistanceToPolygon > fDistanceToPickPos + Tolerance)
								isshadow[k*(lightmapsize + 1) + j] = true;
						}

						for (auto& ObjectInfo : bsp.m_ObjectList)
						{
							if (!ObjectInfo.pVisualMesh) continue;
							float t;

							rmatrix inv = Inverse(ObjectInfo.pVisualMesh->m_WorldMat);

							rvector origin = plight->Position * inv;
							rvector target = position * inv;

							rvector dir = target - origin;
							rvector dirorigin = position - plight->Position;

							rvector vOut;

							rboundingbox bbox;
							bbox.vmin = ObjectInfo.pVisualMesh->m_vBMin;
							bbox.vmax = ObjectInfo.pVisualMesh->m_vBMax;
							auto bBBTest = IntersectLineAABB(t, origin, dir, bbox);
							if (bBBTest &&
								ObjectInfo.pVisualMesh->Pick(plight->Position, dirorigin, &vOut, &t))
							{
								rvector PickPos = plight->Position + vOut*t;
								isshadow[k*(lightmapsize + 1) + j] = true;
							}
						}
					}
				}


				for (int j = 0; j < lightmapsize; j++)
				{
					for (int k = 0; k < lightmapsize; k++)
					{
						rvector color = rvector(0, 0, 0);

						int nShadowCount = 0;

						for (int m = 0; m < 4; m++)
						{
							if (isshadow[(k + m % 2)*(lightmapsize + 1) + j + m / 2])
								nShadowCount++;
						}


						if (nShadowCount < 4)
						{
							if (nShadowCount > 0)
							{
								int m, n;
								rvector tempcolor = rvector(0, 0, 0);

								for (m = 0; m < Supersample; m++)
								{
									for (n = 0; n < Supersample; n++)
									{
										rvector position;
										position[au] = bbox.vmin[au] + (((float)k + ((float)n + .5f) / (float)Supersample) / (float)lightmapsize)*diff[au];
										position[av] = bbox.vmin[av] + (((float)j + ((float)m + .5f) / (float)Supersample) / (float)lightmapsize)*diff[av];
										position[ax] = (-poly->plane.d - pnormal[au] * position[au] - pnormal[av] * position[av]) / pnormal[ax];

										bool bShadow = false;

										float fDistanceToPolygon = Magnitude(position - plight->Position);

										RBSPPICKINFO bpi;
										if (bsp.PickShadow(plight->Position, position, &bpi))
										{
											float fDistanceToPickPos = Magnitude(bpi.PickPos - plight->Position);
											if (fDistanceToPolygon > fDistanceToPickPos + Tolerance)
												bShadow = true;
										}

										if (!bShadow)
										{
											rvector dpos = plight->Position - position;
											float fdistance = Magnitude(dpos);
											float fIntensity = (fdistance - plight->fAttnStart) / (plight->fAttnEnd - plight->fAttnStart);
											fIntensity = min(max(1.0f - fIntensity, 0.f), 1.f);
											Normalize(dpos);

											rvector normal;
											GetNormal(poly, position, &normal, au, av);

											float fDot;
											fDot = DotProduct(dpos, normal);
											fDot = max(0.f, fDot);

											tempcolor += fIntensity*plight->fIntensity*fDot*plight->Color;
										}
									}
								}
								tempcolor *= 1.f / Square(Supersample);
								color += tempcolor;
							}
							else
							{
								rvector position;
								position[au] = bbox.vmin[au] + (((float)k + .5f) / (float)lightmapsize)*diff[au];
								position[av] = bbox.vmin[av] + (((float)j + .5f) / (float)lightmapsize)*diff[av];
								position[ax] = (-poly->plane.d - pnormal[au] * position[au] - pnormal[av] * position[av]) / pnormal[ax];

								rvector dpos = plight->Position - position;
								float fdistance = Magnitude(dpos);
								float fIntensity = (fdistance - plight->fAttnStart) / (plight->fAttnEnd - plight->fAttnStart);
								fIntensity = min(max(1.0f - fIntensity, 0.f), 1.f);
								Normalize(dpos);

								rvector normal;
								GetNormal(poly, position, &normal, au, av);

								float fDot;
								fDot = DotProduct(dpos, normal);
								fDot = max(0.f, fDot);

								color += fIntensity*plight->fIntensity*fDot*plight->Color;
							}
						}

						lightmap[j*lightmapsize + k] += color;
					}
				}
			}

			for (int j = 0; j < lightmapsize*lightmapsize; j++)
			{
				rvector color = lightmap[j];

				color *= 0.25f;
				color.x = min(color.x, 1.f);
				color.y = min(color.y, 1.f);
				color.z = min(color.z, 1.f);
				lightmap[j] = color;
				lightmapdata[j] = ((u32)(color.x * 255)) << 16 | ((u32)(color.y * 255)) << 8 | ((u32)(color.z * 255));
			}
		}

		bool bConstmap = true;
		for (int j = 0; j < lightmapsize*lightmapsize; j++)
		{
			if (lightmapdata[j] != lightmapdata[0])
			{
				bConstmap = false;
				nConstCount++;
				break;
			}
		}

		bool bNeedInsert = true;
		if (bConstmap)
		{
			lightmapsize = 2;

			map<u32, int>::iterator it = ConstmapTable.find(lightmapdata[0]);
			if (it != ConstmapTable.end())
			{
				SourceLightmap[i] = (*it).second;
				bNeedInsert = false;
			}
		}

		if (bNeedInsert)
		{
			int nLightmap = sourcelightmaplist.size();

			SourceLightmap[i] = nLightmap;
			if (bConstmap)
				ConstmapTable.insert(map<u32, int>::value_type(lightmapdata[0], nLightmap));

			RLIGHTMAPTEXTURE *pnew = new RLIGHTMAPTEXTURE;
			pnew->bLoaded = false;
			pnew->nSize = lightmapsize;
			pnew->data = decltype(pnew->data){new u32[Square(lightmapsize)]};
			memcpy(pnew->data.get(), lightmapdata.get(), Square(lightmapsize) * sizeof(u32));
			sourcelightmaplist.push_back(pnew);
		}
	}

	auto CalcTreeUV = [&](auto* tree) {
		CalcLightmapUV(tree, SourceLightmap.get(),
			sourcelightmaplist, LightmapList, bsp.ConvexPolygons);
	};
	CalcTreeUV(bsp.BspRoot.data());
	CalcTreeUV(bsp.OcRoot.data());

	FILE *file = nullptr;
	fopen_s(&file, filename, "wb+");
	if (!file) {
		return false;
	}

	fwrite(&header, sizeof(RHEADER), 1, file);

	auto nConvexPolygons = bsp.ConvexPolygons.size();
	fwrite(&nConvexPolygons, sizeof(int), 1, file);
	int NodeCount = bsp.GetNodeCount();
	fwrite(&NodeCount, sizeof(int), 1, file);

	auto nLightmap = LightmapList.size();
	fwrite(&nLightmap, sizeof(int), 1, file);
	for (size_t i = 0; i < LightmapList.size(); i++)
	{
		char lightfilename[256];
		sprintf_safe(lightfilename, "%s.light%d.bmp", filename, i);
		RSaveAsBmp(LightmapList[i].GetSize(), LightmapList[i].GetSize(),
			LightmapList[i].GetData(), lightfilename);

		void *memory;
		int nSize;
		bool bSaved = SaveMemoryBmp(LightmapList[i].GetSize(), LightmapList[i].GetSize(),
			LightmapList[i].GetData(), &memory, &nSize);
		_ASSERT(bSaved);
		fwrite(&nSize, sizeof(int), 1, file);
		fwrite(memory, nSize, 1, file);
		delete memory;
	}

	bsp.Sort_Nodes(bsp.OcRoot.data());

	for (int i = 0; i < bsp.GetPolygonCount(); i++)
		fwrite(&bsp.OcInfo[i].nPolygonID, sizeof(int), 1, file);

	for (int i = 0; i < bsp.GetPolygonCount(); i++)
		fwrite(&bsp.OcInfo[i].nLightmapTexture, sizeof(int), 1, file);

	for (size_t i = 0; i < bsp.OcVertices.size(); i++)
		fwrite(&bsp.OcVertices[i].tu2, sizeof(float), 2, file);

	fclose(file);

	return true;
}

_NAMESPACE_REALSPACE2_END
