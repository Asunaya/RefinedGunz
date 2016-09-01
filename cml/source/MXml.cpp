#include "stdafx.h"
#include "MXml.h"
#include "MLocale.h"
#include "MZFileSystem.h"
#include "MDebug.h"
#include <cstring>
#include <string>
#include <algorithm>
#include <cassert>

#ifndef MSVC_VER
#include "msxml.tli"
#endif

BSTR _AsciiToBSTR(const char* ascii)
{
	WCHAR wide[1024];
	int ret=::MultiByteToWideChar(CP_ACP, 0, ascii, -1, wide, 1024);
	MASSERT(ret!=0);
	return SysAllocString(wide);
}


//-----------------------------------------------------------------------------
void MXmlNode::GetNodeName(char* sOutStr, int maxlen)
{
	if (m_pDomNode)
	{
		strcpy_safe(sOutStr, maxlen, _BSTRToAscii(m_pDomNode->nodeName));
	}
}

void MXmlNode::GetText(char* sOutStr, int nMaxCharNum)
{
	if ( m_pDomNode)
	{
		if ( nMaxCharNum == -1)
		{
			strcpy_safe(sOutStr, nMaxCharNum, _BSTRToAscii(m_pDomNode->Gettext()));
			return;
		}
		else
		{
			int nLen = min( nMaxCharNum, (int)strlen( m_pDomNode->Gettext()));

			if (m_pDomNode)
			{
				strncpy_s(sOutStr, nMaxCharNum, _BSTRToAscii(m_pDomNode->Gettext()), nLen);
				*(sOutStr + nLen) = 0;
			}
		}
	}
}

void MXmlNode::SetText(const char* sText)
{
	if (m_pDomNode)
	{
		BSTR bszText;

		bszText = _AsciiToBSTR(sText);
		m_pDomNode->Puttext(bszText);
		SysFreeString(bszText);
	}
}


int	MXmlNode::GetChildNodeCount()
{
	if (m_pDomNode) return m_pDomNode->childNodes->length;
	else return -1;
}


DOMNodeType MXmlNode::GetNodeType()
{
	if (m_pDomNode) return (DOMNodeType)m_pDomNode->nodeType;
	return NODE_INVALID;
}


bool MXmlNode::HasChildNodes()
{
	if ((m_pDomNode != NULL) && 
		(m_pDomNode->hasChildNodes() == VARIANT_TRUE)) return true;

	return false;
}

MXmlNode MXmlNode::GetChildNode(int iIndex)
{
	if (m_pDomNode)
	{
		return MXmlNode(m_pDomNode->childNodes->Getitem(iIndex));
	}
	else
	{
		return MXmlNode();
	}
}

void MXmlNode::NextSibling()
{
	if (m_pDomNode)
	{
		m_pDomNode = m_pDomNode->nextSibling;
	}
}

void MXmlNode::PreviousSibling()
{
	if (m_pDomNode)
	{
		m_pDomNode = m_pDomNode->previousSibling;
	}
}

bool MXmlNode::FindChildNode(const char* sNodeName, MXmlNode* pOutNode)
{
	int iCount, i;
	char szBuf[8192];
	MXmlDomNodePtr		pNode;

	iCount = m_pDomNode->childNodes->length;

	for (i = 0; i < iCount; i++)
	{
		pNode = m_pDomNode->childNodes->Getitem(i);
		strcpy_safe(szBuf, _BSTRToAscii(pNode->nodeName));

		if (!_stricmp(szBuf, sNodeName))
		{
			pOutNode->SetXmlDomNodePtr(pNode);
			return true;
		}
	}

	return false;
}


MXmlNode MXmlNode::SelectSingleNode(TCHAR* sQueryStr)
{
	BSTR bszQueryStr;

	bszQueryStr = _AsciiToBSTR(sQueryStr);
	MXmlDomNodePtr pNode = m_pDomNode->selectSingleNode(bszQueryStr);
	SysFreeString(bszQueryStr);

	return MXmlNode(pNode);
}

MXmlDomNodeListPtr MXmlNode::SelectNodes(TCHAR* sQueryStr)
{
	BSTR bszQueryStr;

	bszQueryStr = _AsciiToBSTR(sQueryStr);
	MXmlDomNodeListPtr pNodeList = m_pDomNode->selectNodes(bszQueryStr);
	SysFreeString(bszQueryStr);

	return pNodeList;
}

