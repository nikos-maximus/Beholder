#ifndef USERNODE_H
#define USERNODE_H

#include "Game.h"
#include "SceneNode.h"

class Camera;

////////////////////////////////////////////////////////////////////////////////

class UserNode : public SceneNode, public IEventListener
{
public:

	UserNode(SceneNode* iparent = 0);
	~UserNode(void);
	//virtual void Render(const Camera* cam);
	void OnEvent(const Event& evt);
	void AssignCamera(Camera* icam);

	inline void ReleaseCamera()
	{
		cam = 0;
	}

protected:

	GTMath::vec3<float> vel;
	GTMath::vec3<float> accel;
	Camera* cam;

private:
};

////////////////////////////////////////////////////////////////////////////////

#endif
