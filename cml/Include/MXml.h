#pragma once

#include <stdio.h>
#include <string>

#ifdef _MSXML2
	#import "msxml4.dll" named_guids no_implementation

	typedef MSXML2::IXMLDOMDocumentPtr				MXmlDomDocPtr;
	typedef MSXML2::IXMLDOMNodePtr					MXmlDomNodePtr;
	typedef MSXML2::IXMLDOMNodeListPtr				MXmlDomNodeListPtr;
	typedef MSXML2::IXMLDOMElementPtr				MXmlDomElementPtr;
	typedef MSXML2::IXMLDOMProcessingInstructionPtr MXmlDomPIPtr;
	typedef MSXML2::IXMLDOMNamedNodeMapPtr			MXmlDomNamedNodeMapPtr;
	typedef MSXML2::IXMLDOMTextPtr					MXmlDomTextPtr;
	typedef MSXML2::IXMLDOMParseErrorPtr			MXmlDomParseErrorPtr;

#else
#ifdef _MSC_VER
	#import "msxml.dll" named_guids no_implementation
#else
#include "msxml.tlh"
#endif

	typedef MSXML::IXMLDOMDocumentPtr				MXmlDomDocPtr;
	typedef MSXML::IXMLDOMNodePtr					MXmlDomNodePtr;
	typedef MSXML::IXMLDOMNodeListPtr				MXmlDomNodeListPtr;
	typedef MSXML::IXMLDOMElementPtr				MXmlDomElementPtr;
	typedef MSXML::IXMLDOMProcessingInstructionPtr	MXmlDomPIPtr;
	typedef MSXML::IXMLDOMNamedNodeMapPtr			MXmlDomNamedNodeMapPtr;
	typedef MSXML::IXMLDOMTextPtr					MXmlDomTextPtr;
	typedef MSXML::IXMLDOMParseErrorPtr				MXmlDomParseErrorPtr;
#endif

class MXmlDocument;

class MXmlNode
{
private:

protected:
	MXmlDomNodePtr		m_pDomNode;	
public:
	MXmlNode() { m_pDomNode = NULL; }
	MXmlNode(MXmlDomNodePtr a_pDomNode) { m_pDomNode = a_pDomNode; }
	virtual ~MXmlNode() { m_pDomNode = NULL; }

	MXmlDomNodePtr	GetXmlDomNodePtr() { return m_pDomNode; }
	void			SetXmlDomNodePtr(MXmlDomNodePtr pNode) { m_pDomNode = pNode; }

	bool IsEmpty() { if (m_pDomNode == NULL) return true; else return false; }

	template<size_t size> void GetNodeName(char(&sOutStr)[size]) {
		GetNodeName(sOutStr, size);
	}
	void GetNodeName(char* sOutStr, int maxlen);
	void GetText(char* sOutStr, int nMaxCharNum = -1);
	void SetText(const char* sText);
	
	int	GetChildNodeCount();
	DOMNodeType GetNodeType();
	bool HasChildNodes();

	void NextSibling();
	void PreviousSibling();

	bool AppendChild(MXmlNode node);

	bool FindChildNode(const char* sNodeName, MXmlNode* pOutNode);

	MXmlNode GetParent() { if (m_pDomNode) return MXmlNode(m_pDomNode->parentNode); else return MXmlNode(); }
	MXmlNode GetChildNode(int iIndex);

	MXmlNode SelectSingleNode(TCHAR* sQueryStr);
	MXmlDomNodeListPtr	SelectNodes(TCHAR* sQueryStr);

	MXmlNode& operator= (MXmlNode aNode) { m_pDomNode = aNode.GetXmlDomNodePtr(); return *this; }
};

class MXmlElement: public MXmlNode
{
private:

protected:

public:
	MXmlElement() { }
	MXmlElement(MXmlDomElementPtr a_pDomElement)	{ m_pDomNode = a_pDomElement; }
	MXmlElement(MXmlDomNodePtr a_pDomNode)			{ m_pDomNode = a_pDomNode; }
	MXmlElement(MXmlNode aNode)						{ m_pDomNode = aNode.GetXmlDomNodePtr(); }
	virtual ~MXmlElement() { }