bool MXmlNode::AppendChild(MXmlNode node)
{
	m_pDomNode->appendChild(node.GetXmlDomNodePtr());

	return true;
}


//-----------------------------------------------------------------------------
void MXmlElement::SetContents(int iValue)
{
	char szTemp[20];
	_itoa_s(iValue, szTemp, 10);

	SetContents(szTemp);
	
}

void MXmlElement::SetContents(float fValue)
{
	char szTemp[32];
	sprintf_safe(szTemp, "%12.4f", fValue);
	SetContents(szTemp);
}

void MXmlElement::SetContents(bool bValue)
{
	if (bValue)
	{
		SetContents("true");
	}
	else
	{
		SetContents("false");
	}
}

bool MXmlElement::AppendText(const char* sText)
{
	MXmlDomTextPtr		pText;
	MXmlDomDocPtr		pDom(MSXML::CLSID_DOMDocument);

	BSTR pBSTRText = _AsciiToBSTR(sText);
	pText = pDom->createTextNode(pBSTRText);
	SysFreeString(pBSTRText);
	m_pDomNode->appendChild(pText);

	return true;
}

bool MXmlElement::AppendChild(MXmlElement aChildElement)
{
	MXmlDomElementPtr	pElement;

	pElement = aChildElement.GetXmlDomNodePtr();
	m_pDomNode->appendChild(pElement);

	return true;
}

bool MXmlElement::AppendChild(const char* sTagName, const char* sTagText)
{
	MXmlDomElementPtr	pElement;
	MXmlDomDocPtr		pDom(MSXML::CLSID_DOMDocument);

	pElement = pDom->createElement(sTagName);

	if (sTagText != NULL)
	{
		pElement->appendChild(pDom->createTextNode(sTagText));
	}

	m_pDomNode->appendChild(pElement);

	return true;
}

MXmlElement	MXmlElement::CreateChildElement(const char* sTagName)
{
	MXmlDomElementPtr	pDomElement;
	MXmlDomDocPtr		pDom(MSXML::CLSID_DOMDocument);

	pDomElement = pDom->createElement(sTagName);

	m_pDomNode->appendChild(pDomElement);

	return MXmlElement(pDomElement);
}

bool MXmlElement::GetAttribute(char* sOutText, int maxlen, const char* sAttrName, char* sDefaultText)
{
	MXmlDomNamedNodeMapPtr pAttributes = m_pDomNode->attributes;
	MXmlDomNodePtr pNode;

	for(int i=0; i < pAttributes->length; i++)
	{
		 pNode = pAttributes->Getitem(i);
		 
		 if(!_stricmp(_BSTRToAscii(pNode->nodeName), sAttrName))
		 {
			 strcpy_safe(sOutText, maxlen, _BSTRToAscii(pNode->text));
			 return true;
		 }
	}

	strcpy_safe(sOutText, maxlen, sDefaultText);
	return false;
}

bool MXmlElement::GetAttribute(int* ipOutValue, const char* sAttrName, int nDefaultValue)
{
	char szTemp[256];
	memset(szTemp, 0, 256);

	if (!GetAttribute(szTemp, sAttrName)) 
	{
		*ipOutValue = nDefaultValue;
		return false;
	}

	try
	{
		*ipOutValue = atoi(szTemp);
	}
	catch(...)
	{
		*ipOutValue = nDefaultValue;
		return false;
	}
	
	return true;
}

bool MXmlElement::GetAttribute(float* fpOutValue, const char* sAttrName, float fDefaultValue)
{
	char szTemp[256];
	memset(szTemp, 0, 256);

	if (!GetAttribute(szTemp, sAttrName)) 
	{
		*fpOutValue = fDefaultValue;
		return false;
	}

	try
	{
		*fpOutValue = (float)atof(szTemp);
	}
	catch(...)
	{
		*fpOutValue = fDefaultValue;
		return false;
	}
	
	return true;
}
bool MXmlElement::GetAttribute(bool* bOutValue, const char* sAttrName, bool bDefaultValue)
{
	char szTemp[1024];
	memset(szTemp, 0, 1024);

	if (!GetAttribute(szTemp, sAttrName))
	{
		*bOutValue = bDefaultValue;
		return false;
	}

	if (!_stricmp(szTemp, "true"))
	{
		*bOutValue = true;
	}
	else if (!_stricmp(szTemp, "false"))
	{
		*bOutValue = false;
	}
	else
	{
		*bOutValue = bDefaultValue;
	}
	return true;
}

