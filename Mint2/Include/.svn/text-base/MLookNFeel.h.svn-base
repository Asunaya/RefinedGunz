//
//		Look&Feel Macro
//
//
//		Header :
//
//			DECLARE_LOOK
//				m_DefaultLook, m_pStaticLook, m_pCustomLook, OnDraw(), ChangeLook(), ChangeCustomLook(), GetLook()
//
//			DECLARE_LOOK_CLIENT
//				GetClientRect()
//
//
//		Source :
//
//			IMPLEMENT_LOOK
//				DECLARE_LOOK Implementation
//				
//			LOOK_IN_CONSTRUCTOR
//				Constructor Initiailize Code(m_pCustomLook = NULL;)
//
#ifndef MLOOKNFELL_H
#define MLOOKNFELL_H


// Look&Feel Declaration
// Variable	: m_DefaultLook, m_pStaticLook, m_pCustomLook
// Funtion	: OnDraw(), ChangeLook(), ChangeCustomLook()
#define DECLARE_LOOK(CLASS_LOOK)								\
	private:													\
	static CLASS_LOOK	m_DefaultLook;							\
	static CLASS_LOOK*	m_pStaticLook;							\
	CLASS_LOOK*			m_pCustomLook;							\
	protected:													\
	virtual void OnDraw(MDrawContext* pDC);						\
	public:														\
	static void ChangeLook(CLASS_LOOK* pLook);					\
	void ChangeCustomLook(CLASS_LOOK* pLook);					\
	CLASS_LOOK* GetLook(void){									\
		if(m_pCustomLook!=NULL) return m_pCustomLook;			\
		return m_pStaticLook;									\
	}


// Look&Feel Client Info. Declaration
// Funtion	: GetClientRect()
#define DECLARE_LOOK_CLIENT()									\
	public:														\
	virtual MRECT GetClientRect(void){							\
		if(GetLook()==NULL)										\
			return MWidget::GetClientRect();					\
		return GetLook()->GetClientRect(this,					\
			MWidget::GetClientRect());							\
	}


// Look&Feel Implementation
#define IMPLEMENT_LOOK(CLASS, CLASS_LOOK)						\
	CLASS_LOOK CLASS::m_DefaultLook;							\
	CLASS_LOOK* CLASS::m_pStaticLook = &CLASS::m_DefaultLook;	\
	void CLASS::OnDraw(MDrawContext* pDC){						\
		if(GetLook()!=NULL) GetLook()->OnDraw(this, pDC);		\
	}															\
	void CLASS::ChangeLook(CLASS_LOOK* pLook){					\
		if(pLook==NULL){										\
			m_pStaticLook = &m_DefaultLook;						\
			return;												\
		}														\
		m_pStaticLook = pLook;									\
	}															\
	void CLASS::ChangeCustomLook(CLASS_LOOK* pLook){			\
		m_pCustomLook = pLook;									\
	}


// Look&Feel in Class Constructor
#define LOOK_IN_CONSTRUCTOR()	m_pCustomLook = NULL;


#endif