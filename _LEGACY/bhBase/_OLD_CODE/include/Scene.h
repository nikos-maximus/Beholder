#ifndef SCENE_H
#define SCENE_H

#include "Camera.h"
#include "Sprite.h"

using std::vector;

////////////////////////////////////////////////////////////

class Scene
{
public:

	Scene(void);
	~Scene(void);
	void Render();
	bool Init();

protected:

	Sprite* background;
	vector<Sprite*> bgSprites;
	vector<Sprite*> actors;
	vector<Sprite*> super;

	Camera cam;

private:
};

////////////////////////////////////////////////////////////

#endif