bool MXmlElement::GetAttribute(std::string* pstrOutValue, const char* sAttrName, char* sDefaultValue)
{
	char szTemp[256];
	memset(szTemp, 0, 256);

	if (!GetAttribute(szTemp, sAttrName)) 
	{
		*pstrOutValue = sDefaultValue;
		return false;
	}

	*pstrOutValue = szTemp;
	
	return true;

}

int MXmlElement::GetAttributeCount()
{
	MXmlDomNamedNodeMapPtr pAttributes = m_pDomNode->attributes;
	return pAttributes->length;
}

void MXmlElement::GetAttribute(int index, char* szoutAttrName, int maxlen1, char* szoutAttrValue, int maxlen2)
{
	MXmlDomNamedNodeMapPtr pAttributes = m_pDomNode->attributes;
	MXmlDomNodePtr pNode = pAttributes->Getitem(index);

	strcpy_safe(szoutAttrName, maxlen1, (_BSTRToAscii(pNode->nodeName)));
	strcpy_safe(szoutAttrValue, maxlen2, _BSTRToAscii(pNode->text));
}

bool MXmlElement::AddAttribute(const char* sAttrName, const char* sAttrText)
{
	MXmlDomDocPtr	pDom(MSXML::CLSID_DOMDocument);
	MXmlDomNodePtr pNode;
	MXmlDomNamedNodeMapPtr pAttrs;

	BSTR pBSTRAttrName = _AsciiToBSTR(sAttrName);
	BSTR pBSTRAttrText = _AsciiToBSTR(sAttrText);

	pAttrs = m_pDomNode->Getattributes();
	pNode = pDom->createAttribute(pBSTRAttrName);
	pNode->Puttext(pBSTRAttrText);
	pAttrs->setNamedItem(pNode);

	SysFreeString(pBSTRAttrName);
	SysFreeString(pBSTRAttrText);

	return true;
}

bool MXmlElement::AddAttribute(const char* sAttrName, int iAttrValue)
{
	char szTemp[20];
	_itoa_s(iAttrValue, szTemp, 10);

	return AddAttribute(sAttrName, szTemp);
}

bool MXmlElement::AddAttribute(const char* sAttrName, bool bAttrValue)
{
	if (bAttrValue)
	{
		return AddAttribute(sAttrName, "true");
	}
	else
	{
		return AddAttribute(sAttrName, "false");
	}
	return false;
}
bool MXmlElement::SetAttribute(const char* sAttrName, char* sAttrText)
{
	MXmlDomNodePtr pNode;
	MXmlDomNamedNodeMapPtr pAttrs;
	char sTemp[8192];

	if (!GetAttribute(sTemp, sAttrName))
	{
		AddAttribute(sAttrName, sAttrText);
	}
	else
	{
		pAttrs = m_pDomNode->attributes;
		for (int i = 0; i < pAttrs->length; i++)
		{
			pNode = pAttrs->Getitem(i);
			if (!_stricmp(_BSTRToAscii(pNode->nodeName), sAttrName))
			{
				BSTR pBSTRAttrText = _AsciiToBSTR(sAttrText);
				pNode->Puttext(pBSTRAttrText);
				SysFreeString(pBSTRAttrText);
			}
		}
	}

	return true;
}

bool MXmlElement::SetAttribute(const char* sAttrName, int iAttrValue)
{
	char szTemp[20];
	_itoa_s(iAttrValue, szTemp, 10);

	return SetAttribute(sAttrName, szTemp);
}

bool MXmlElement::GetChildContents(char* sOutStr, const char* sChildTagName, int nMaxCharNum)
{
	MXmlNode		node;

	if (FindChildNode(sChildTagName, &node))
	{
		node.GetText(sOutStr, nMaxCharNum);
		return true;
	}
	else
	{
		return false;
	}
	
}

bool MXmlElement::GetChildContents(int* iOutValue, const char* sChildTagName)
{
	char szBuf[256];
	if (GetChildContents(szBuf, sChildTagName))
	{
		try
		{
			*iOutValue = atoi(szBuf);
		}
		catch(...)
		{
			*iOutValue = 0;
			return false;
		}

		return true;
	}
	
	return false;
}

