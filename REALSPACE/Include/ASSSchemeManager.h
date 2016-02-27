#ifndef __ASSSCHEMEMANAGER_H
#define __ASSSCHEMEMANAGER_H

#include "RSMaterialManager.h"
#include "CMList.h"

/*
   scheme이 되는 rml에는 다음과 같은 이름을 가직 material들이 들어있다.

pattern			:	base pattern ( 멀티 텍스쳐 )
base_#1			:	base가 되는 material
ext_#1_#2_#3	:	#1 : nTileSet			:	확장된 텍스처 셋의 종류 : ( 풀, 땅, ...)
				 	#2 : nTileStyle			:	타일의 이음 형식 ( 다섯종류가 있다 )
				 	#3 : nTileAppearance	:	같은 스타일의 여러모양
												즉 이 번호는 달라져도 타일이 이어져야한다.
												(꼭 같은 벌 수로 가질필요는 없다)

					#2의 TileStyle에 따라 material은 다음과 같은 기본적인 모양을 가져야한다.

				0				1				2				3				4
            +-----------+   +-----------+   +-----------+   +-----------+   +-----------+
            |***********|   |*****      |   |***********|   |***********|   |*****      |
            |***********|   |*****      |   |***********|   |***********|   |*****      |
            |***********|   |*****      |   |***********|   |***********|   |*****      |
            |***********|   |           |   |           |   |*****      |   |      *****|
            |***********|   |           |   |           |   |*****      |   |      *****|
            |***********|   |           |   |           |   |*****      |   |      *****|
            +-----------+   +-----------+   +-----------+   +-----------+   +-----------+

  참고 1. #1혹은 #3이 열 개 이상이 될 경우에는 MaterialBlast 의 순서에 따름. 따라서
			순서를 보존하려면 01 ~ 09 , 10 ~ ... 와 같은 형식으로 써주어야 함.

  참고 2. MaterialBlast는 대소문자를 가리므로 이 또한 마찬가지. 다~ 소문자로.
  참고 3. Texture이름이 아닌 Material이름이므로 MaterialBlast에서 
			add할때 Create Materials... 해야함.

  실제 예는 sample.rml을 참조, 

*/

class ASSTile
{
public:
	ASSTile();
	virtual ~ASSTile();

	int nTextureHandle[4];
	int nTextureIndex;
};

typedef CMLinkedList <ASSTile> ASSTileAppearance;

class ASSTileSet
{
public:
	ASSTileAppearance *Get(int i) { return &m_TileAppearances[i]; }

private:
	ASSTileAppearance m_TileAppearances[5];
};

class ASSSchemeManager : public CMLinkedList <ASSTileSet>
{
public:
	bool Create(const char *RMLName);
	void Destroy();

	int	GetBaseCount() { return m_BaseTileList.GetCount(); }
	ASSTile *GetBaseTile(int n) { return m_BaseTileList.Get(n); }

	int GetTextureHandle(int index);
	RSMaterialManager* GetMaterialManager() { return &m_MaterialManager; }

private:
	bool IsExtensionName(const char *name,int *nTileSet,int *nTileStyle,int *nTileAppearance);

	CMLinkedList <ASSTile> m_BaseTileList;
	RSMaterialManager m_MaterialManager;
};

#endif