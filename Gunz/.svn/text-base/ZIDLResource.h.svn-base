#ifndef ZIDLRESOURCE_H
#define ZIDLRESOURCE_H

#include "MIDLResource.h"
#include "ZFilePath.h"


class ZFrame;
class ZMapListBox;
class ZScoreListBox;
class ZScoreBoardFrame;
class ZMeshView;
class ZMeshViewList;
class ZCharacterView;
class ZCharacterViewList;
class ZEquipmentListBox;
class ZStageInfoBox;
class ZItemSlotView;
class ZRoomListBox;
class ZPlayerListBox;
class ZCanvas;
class ZPlayerSelectListBox;
class ZBmNumLabel;
class ZClanListBox;
class ZServerView;
class ZActionKey;

class ZIDLResource : public MIDLResource
{
private:
protected:
	ZMapListBox* GetMapListBox(MXmlElement& element);
	ZScoreBoardFrame* GetScoreBoardFrame(MXmlElement& element);
	ZScoreListBox* GetScoreListBox(MXmlElement& element);
	ZMeshView* GetMeshView(MXmlElement& element);
	ZMeshViewList* GetMeshViewList(MXmlElement& element);
	ZCharacterView* GetCharacterView(MXmlElement& element);
	ZCharacterViewList* GetCharacterViewList(MXmlElement& element);
	ZEquipmentListBox* GetEquipmentListBox(MXmlElement& element);
	ZStageInfoBox* GetStageInfoBox(MXmlElement& element);
	ZItemSlotView* GetItemSlot(MXmlElement& element);
	ZRoomListBox* GetRoomListBox(MXmlElement& element);
	ZPlayerListBox* GetPlayerListBox(MXmlElement& element);
	ZCanvas* GetCanvas(MXmlElement& element);
	ZPlayerSelectListBox* GetPlayerSelectListBox(MXmlElement& element);
	ZBmNumLabel *GetBmNumLabel(MXmlElement& element);
	ZClanListBox* GetClanListBox( MXmlElement& element );
	ZServerView* GetServerView(MXmlElement& element);
	ZActionKey* GetActionKey(MXmlElement& element);

	virtual void TransText(char* szSrc, char* szOut);		// 국제화를 위한 함수
	virtual void Parse(MXmlElement& element);
	virtual MFrame*	CreateFrame(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual MFont* CreateFont(char* szAliasName, char* szFontName, int nHeight
		              ,bool bBold = false, bool bItalic = false, int nOutlineStyle = 0, bool bAntialiasing = false, DWORD nColorArg1=0, DWORD nColorArg2=0);
public:
	ZIDLResource();
	virtual ~ZIDLResource();
};

void ZGetInterfaceSkinPath(char* pOutPath, const char* szSkinName);

#endif