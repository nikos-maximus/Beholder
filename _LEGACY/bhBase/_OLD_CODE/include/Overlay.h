#ifndef BH_OVERLAY_H
#define BH_OVERLAY_H

#include "Window.h"

////////////////////////////////////////////////////////////////////////////////
namespace bh{

class Overlay : public Window
{
public:

	Overlay(short w,short h);
	Overlay(short x,short y,short w,short h);
	Overlay(char const* configName);
	void Render();

protected:

	void ReadConfig(char const* fileName);

private:
};

};
////////////////////////////////////////////////////////////////////////////////
#endif
