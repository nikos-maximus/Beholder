#include "bhMap.h"
#include <vector>
#include <map>
#include <tinyxml2.h>
//#include <btBulletDynamicsCommon.h>
#include "bhRendering.h"
#include "bhCore.h"
#include "bhCamera.h"
#include "bhLight.h"
#include "bhMesh.h"
#include "bhMaterial.h"
#include "bhScene.h"
#include "bhLog.h"
#include "bhRect.h"

namespace bhMap
{
	////////////////////////////////////////////////////////////////////////////////
	// FileData

	char const* GetSectorTypeName(SectorTypes typeEnum)
	{
		static char const* names[NUM_SECTOR_TYPES] = 
		{
			"Solid",
			"Empty",
			"Door"
		};
		return names[typeEnum];
	}

	FileData::FileData()
		: lastSelectionX(-1)
		, lastSelectionY(-1)
	{
		Clear();
	}

	void FileData::Clear()
	{
		memset(wallMatName, '\0', sizeof(char) * ASSET_NAME_LEN);
		memset(floorMatName, '\0', sizeof(char) * ASSET_NAME_LEN);
		memset(ceilMatName, '\0', sizeof(char) * ASSET_NAME_LEN);
		memset(rawTiles, ST_SOLID, sizeof(uint8_t) * MAP_SIZ * MAP_SIZ);
	}

	bool FileData::Load(char const* path)
	{
		FILE* file = fopen(path, "rb");
		if (!file)
		{
			bhLog::Error("Could not open map file %s for reading", path);
			return false;
		}
		fread(wallMatName, sizeof(char), ASSET_NAME_LEN, file);
		fread(floorMatName, sizeof(char), ASSET_NAME_LEN, file);
		fread(ceilMatName, sizeof(char), ASSET_NAME_LEN, file);
		fread(rawTiles, sizeof(uint8_t), MAP_SIZ * MAP_SIZ, file);
		fclose(file);
		return true;
	}

