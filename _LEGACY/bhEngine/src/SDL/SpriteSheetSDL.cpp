#include "SDL/SpriteSheetSDL.hpp"

SpriteSheetSDL::~SpriteSheetSDL()
{
	free(tiles);
}

bool SpriteSheetSDL::CreateFrames(const FrameCreateInfo& frameCI)
{
	SDL_Surface* surf = GetSurface();
	if (!surf) return false;

	const int deadXSpace = frameCI.leftMargin + frameCI.rightMargin + frameCI.xSpacing * (frameCI.numXTiles - 1);
	const int frameWidth = (surf->w - deadXSpace) / frameCI.numXTiles;
	if (frameWidth <= 0) return false;

	const int deadYSpace = frameCI.topMargin + frameCI.bottomMargin + frameCI.ySpacing * (frameCI.numYTiles - 1);
	const int frameHeight = (surf->h - deadYSpace) / frameCI.numYTiles;
	if (frameHeight <= 0) return false;

	tilesPerRow = frameCI.numXTiles;
	numTiles = frameCI.numXTiles * frameCI.numYTiles;
	tiles = (SDL_Rect*)calloc(numTiles, sizeof(SDL_Rect));

	size_t tileIndex = 0;
	
	int yOffset = frameCI.topMargin;
	for (int yf = 0; yf < frameCI.numYTiles; ++yf)
	{
		int xOffset = frameCI.leftMargin;
		for (int xf = 0; xf < frameCI.numXTiles; ++xf)
		{
			SDL_Rect& currFrame = tiles[tileIndex];
			currFrame.x = xOffset;
			currFrame.y = yOffset;
			currFrame.w = frameWidth;
			currFrame.h = frameHeight;

			xOffset += frameWidth + frameCI.xSpacing;

			++tileIndex;
		}
		yOffset += frameHeight + frameCI.ySpacing;
	}
	return true;
}

int SpriteSheetSDL::GetTileIndex(int row, int col) const
{
	int idx = row * tilesPerRow + col;
	return (idx < numTiles) ? idx : 0;
}