bool MXmlElement::GetChildContents(float* fOutValue, const char* sChildTagName)
{
	char szBuf[256];
	if (GetChildContents(szBuf, sChildTagName))
	{
		try
		{
			*fOutValue = (float)atof(szBuf);
		}
		catch(...)
		{
			*fOutValue = 0;
			return false;
		}

		return true;
	}
	
	return false;
}

bool MXmlElement::GetChildContents(bool* bOutValue, const char* sChildTagName)
{
	char szBuf[256];
	if (GetChildContents(szBuf, sChildTagName))
	{
		if (!_stricmp(szBuf, "true"))
		{
			*bOutValue = true;
		}
		else
		{
			*bOutValue = false;
		}
		return true;
	}

	return false;
}

void MXmlElement::GetContents(int* ipOutValue)
{
	char sTemp[256];
	memset(sTemp, 0, 256);

	MXmlNode::GetText(sTemp);

	try
	{
		*ipOutValue = atoi(sTemp);
	}
	catch(...)
	{
		*ipOutValue = 0;
	}
}

void MXmlElement::GetContents(bool* bpOutValue)
{
	char sTemp[64];
	memset(sTemp, 0, 64);
	MXmlNode::GetText(sTemp);

	if (!_stricmp(sTemp, "true"))
	{
		*bpOutValue = true;
	}
	else
	{
		*bpOutValue = false;
	}
}

void MXmlElement::GetContents(float* fpOutValue)
{
	char sTemp[256];
	memset(sTemp, 0, 256);

	MXmlNode::GetText(sTemp);

	try
	{
		*fpOutValue = (float)atof(sTemp);
	}
	catch(...)
	{
		*fpOutValue = 0.0f;
	}

}

void MXmlElement::GetContents(std::string* pstrValue)
{
	char sTemp[256];
	memset(sTemp, 0, 256);
	MXmlNode::GetText(sTemp);
	*pstrValue = sTemp;

}

bool MXmlElement::RemoveAttribute(const char* sAttrName)
{
	BSTR pBSTRAttrName = _AsciiToBSTR(sAttrName);
	((MXmlDomElementPtr)m_pDomNode)->removeAttribute(pBSTRAttrName);
	SysFreeString(pBSTRAttrName);

	return true;
}
//-----------------------------------------------------------------------------


MXmlDocument::MXmlDocument(void)
{
	m_bInitialized = false;

	m_ppDom = NULL;

	CoInitialize(NULL);
}

MXmlDocument::~MXmlDocument(void)
{
	if (m_bInitialized) Destroy();

	CoUninitialize();
}

bool MXmlDocument::Create(void)
{
	// CoInitialize()를 먼저 수행하기 위해 동적으로 할당함.
	m_ppDom = new MXmlDomDocPtr;


	MXmlDomDocPtr pDom(MSXML::CLSID_DOMDocument);


	(*m_ppDom) = pDom;

	(*m_ppDom)->async = false;
	(*m_ppDom)->preserveWhiteSpace = VARIANT_TRUE;

	m_bInitialized = true;

	return true;
}

bool MXmlDocument::Destroy(void)
{
	if (!m_bInitialized) return false;

	delete m_ppDom;	m_ppDom = NULL;

	m_bInitialized = false;

	return true;
}

bool MXmlDocument::LoadFromFile(const char* m_sFileName)
{
	if (!m_bInitialized) return false;

	_variant_t varOut((bool)TRUE);
	varOut = (*m_ppDom)->load((_variant_t)m_sFileName);
	if ((bool)varOut == FALSE)
	{
		MXmlDomParseErrorPtr errPtr = (*m_ppDom)->GetparseError();
		_bstr_t bstrErr(errPtr->reason);

		char szBuf[8192];

		sprintf_safe(szBuf, "-------------------------------\n");
		OutputDebugString(szBuf);
		sprintf_safe(szBuf, "Error In Xml File(%s)\n", m_sFileName);		
		OutputDebugString(szBuf);
		sprintf_safe(szBuf, "Code = 0x%x\n", errPtr->errorCode);
		OutputDebugString(szBuf);
		sprintf_safe(szBuf, "Source = Line : %ld; Char : %ld\n", errPtr->line, errPtr->linepos);
		OutputDebugString(szBuf);
		sprintf_safe(szBuf, "Error Description = %s\n", (char*)bstrErr);
		OutputDebugString(szBuf);

		_ASSERT(0);

		return false;
	}

	return true;
}

