#ifndef GT_UIMENU_H
#define GT_UIMENU_H

#include "Window.h"

////////////////////////////////////////////////////////////////////////////////

//DOC
//Menu calculates the hit area on the fly
//based on the geometry coverage of contained children
//i.e. creates a max (all containing) rect

////////////////////////////////////////////////////////////////////////////////
namespace bh{

class Menu : public Window
{
public:

	Menu();
	bool OnKey(int key,EventStack<unsigned short>* es);
	void PrevItem();
	void NextItem();
	void ReadDesc(TiXmlElement const* el);

protected:

	short currItem;

private:
};

};
////////////////////////////////////////////////////////////////////////////////
#endif