	bool FileData::Save(char const* path)
	{
		FILE* file = fopen(path, "wb");
		if (!file)
		{
			bhLog::Error("Could not open map file %s for writing", path);
			return false;
		}
		fwrite(wallMatName, sizeof(char), ASSET_NAME_LEN, file);
		fwrite(floorMatName, sizeof(char), ASSET_NAME_LEN, file);
		fwrite(ceilMatName, sizeof(char), ASSET_NAME_LEN, file);
		fwrite(rawTiles, sizeof(uint8_t), MAP_SIZ * MAP_SIZ, file);
		fclose(file);
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////

	// Bullet
	//btBroadphaseInterface* g_broadphase = 0;
	//btCollisionConfiguration* g_collisionConfig = 0;
	//btCollisionDispatcher* g_collisionDispatcher = 0;
	//btSequentialImpulseConstraintSolver* g_solver = 0;
	//btDiscreteDynamicsWorld* g_dynamicsWorld = 0;

	// Graphics
	std::vector<bhLight*> lights;
	glm::vec3 camPos;
	static bhCamera* cam = 0;
	//static bhOctree* sceneTree = 0;
	static float walkSpeed = 0.1f;
	static std::vector<bhMesh*> meshes;
	static std::map<std::string, bhMesh*> namedMeshes;
	static std::map<std::string, bhMaterial*> namedMaterials;

	bhLight* camLight = 0;

	static bhSector* sceneTiles[MAP_SIZ][MAP_SIZ] = {};

	//void UpdatePhysicsWorld(float timeDelta);

	// Command handling
	void(*commandFuncs[bhCommandID::NUM_COMMANDS])(void) = {};

	bhMesh* LoadMesh(char const* name)
	{
		auto meshIt = namedMeshes.find(name);
		if (meshIt == namedMeshes.end())
		{
			bhMesh* newMesh = bhMesh::Load(name);
			if (newMesh)
			{
				namedMeshes[name] = newMesh;
				meshes.push_back(newMesh);
				return newMesh;
			}
			return 0;
		}
		return meshIt->second;
	}

	bhMaterial* LoadMaterial(char const* name)
	{
		auto matIt = namedMaterials.find(name);
		if (matIt == namedMaterials.end())
		{
			//bhMaterial* newMat = bhMaterial::Load(name);
			bhMaterial* newMat = bhMaterial::CreateFromFile(name);
			if (newMat)
			{
				namedMaterials[name] = newMat;
				return newMat;
			}
			return 0;
		}
		return matIt->second;
	}

	void InitCommandFuncs()
	{
		commandFuncs[CMD_FWD] = [](){ camPos += (cam->GetForwardVec() * walkSpeed); };
		commandFuncs[CMD_BACK] = [](){ camPos -= (cam->GetForwardVec() * walkSpeed); };
		commandFuncs[CMD_STRAFE_LEFT] = [](){ camPos -= (cam->GetRightVec() * walkSpeed); };
		commandFuncs[CMD_STRAFE_RIGHT] = [](){ camPos += (cam->GetRightVec() * walkSpeed); };
		commandFuncs[CMD_UP] = [](){ camPos += (bhCore::GetWorldUpVec() * walkSpeed); };
		commandFuncs[CMD_DOWN] = [](){ camPos -= (bhCore::GetWorldUpVec() * walkSpeed); };
	}

	void HandleCommands(uint8_t const* commands)
	{
		glm::vec3 dir;
		for (uint8_t cmd = 0; cmd < bhCommandID::NUM_COMMANDS; ++cmd)
		{
			if (commands[cmd] && commandFuncs[cmd])
			{
				commandFuncs[cmd]();
			}
		}
	}

	bool InitPhysics();
	bool DestroyPhysics();

	bool TileIsNull(uint8_t const sceneTiles[MAP_SIZ][MAP_SIZ], int x, int y)
	{
		//assert((x > 0) && (x < MAP_SIZ - 1) && (y > 0) && (y < MAP_SIZ - 1));
		return (sceneTiles[y][x] == ST_SOLID) &&
			(sceneTiles[y - 1][x] == ST_SOLID) &&
			(sceneTiles[y + 1][x] == ST_SOLID) &&
			(sceneTiles[y][x - 1] == ST_SOLID) &&
			(sceneTiles[y][x + 1] == ST_SOLID);
	}

	void Create(FileData const* fd)
	{
		//InitPhysics();

		bhMaterial* defWallMat = LoadMaterial(fd->wallMatName);
		bhMaterial* defFloorMat = LoadMaterial(fd->floorMatName);
		bhMaterial* defCeilMat = LoadMaterial(fd->ceilMatName);

		bhMesh* planeMesh = bhMesh::CreateSquare(TILE_SZ, bhMesh::M_NORMALS | bhMesh::M_UV | bhMesh::M_TANGENTS | bhMesh::M_BITANGENTS);
		meshes.push_back(planeMesh);

		bhMesh* cubeMesh = bhMesh::CreateCube(TILE_SZ, bhMesh::M_NORMALS | bhMesh::M_UV | bhMesh::M_TANGENTS | bhMesh::M_BITANGENTS);
		meshes.push_back(cubeMesh);

		//cubeMesh->ScaleUV(2.0f, 2.0f);
		//bhMesh* cylinderMesh = bhMesh::CreateCylinder(2.0f, 3.0f, 10, bhMesh::M_NORMALS | bhMesh::M_UV | bhMesh::M_TANGENTS | bhMesh::M_BITANGENTS);
		//cylinderMesh->ScaleUV(3.0f, 1.0f);

		float halfTileSz = TILE_SZ * 0.5f;

		for (int8_t y = 0; y < MAP_SIZ; ++y)
		{
			for (int8_t x = 0; x < MAP_SIZ; ++x)
			{
				glm::vec3 tilePos((x + 0.5f) * TILE_SZ, (y + 0.5f) * TILE_SZ, 0.0f);
				glm::vec3 tileSiz(TILE_SZ, TILE_SZ, TILE_SZ);

				switch (fd->rawTiles[y][x])
				{
					case ST_EMPTY:
					{
						sceneTiles[y][x] = new bhSector(tilePos, tileSiz);

						bhMeshComponent mc1(planeMesh, defFloorMat);
						sceneTiles[y][x]->AddMeshComponent(mc1);

						bhMeshComponent mc2(planeMesh, defCeilMat);
						mc2.Translate(0.0f, 0.0f, TILE_SZ);
						mc2.Rotate(glm::vec3(1.0f, 0.0f, 0.0f), 180.0f);
						sceneTiles[y][x]->AddMeshComponent(mc2);

						if ((rand() % 5) < 1)
						{
							bhLight* light = bhLight::New();
							glm::vec3 color(rand() % 255, rand() % 255, rand() % 255);
							color /= 255.0f;
							light->SetDiffuse(color);
							light->SetPosition(glm::vec3(x * TILE_SZ + halfTileSz, y * TILE_SZ + halfTileSz, halfTileSz));
							lights.push_back(light);
						}

						break;
					}
					case ST_SOLID:
					{
						if (TileIsNull(fd->rawTiles, x, y))
						{
							continue;
						}
						sceneTiles[y][x] = new bhSector(tilePos, tileSiz);

						bhMeshComponent mc(cubeMesh, defWallMat);
						mc.Translate(0.0f, 0.0f, halfTileSz);
						sceneTiles[y][x]->AddMeshComponent(mc);

						// Bullet
						//btScalar mass = 100.0f;
						//btVector3 inertia;
						//btCollisionShape* newBoxShape = new btBoxShape(btVector3(TILE_SZ, TILE_SZ, TILE_SZ));
						//newBoxShape->calculateLocalInertia(mass, inertia);
						//btDefaultMotionState* newBoxMS = new btDefaultMotionState(btTransform(btQuaternion(), btVector3(t[3][0], t[3][1], t[3][2])));
						//btRigidBody::btRigidBodyConstructionInfo newBoxCI(mass, newBoxMS, newBoxShape, inertia);
						//btRigidBody* newBoxRB = new btRigidBody(newBoxCI);
						//g_dynamicsWorld->addRigidBody(newBoxRB);
						// Bullet

						break;
					}
					default:
					{
						break;
					}
				}
			}
		}

		camLight = bhLight::New();
		lights.push_back(camLight);

		cam = bhCamera::New(bhRendering::GetViewportAspect());
		//camPos = glm::vec3(-TILE_SZ, -TILE_SZ, TILE_SZ * 0.5f);
		camPos = glm::vec3(0.0f, 0.0f, TILE_SZ * 0.5f);
		cam->SetPosition(camPos);
		cam->LookAt(glm::vec3(1.0f, 1.0f, TILE_SZ * 0.5f));

		InitCommandFuncs();
	}

	bool Load(char const* name)
	{
		char const* path = bhCore::CreatePath(bhCore::GetEnvString(bhCore::MAP_PATH), name);
		FileData fd;
		if (fd.Load(path))
		{
			Create(&fd);
			return true;
		}
		return false;
	}

	//bool Save(char const* name)
	//{
	//	char const* path = bhCore::CreatePath(bhCore::GetEnvString(bhCore::MAP_PATH), name);
	//	FILE* file = fopen(path, "wb");
	//	if (!file)
	//	{
	//		bhLog::Error("Could not open map file %s for writing", path);
	//		return false;
	//	}
	//	fclose(file);
	//	return true;
	//}

	bool Destroy()
	{
		//DestroyPhysics();
		
		lights.clear();
		
		// Delete sceneTiles
		for (int8_t y = 0; y < MAP_SIZ; ++y)
		{
			for (int8_t x = 0; x < MAP_SIZ; ++x)
			{
				delete sceneTiles[y][x];
			}
		}
		memset(sceneTiles, 0, sizeof(uint8_t) * MAP_SIZ * MAP_SIZ);

		for (auto m : meshes)
		{
			bhMesh::Delete(m);
		}
		meshes.clear();
		namedMeshes.clear();
		for (auto m : namedMaterials)
		{
			bhMaterial::Delete(m.second);
		}
		namedMaterials.clear();
		bhCamera::Delete(cam);
		return true;
	}

	void Update(float timeDelta, double mouseRelx, double mouseRely)
	{
		// Commit input effects
		// Update Physics - world state
		// Update scene manager state for next frame (cam - visibility)

		cam->Yaw(float(-mouseRelx));
		cam->Pitch(float(-mouseRely));
		//sceneTree->UpdateVisibility(cam);

		//UpdatePhysicsWorld(timeDelta);

		cam->SetPosition(camPos);
		camLight->SetPosition(camPos);
	}

	void Render()
	{
		GLuint prog = 0;
		GLint mvpULocation = 0;

		glEnable(GL_DEPTH_TEST);
		bhRendering::UseProgram("EarlyDepth");
		bhTransform::QueryUniforms();
		cam->SetViewVMP_Only();

		glClear(GL_DEPTH_BUFFER_BIT);
		for (int8_t y = 0; y < MAP_SIZ; ++y)
		{
			for (int8_t x = 0; x < MAP_SIZ; ++x)
			{
				bhSector* sec = sceneTiles[y][x];
				if (!sec)
				{
					continue;
				}
				for (auto c : sec->GetMeshComponents())
				{
					bhMeshComponent::RenderEarlyZ(&c);
				}
			}
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		bhRendering::UseProgram("Scene");
		bhTransform::QueryUniforms();
		cam->SetView();

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		for (auto ls : lights)
		{
			if (!ls->IsActive())
			{
				continue;
			}
			ls->Render();
			bhBox lightArea(ls->GetPosition(), ls->GetRadius());
			for (int8_t y = 0; y < MAP_SIZ; ++y)
			{
				for (int8_t x = 0; x < MAP_SIZ; ++x)
				{
					bhSector* sec = sceneTiles[y][x];
					if (!sec)
					{
						continue;
					}
					if (sec->Overlaps(lightArea))
					{
						for (auto c : sec->GetMeshComponents())
						{
							bhMeshComponent::Render(&c);
						}
					}
				}
			}
		}

		glDisable(GL_BLEND);

		// DEBUG
		glUseProgram(0);
		//cam->DEBUG_SetView_FixedFunction();
		cam->SetView();
		bhRendering::DEBUG_DrawOrigin();
		// DEBUG
	}

	/*
	void UpdatePhysicsWorld(float timeDelta)
	{
		// See ref:
		// http://bulletphysics.org/mediawiki-1.5.8/index.php/Stepping_the_World
		static const btScalar fixedTimeStep = 1.0f / 60.0f;
		g_dynamicsWorld->stepSimulation(timeDelta, 10, fixedTimeStep); // TODO: Parameterize these values 

	}
	
	void PreInternalTickCallback(btDynamicsWorld* world, btScalar timestep)
	{}

	void PostInternalTickCallback(btDynamicsWorld* world, btScalar timestep)
	{}

	bool InitPhysics()
	{
		g_broadphase = new btDbvtBroadphase();
		g_collisionConfig = new btDefaultCollisionConfiguration();
		g_collisionDispatcher = new btCollisionDispatcher(g_collisionConfig);
		g_solver = new btSequentialImpulseConstraintSolver();
		g_dynamicsWorld = new btDiscreteDynamicsWorld(g_collisionDispatcher, g_broadphase, g_solver, g_collisionConfig);

		g_dynamicsWorld->setInternalTickCallback(PreInternalTickCallback, 0, true);
		g_dynamicsWorld->setInternalTickCallback(PostInternalTickCallback, 0, false);

		g_dynamicsWorld->setGravity(btVector3(0.0f, 0.0f, -9.8f));

		return true;
	}

	bool DestroyPhysics()
	{
		delete g_dynamicsWorld;
		delete g_solver;
		delete g_collisionDispatcher;
		delete g_collisionConfig;
		delete g_broadphase;
		return true;
	}
	*/
}

#ifdef BH_AUTHORING // Editing functionality

void RenderEditorGrid(FileData const* fd)
{
	glDisable(GL_DEPTH_TEST);

	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	float mapSize = MAP_SIZ * TILE_SZ;
	float width = 0.0f, height = 0.0f;
	if (vp[2] > vp[3])
	{
		height = mapSize;
		width = height * float(vp[2]) / float(vp[3]);
	}
	else
	{
		width = mapSize;
		height = width * float(vp[3]) / float(vp[2]);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, 0.0, height, 0.1, 10.0);
	gluLookAt(0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	for (uint8_t y = 0; y < MAP_SIZ; ++y)
	{
		for (uint8_t x = 0; x < MAP_SIZ; ++x)
		{
			float ox = x * TILE_SZ;
			float oy = y * TILE_SZ;

			if (fd->rawTiles[y][x])
			{
				glColor3f(0.8f, 0.8f, 0.8f); // Light
			}
			else
			{
				glColor3f(0.4f, 0.4f, 0.4f); // Dark
			}

			glBegin(GL_QUADS);
			glVertex2f(ox, oy);
			glVertex2f(ox + TILE_SZ, oy);
			glVertex2f(ox + TILE_SZ, oy + TILE_SZ);
			glVertex2f(ox, oy + TILE_SZ);
			glEnd();

			glColor3f(0.0f, 0.0f, 0.0f);
			glLineWidth(1.5f);
			glBegin(GL_LINE_LOOP);
			glVertex2f(ox, oy);
			glVertex2f(ox + TILE_SZ, oy);
			glVertex2f(ox + TILE_SZ, oy + TILE_SZ);
			glVertex2f(ox, oy + TILE_SZ);
			glEnd();
			glLineWidth(1.0f);
		}
	}
	if ((fd->lastSelectionX > -1) && (fd->lastSelectionY > -1))
	{
		float ox = fd->lastSelectionX * TILE_SZ;
		float oy = fd->lastSelectionY * TILE_SZ;
		glColor3f(1.0f, 0.0f, 0.0f);
		glLineWidth(2.0f);
		glBegin(GL_LINE_LOOP);
		glVertex2f(ox, oy);
		glVertex2f(ox + TILE_SZ, oy);
		glVertex2f(ox + TILE_SZ, oy + TILE_SZ);
		glVertex2f(ox, oy + TILE_SZ);
		glEnd();
		glLineWidth(1.0f);
	}
}

void EditorSelectTile(FileData* fd, float x, float y, MapSelectionMode mode)
{
	x *= MAP_SIZ;
	y *= MAP_SIZ;
	if (!((x >= 0.0f) && (x < MAP_SIZ) && (y >= 0.0f) && (y < MAP_SIZ)))
	{
		return;
	}
	int ix = (int)glm::floor(x);
	int iy = (int)glm::floor(y);

	if (mode == SEL_ACTIVATE)
	{
		fd->rawTiles[iy][ix] = ST_EMPTY;
	}
	else if (mode == SEL_DEACTIVATE)
	{
		fd->rawTiles[iy][ix] = ST_SOLID;
	}
	fd->lastSelectionX = static_cast<int8_t>(x);
	fd->lastSelectionY = static_cast<int8_t>(y);
}

#endif
