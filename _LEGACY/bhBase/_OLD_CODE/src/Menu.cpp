#include "Menu.h"
#include "../Renderer.h"

////////////////////////////////////////////////////////////////////////////////
namespace bh{

Menu::Menu()
	:Window()
	,currItem(0)
{
}

void Menu::PrevItem()
{
	children[currItem]->SetSelected(false);
	--currItem;
	if(currItem < 0)
	{
		currItem += children.size();
	}
	children[currItem]->SetSelected(true);
}

void Menu::NextItem()
{
	children[currItem]->SetSelected(false);
	currItem = (++currItem) % children.size();
	children[currItem]->SetSelected(true);
}

void Menu::ReadDesc(TiXmlElement const* el)
{
	Window::ReadDesc(el);
	for(std::vector<Window*>::iterator ci = children.begin();ci != children.end();++ci)
	{
		rect = rect.GetExpansion((*ci)->GetRect());
	}
}

bool Menu::OnKey(int key,EventStack<unsigned short>* es)
{
	/*
	switch(key)
	{
	case SDLK_UP:
		{
			PrevItem();
			break;
		}
	case SDLK_DOWN:
		{
			NextItem();
			break;
		}
	case SDLK_RETURN:
		{
			children[currItem]->OnSelect(es);
			break;
		}
	default:
		{
			break;
		}
	}
	*/
	return true;
}

};
////////////////////////////////////////////////////////////////////////////////
