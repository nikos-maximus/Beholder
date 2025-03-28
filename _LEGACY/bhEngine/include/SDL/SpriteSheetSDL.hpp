#ifndef SPRITE_SDL_HPP
#define SPRITE_SDL_HPP

#include "SDL/SurfaceSDL.hpp"

class SpriteSheetSDL : public SurfaceSDL
{
public:
	struct FrameCreateInfo
	{
		int numXTiles{ 1 }, leftMargin{ 0 }, rightMargin{ 0 }, xSpacing{ 0 };
		int numYTiles{ 1 }, topMargin{ 0 }, bottomMargin{ 0 }, ySpacing{ 0 };
	};

	bool CreateFrames(const FrameCreateInfo& frameCI);
	~SpriteSheetSDL();

protected:
	int GetTileIndex(int row, int col) const;

private:
	SDL_Rect* tiles{ nullptr };
	int numTiles{ 0 }, tilesPerRow{ 0 };
};

#endif //SPRITE_SDL_HPP
