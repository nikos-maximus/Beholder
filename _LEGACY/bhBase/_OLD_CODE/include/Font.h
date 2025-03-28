#ifndef BH_FONT_H
#define BH_FONT_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Asset.h"
#include "Renderer.h"
#include "Texture.h"

namespace bh{

class Font : public Asset
{
	friend Font* LoadFont(char const* name);
	friend void DeleteFonts();

public:

	void Bind();
	void WriteChar(char c);
	
	inline short GetTextWidth(char const* text)
	{
		char c(0);
		short len(0);
		while((c = *text++) != '\0')
		{
			len += GetCharWidth(c);
		}
		return len;
	}
	
	inline short GetCharWidth(char c)
	{
		return widths[c - firstChar];
	}
	
	inline short GetCharHeight()
	{
		return charHeight;
	}

protected:
private:

	Font();
	~Font();
	void ReadMetrics(char const* name);

	unsigned short* inds;
	unsigned char* widths;
	BufferId coordsVB;
	BufferId texCoordsVB;
	Texture* texture;
	unsigned char firstChar,charHeight;
};

Font* LoadFont(char const* name);
void DeleteFonts();
void InitFontEnv();
//void BindFont(FontID fid);
//short WriteChar(FontID fid,char c);
//short GetTextWidth(FontID fid,char const* text);
//short GetLineHeight(FontID fid);

};

#endif
