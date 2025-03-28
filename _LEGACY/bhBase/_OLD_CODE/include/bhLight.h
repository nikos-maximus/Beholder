#pragma once
#include "bhDefines.h"
#include "bhColor.h"
#include "ECS/bhEntity.h"

////////////////////////////////////////////////////////////////////////////////
class bhLight
{
public:
	bhLight();
	void SetRadius(float newRadius);
	void SetIntensity(float newIntensity);
	
	float GetArea() const 
	{ 
		return effectArea; 
	}

	void Activate() { flags |= LIGHT_ACTIVE; }
	void Deactivate() { flags &= ~LIGHT_ACTIVE; }

protected:
private:
	enum Flags
	{
		LIGHT_ACTIVE = BH_BIT(0)
	};

	enum Type
	{
		LIGHT_TYPE_AMBIENT,
		LIGHT_TYPE_DIRECTIONAL,
		LIGHT_TYPE_POINT,
		LIGHT_TYPE_SPOT
	};

	bhColor4f color;
	float intensity = 1.f;
	float sourceRadius = 1.f;
	float effectArea = 0.f;
	int flags = 0;
};

////////////////////////////////////////////////////////////////////////////////
class bhCLight : public bhComponent
{
public:
	bhLight light;

protected:
private:
};

class bhELightSource : public bhEntity
{
public:
	bhTransform transform;
	bhLight light;

protected:
private:
};
