#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

#include "Game.h"
#include "Camera.h"
#include "UserNode.h"

////////////////////////////////////////////////////////////////////////////////

class TiXmlElement;
class Terrain;

namespace GTCore
{
	class SceneManager
	{
	public:

		SceneManager();
		~SceneManager();
		Camera* AddCamera(const std::string& cn);
		bool SetActiveCamera(const std::string& cn);
		virtual bool Load(const TiXmlElement* rootElement);
		virtual bool Save();
		virtual void Update();
		virtual void Render();

	protected:

		SceneNode* root;
		Camera* activeCamera;
		UserNode* activeUserNode;
		//Terrain* terrain;
		
	private:

		std::map<std::string,Camera*> cameras;
	};
};

////////////////////////////////////////////////////////////////////////////////

#endif
