#ifndef BH_PARTICLE_H
#define BH_PARTICLE_H

#include "Box.h"
#include "Mesh.h"
#include "Material.h"

////////////////////////////////////////////////////////////////////////////////
namespace bh{

class ParticleEmitter
{
public:

	ParticleEmitter(char const* meshName,char const* matName,int iNumParticles,bool loop);
	~ParticleEmitter();
	void Update();
	void Render();
	void Reset();

	inline void SetPosition(glm::vec3 p)
	{
		position = p;
	}

protected:

	////////////////////////////////////////////////////////////////////////////////
	struct Particle
	{
		float energy;
		bool active;
		glm::vec3 position,vel,acc;
	};
	////////////////////////////////////////////////////////////////////////////////

	void ResetParticle(Particle* p);

	float energyDrain,resetEnergy;
	unsigned int numParticles;
	Material* material;
	Mesh* mesh;
	Particle* particles;
	bool active,looping;
	glm::vec3 position;

private:
};

};
////////////////////////////////////////////////////////////////////////////////
#endif
