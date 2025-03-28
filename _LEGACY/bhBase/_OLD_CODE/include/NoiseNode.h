#ifndef _NOISENODE_H_
#define _NOISENODE_H_

#include "Common.h"
#include <wx/wx.h>
#include <noise.h>
#include "noiseutils.h"

////////////////////////////////////////////////////////////

static const int INVALID_ID = 0;

class NoiseNode
{
public:

	enum NodeGroup
	{
		GROUP_MODIFIER = 1,
		GROUP_COMBINER,
		GROUP_GENERATOR,
		GROUP_SELECTOR,
		GROUP_TRANSFORMER
	};

	enum NodeType
	{
		NODE_ABS = 1,
		NODE_CLAMP,
		NODE_CURVE,
		NODE_EXPONENT,
		NODE_INVERT,
		NODE_SCALEBIAS,
		NODE_TERRACE,
		NODE_ADD,
		NODE_MAX,
		NODE_MIN,
		NODE_MULTIPLY,
		NODE_POWER,
		NODE_BILLOW,
		NODE_CHECKERBOARD,
		NODE_CONST,
		NODE_CYLINDERS,
		NODE_PERLIN,
		NODE_RIDGEDMULTI,
		NODE_SPHERES,
		NODE_VORONOI,
		NODE_BLEND,
		NODE_SELECT,
		NODE_DISPLACE,
		NODE_ROTATEPOINT,
		NODE_SCALEPOINT,
		NODE_TRANSLATEPOINT,
		NODE_TURBULENCE
	};

	NoiseNode(const NodeType itype);
	
	~NoiseNode()
	{
		delete noiseModule;noiseModule = 0;
	}

	inline const std::string& GetName()
	{
		return name;
	}

	inline int GetId()
	{
		return id;
	}

	inline NodeType GetType()
	{
		return type;
	}

	inline const module::Module* GetModule()
	{
		return noiseModule;
	}

	inline const module::Module& GetSourceModule(int index)
	{
		return noiseModule->GetSourceModule(index);
	}

	inline int GetSourceModuleCount()
	{
		return noiseModule->GetSourceModuleCount();
	}

	inline void SetSourceModule(int index,const module::Module& sourceModule)
	{
		noiseModule->SetSourceModule(index,sourceModule);
	}

	void WriteNode(std::ofstream& ofile);
	void ReadNode(std::ifstream& ifile);

	static NoiseNode* GetNodeById(const int id)
	{
		if(aliveNodes.find(id) == aliveNodes.end())
		{
			return 0;
		}
		return aliveNodes[id];
	}

protected:

	module::Module* noiseModule;

	NodeGroup group;
	NodeType type;
	std::string name;

	static int genId;
	int id;
	int inputId[4];

private:

	static std::map<int,NoiseNode*> aliveNodes;
};

////////////////////////////////////////////////////////////

class EditorNoiseNode : public NoiseNode
{
public:

	struct Link
	{
		Link(int x,int y);
		Link(wxPoint pos);
		wxRect linkRect;
		EditorNoiseNode* source;
		inline void Connect(EditorNoiseNode* src = 0)
		{
			source = src;
		}
	};

	EditorNoiseNode(const NodeType type);
	~EditorNoiseNode(void);
	virtual void Render(wxDC& dc);
	
	inline bool Contains(const wxPoint& pt)
	{
		return geom.Contains(pt);
	}

	inline bool OutputContains(const wxPoint& pt)
	{
		return outputArea->Contains(pt - geom.GetPosition());
	}

	EditorNoiseNode::Link* CheckInput(const wxPoint& pt);
	void UpdateInput();
	bool InImage(const wxPoint& pt);
	
	inline static int GetConnectorSz()
	{
		return connectorSize;
	}

	wxPoint GetOutputCoord();

	inline void Move(const int x,const int y)
	{
		geom.SetX(geom.GetX() + x);
		geom.SetY(geom.GetY() + y);
	}

	void BuildImage();
	
	inline void SetSelected(bool select = true)
	{
		selected = select;
	}

protected:

	wxRect geom;

	utils::NoiseMap noiseMap;
	utils::Image image;
	wxImage* noiseImage;
	bool drawImage;
	
	static int thumbNailSize;
	bool selected;

	wxRect* outputArea;
	Link* inputLink[4];
	static const int connectorSize = 10;
	void RenderLink(wxDC& dc,short linkIndex);

private:

	static wxColour modifierColor;
	static wxColour combinerColor;
	static wxColour generatorColor;
	static wxColour selectorColor;
	static wxColour transformerColor;
	static wxColour penColor;
	static wxColour selectedPenColor;
	static int penWidth;
};

////////////////////////////////////////////////////////////

#endif
