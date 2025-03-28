#ifndef BH_WINDOW_H
#define BH_WINDOW_H

#include "../Types.h"
#include "../Rect.h"
#include "../EventStack.h"

class TiXmlElement;

////////////////////////////////////////////////////////////////////////////////
namespace bh{

enum UIEventID
{
	BH_UIE_QUIT,
	BH_UIE_PAGE,

	BH_UIE_COUNT,
	BH_UIE_UNKNOWN
};

struct UIEvent
{
	UIEvent()
		:type(BH_UIE_UNKNOWN)
		,page(0)
	{}

	UIEventID type;
	short page;
};

void InitUIEnv();
UIEventID GetUIEventEnum(char const* str);
//void ShutdownUIEnv();

////////////////////////////////////////////////////////////////////////////////

class Window
{
public:

	Window();
	Window(short w,short h);
	Window(short x,short y,short w,short h);
	~Window();
	virtual bool OnKey(short key,EventStack<UIEvent>* es);
	virtual UIEvent OnMouse(short button,short mx,short my,EventStack<UIEvent>* es);
	virtual void OnMouseMove(short mx,short my);
	virtual void SetSelected(bool selected){};
	virtual void OnSelect(EventStack<unsigned short>* es){};
	virtual void Update();
	virtual void ReadDesc(TiXmlElement const* el);
	virtual void Render();

	inline void RenderChildren()
	{
		for(std::vector<Window*>::const_iterator ci = children.begin();ci != children.end();ci++)
		{
			(*ci)->Render();
		}
	}

	inline Window* GetChild(unsigned short index)
	{
		if(index < children.size())
		{
			return children[index];
		}
		return 0;
	}

	inline bool Contains(short x,short y)
	{
		return rect.Contains(x,y);
	}

	inline Rects* GetRect()
	{
		return &rect;
	}

	inline void SetPosition(short x,short y)
	{
		rect.x = x;
		rect.y = y;
	}

	inline void SetSize(short w,short h)
	{
		rect.w = w;
		rect.h = h;
	}

	inline void Move(short x,short y)
	{
		rect.x += x;
		rect.y += y;
	}

	inline void AddChild(Window *iChild)
	{
		iChild->parent = this;
		children.push_back(iChild);
	}

protected:

	enum
	{
		BH_UI_MOUSEHOVER = 1
	};

	Window* FindLeafContainer(short mx,short my);

	Window* parent;
	std::vector<Window*> children;
	Rects rect;
	short flags;

private:
};

};
////////////////////////////////////////////////////////////////////////////////
#endif
