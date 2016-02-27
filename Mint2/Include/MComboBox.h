#ifndef MCOMBOBOX_H
#define MCOMBOBOX_H

#include "MButton.h"
#include "MListBox.h"

/// ComboBox에서 드롭되는 ListBox
class MComboListBox : public MListBox{
protected:
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);
public:
	MComboListBox(MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MComboListBox(void){}

	DECLARE_LOOK(MListBoxLook)
	DECLARE_LOOK_CLIENT()

#define MINT_COMBOLISTBOX	"ComboListBox"
	virtual const char* GetClassName(void){ return MINT_COMBOLISTBOX; }
};

class MIDLResource;

class MComboBox : public MButton{
	friend MIDLResource;

	MComboListBox*	m_pListBox;
	int			m_nDropHeight;
	MListener*	m_pComboBoxListener;

	int			m_nComboType;
	int			m_nNextComboBoxTypeSize;

	bool		m_bAutoDrop;
	bool		m_bDropUnder;

protected:
	virtual bool OnCommand(MWidget* pWindow, const char* szMessage);

public:
	MComboBox(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MComboBox(void);

	void SetDropSize(int nHeight);

	void SetComboType(int nType) {
		m_nComboType = nType;
	}

	int GetComboType() {
		return m_nComboType;
	}

	void SetNextComboBoxTypeSize(int nSize) {
		m_nNextComboBoxTypeSize = nSize;
	}

	int GetNextComboBoxTypeSize() {
		return m_nNextComboBoxTypeSize;
	}

	void SetNextSel();
	void SetPrevSel();

	void Add(const char* szItem);
	void Add(MListItem* pItem);
	const char* GetString(int i);
	MListItem* Get(int i);
	void Remove(int i);
	void RemoveAll(void);
	int GetCount(void);
	int GetSelIndex(void);
	bool SetSelIndex(int i);
	const char* GetSelItemString(void);
	MListItem* GetSelItem(void);

	// Field Support
	void AddField(const char* szFieldName, int nTabSize) { m_pListBox->AddField(szFieldName, nTabSize); }
	void RemoveField(const char* szFieldName) { m_pListBox->RemoveField(szFieldName); }
	MLISTFIELD* GetField(int i) { return m_pListBox->GetField(i); }
	int GetFieldCount(void) { return m_pListBox->GetFieldCount(); }

	bool IsVisibleHeader(void) { return m_pListBox->IsVisibleHeader(); }
	void SetVisibleHeader(bool bVisible) { m_pListBox->SetVisibleHeader(bVisible); }

	virtual void SetListener(MListener* pListener);
	virtual MListener* GetListener(void);

	void Sort(void);

	void SetListboxAlignment( MAlignmentMode am)		{ m_pListBox->m_FontAlign = am; }

#define MINT_COMBOBOX	"ComboBox"
	virtual const char* GetClassName(void){ return MINT_COMBOBOX; }

	virtual void OnReleaseFocus();

	void CloseComboBoxList();			// 콤보박스의 열린 리스트를 강제로 닫음
};

#define MCMBBOX_CHANGED		"changed"
#endif