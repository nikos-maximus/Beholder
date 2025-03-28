#ifndef GT_UITEXTPANEL_H
#define GT_UITEXTPANEL_H

#include "Window.h"
#include "../Font.h"
#include "../Color.h"

class TiXmlElement;

////////////////////////////////////////////////////////////////////////////////
namespace bh{

class StaticText : public Window
{
public:

	StaticText();
	StaticText(char const* cText);
	//virtual ~StaticText();
	//void SetFontAttribs(char const* fontName,double sz);

	inline void StaticText::SetText(char const* iText)
	{
		text = std::string(iText);
	}

	inline void StaticText::AddChar(char c)
	{
		text.push_back(c);
	}

	inline void StaticText::AddText(char const* iText)
	{
		text.append(iText);
	}

	inline void StaticText::ClearText()
	{
		text.clear();
	}

	inline void SetFont(char const* fName)
	{
		font = LoadFont(fName);
	}

	void Render();
	void ReadDesc(TiXmlElement const* el);
	
	inline void SetSelected(bool selected)
	{
		//DEBUG
		selected ? fontColor = bh::COLOR_YELLOW : fontColor = bh::COLOR_WHITE;
	}

protected:

	//enum{
	//	GT_TEXTPANEL_DIRTY = 1
	//};

	void PrintText(char* text);

	Color fontColor;
	Font* font;
	short penx,peny,hSpacing,vSpacing;
	//short flags;
	std::string text;

private:
};

};
////////////////////////////////////////////////////////////////////////////////
#endif