	template<size_t size> void GetTagName(char(&sOutStr)[size]) {
		GetTagName(sOutStr, size);
	}
	void GetTagName(char* sOutStr, int maxlen) { MXmlNode::GetNodeName(sOutStr, maxlen); }
	
	void GetContents(char* sOutStr) { MXmlNode::GetText(sOutStr); }
	void GetContents(int* ipOutValue);
	void GetContents(bool* bpOutValue);
	void GetContents(float* fpOutValue);
	void GetContents(std::string* pstrValue);

	void SetContents(const char* sStr) { MXmlNode::SetText(sStr); }
	void SetContents(int iValue);
	void SetContents(bool bValue);
	void SetContents(float fValue);

	bool GetChildContents(char* sOutStr, const char* sChildTagName, int nMaxCharNum = -1);
	bool GetChildContents(int* iOutValue, const char* sChildTagName);
	bool GetChildContents(float* fOutValue, const char* sChildTagName);
	bool GetChildContents(bool* bOutValue, const char* sChildTagName);

	template<size_t size> bool GetAttribute(char(&sOutText)[size], const char *szAttrName,
		char *sDefaultText = "") {
		return GetAttribute(sOutText, size, szAttrName, sDefaultText);
	}
	bool GetAttribute(char* sOutText, int maxlen, const char* sAttrName, char* sDefaultText = "");
	bool GetAttribute(int* ipOutValue, const char* sAttrName, int nDefaultValue = 0);
	bool GetAttribute(bool* bOutValue, const char* sAttrName, bool bDefaultValue = false);
	bool GetAttribute(float* fpOutValue, const char* sAttrName, float fDefaultValue = 0.0f);
	bool GetAttribute(std::string* pstrOutValue, const char* sAttrName, char* sDefaultValue = "");
	bool AddAttribute(const char* sAttrName, const char* sAttrText);
	bool AddAttribute(const char* sAttrName, int iAttrValue);
	bool AddAttribute(const char* sAttrName, bool bAttrValue);
	bool SetAttribute(const char* sAttrName, char* sAttrText);
	bool SetAttribute(const char* sAttrName, int iAttrValue);
	bool RemoveAttribute(const char* sAttrName);

	int GetAttributeCount();
	template<size_t size1, size_t size2> void GetAttribute(int index, char(&szoutAttrName)[size1], char(&szoutAttrValue)[size2]) {
		GetAttribute(index, szoutAttrName, size1, szoutAttrValue, size2);
	}
	void GetAttribute(int index, char* szoutAttrName, int maxlen1, char* szoutAttrValue, int maxlen2);

	bool AppendChild(const char* sTagName, const char* sTagText = NULL);
	bool AppendChild(MXmlElement aChildElement);

	MXmlElement	CreateChildElement(const char* sTagName);

	bool AppendText(const char* sText);

	MXmlElement& operator= (MXmlElement aElement) { m_pDomNode = aElement.GetXmlDomNodePtr(); return *this; }
	MXmlElement& operator= (MXmlNode aNode) { m_pDomNode = aNode.GetXmlDomNodePtr(); return *this; }
};

class MXmlDocument final
{
private:
	bool							m_bInitialized;	
	MXmlDomDocPtr*					m_ppDom;
protected:

public:
	MXmlDocument();
	~MXmlDocument();

	bool				Create();
	bool				Destroy();

	bool				LoadFromFile(const char* m_sFileName);
	bool				LoadFromMemory(char* szBuffer, LANGID lanid = LANG_KOREAN);

	bool				SaveToFile(const char* m_sFileName);

	bool				CreateProcessingInstruction( const char* szHeader = "version=\"1.0\"");
	bool				Delete(MXmlNode* pNode);

	MXmlElement			CreateElement(const char* sName);

	bool				AppendChild(MXmlNode node);

	MXmlDomDocPtr		GetDocument()	{ return (*m_ppDom); }
	MXmlElement			GetDocumentElement()	{ return MXmlElement((*m_ppDom)->documentElement); }

	MXmlNode			FindElement(TCHAR* sTagName);
};

// Utils
#define _BSTRToAscii(s) (const char*)(_bstr_t)(s)
BSTR _AsciiToBSTR(const char* ascii);