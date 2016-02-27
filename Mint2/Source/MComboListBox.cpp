#include "MComboListBox.h"

bool MComboListBox::OnEvent(MEvent* pEvent, MListener* pListener)
{
	if(MListBox::OnEvent(pEvent, pListener)==true) return true;
	
	switch(pEvent->nMessage){
	case MWM_LBUTTONDOWN:
	case MWM_RBUTTONDOWN:
	case MWM_LBUTTONDBLCLK:
	case MWM_RBUTTONDBLCLK:
		Show(false);
		return false;	// 다른 Widget에게 넘겨준다.
	}

	return false;
}