bool MXmlDocument::LoadFromMemory(char* szBuffer, LANGID lanid)
{
	if (!m_bInitialized) return false;

	// UTF8인지 검사한다
	std::string s;
	if ( (szBuffer[0] == (char)0xEF) && (szBuffer[1] == (char)0xBB) && (szBuffer[2] == (char)0xBF))
		s = MLocale::ConvUTF8ToAnsi( szBuffer, lanid);		// UTF8 -> ANSI
	else
		s = szBuffer;										// ANSI


	// XML 내용의 시작을 찾는다
	int pos = s.find( "<XML>");								// <XML>
	if ( pos == -1)
	{
		pos = s.find( "<xml>");								// <xml>
		
		if ( pos == -1)
		{
			pos = s.find( "?>");							// ?>
	
			if ( pos == -1)
			{
				s.clear();
				MLog("MXmlDocument::LoadFromMemory - Failed to find beginning of xml declaration\n");
				assert(false);
				return false;
			}

			pos += 2;
		}
	}
	char* cp = &s[pos];


	// XML 내용의 끝을 찾는다
	pos = s.find( "</XML>");
	if ( pos == -1)
	{
		pos = s.find( "</xml>");
		
		if ( pos == -1)
		{
			s.clear();
			MLog("MXmlDocument::LoadFromMemory - Failed to find end of xml declaration\n");
			assert(false);
			return false;
		}
	}
	s[pos+6] = '\0';


	_bstr_t bsXML(cp);
	if ((*m_ppDom)->loadXML(BSTR(bsXML))!= -1)
	{
		MXmlDomParseErrorPtr errPtr = (*m_ppDom)->GetparseError();
		_bstr_t bstrErr(errPtr->reason);

		MLog("-------------------------------\n");
		MLog("Error In Load Xml Memory\n");
		MLog("Code = 0x%x\n", errPtr->errorCode);
		MLog("Source = Line : %ld; Char : %ld\n", errPtr->line, errPtr->linepos);
		MLog("Error Description = %s\n", static_cast<char*>(bstrErr));

		assert(false);

		s.clear();
		return false;
	}

	s.clear();
	return true;
}


bool MXmlDocument::SaveToFile(const char* m_sFileName)
{
	try
	{
		(*m_ppDom)->save((_variant_t)m_sFileName);
	}
	catch(_com_error& e)
    {
		OutputDebugString(e.ErrorMessage());
    }
 
	return true;
}


bool MXmlDocument::CreateProcessingInstruction( const char* szHeader)
{
	MXmlDomPIPtr	pi;

	_bstr_t bsPI( "xml" );
    _bstr_t bsAttr( szHeader);

	try
	{
		pi = (*m_ppDom)->createProcessingInstruction((BSTR)bsPI, (BSTR)bsAttr);
	}
	catch(_com_error& e)
	{
		OutputDebugString(e.ErrorMessage());
	}

	if (pi != NULL) (*m_ppDom)->appendChild(pi);

	return true;
}

bool MXmlDocument::Delete(MXmlNode* pNode)
{
	pNode->GetParent().GetXmlDomNodePtr()->removeChild(pNode->GetXmlDomNodePtr());

	if (pNode->GetXmlDomNodePtr() != NULL) 
	{
		return true;
	}
	else return false;
}


MXmlNode MXmlDocument::FindElement(TCHAR* sTagName)
{
	char sBuf[1023];
	sprintf_safe(sBuf, "//%s", sTagName);

	BSTR bszQueryStr;
	bszQueryStr = _AsciiToBSTR(sBuf);
	MXmlDomNodePtr pNode = (*m_ppDom)->selectSingleNode(bszQueryStr);
	SysFreeString(bszQueryStr);

	return MXmlNode(pNode);
}

bool MXmlDocument::AppendChild(MXmlNode node)
{
	(*m_ppDom)->appendChild(node.GetXmlDomNodePtr());

	return true;
}

MXmlElement	MXmlDocument::CreateElement(const char* sName)
{
	MXmlDomElementPtr pDomElement;
	pDomElement = (*m_ppDom)->createElement(sName);

	return MXmlElement(pDomElement);
}