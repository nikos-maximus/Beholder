#ifndef GT_PANEL_H
#define GT_PANEL_H

#define GT_UI_RENDER_OPENGL

#include "Window.h"
#include "../Material.h"

#ifdef GT_UI_RENDER_OPENGL
#include <glm/glm.hpp>
#endif

////////////////////////////////////////////////////////////////////////////////
namespace bh{

class Panel : public Window
{
public:

	Panel();
	virtual void ReadDesc(TiXmlElement const* el);
	virtual void Render();

protected:

#ifdef GT_UI_RENDER_OPENGL

	void GenOpenGLData();

	glm::vec2 coords[4];
	glm::vec2 texCoords[4];
	unsigned short inds[6];
	Material* material;

#endif

private:
};

};
////////////////////////////////////////////////////////////////////////////////
#endif
