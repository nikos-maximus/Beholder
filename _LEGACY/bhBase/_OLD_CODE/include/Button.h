#ifndef BH_BUTTON_H
#define BH_BUTTON_H

#include "Panel.h"

////////////////////////////////////////////////////////////////////////////////
namespace bh{

class Button : public Panel
{
public:

	Button();
	void OnSelect(EventStack<UIEvent>* es);
	void ReadDesc(TiXmlElement const* el);

protected:

	UIEvent OnMouse(short button,short mx,short my,EventStack<UIEvent>* es);
	inline void AddChild(Window *iChild){} //Buttons are not allowed to have children
	UIEvent lmbEvent;

private:
};

};
////////////////////////////////////////////////////////////////////////////////
#endif
