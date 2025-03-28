#include "bhPhysics.h"
#include <btBulletDynamicsCommon.h>
#include "bhUtil.h"
#include "bhWorld.h"
#include <string>
#include <map>

namespace bhPhysics
{
    ////////////////////////////////////////////////////////////////////////////////
    inline btVector3 ToBtVector(bhVec3f const& v)
    {
        return btVector3(v.x, v.y, v.z);
    }

    inline bhVec3f FromBtVector(btVector3 const& v)
    {
        return bhVec3f(v.x(), v.y(), v.z());
    }

    ////////////////////////////////////////////////////////////////////////////////
    struct Context
    {
        btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
        btCollisionDispatcher* collisionDispatcher = nullptr;
        btBroadphaseInterface* broadphaseInterface = nullptr;
        btSequentialImpulseConstraintSolver* solver = nullptr;
        btDiscreteDynamicsWorld* world = nullptr;

        std::map<std::string, btCollisionShape*> namedCollisionShapes;
        btAlignedObjectArray<btCollisionShape*> collisionShapes;
    };

    static Context g_context;
    btRigidBody* playerCapsuleBody = nullptr;

    bool CreateContext(Context& context)
    {
        //btDefaultCollisionConstructionInfo collisionConstructionInfo = {};
        context.collisionConfiguration = new btDefaultCollisionConfiguration();
        context.collisionDispatcher = new btCollisionDispatcher(context.collisionConfiguration); // See also:  Extras/BulletMultiThreaded
        context.broadphaseInterface = new btDbvtBroadphase(); // See also: btAxis3Sweep etc.
        context.solver = new btSequentialImpulseConstraintSolver(); // See also:  Extras/BulletMultiThreaded
        context.world = new btDiscreteDynamicsWorld(context.collisionDispatcher, context.broadphaseInterface, context.solver, context.collisionConfiguration);
        context.world->setGravity(ToBtVector(bhWorld::UpVec * -9.807f));

        return true;
    }

    void ClearCollisionShapes(Context& context)
    {
        for (int cs = 0; cs < context.collisionShapes.size(); cs++)
        {
            delete context.collisionShapes[cs];
        }
        context.collisionShapes.clear();
        context.namedCollisionShapes.clear();
    }

    void DestroyContext(Context& context)
    {
        ClearCollisionShapes(context);

        bhUtil::DeleteAndNul(context.world);
        bhUtil::DeleteAndNul(context.solver);
        bhUtil::DeleteAndNul(context.broadphaseInterface);
        bhUtil::DeleteAndNul(context.collisionDispatcher);
        bhUtil::DeleteAndNul(context.collisionConfiguration);
    }

    ////////////////////////////////////////////////////////////////////////////////
    bool Create()
    {
        return CreateContext(g_context);
    }

    void Destroy()
    {
        DestroyContext(g_context);
    }

    void StepSimulation(float dTime)
    {
        g_context.world->stepSimulation(dTime);
    }

    void SetGravity(bhVec3f const& g)
    {
        assert(g_context.world != nullptr);
        g_context.world->setGravity(ToBtVector(g));
    }

    void ClearWorld()
    {
        for (int co = 0; co < g_context.world->getNumCollisionObjects(); ++co)
        {
            btCollisionObject* obj = g_context.world->getCollisionObjectArray()[co];
            btRigidBody* body = btRigidBody::upcast(obj);
            if (body && body->getMotionState())
            {
                delete body->getMotionState();
            }
            g_context.world->removeCollisionObject(obj);
            delete obj;
        }
        ClearCollisionShapes(g_context);
        playerCapsuleBody = nullptr;
    }

    void AddPlayerCapsule(bhVec3f const& origin)//, void* camera_p)
    {
        btCollisionShape* playerCapsule = new btCapsuleShapeZ(.5f, 0.5f);
        g_context.collisionShapes.push_back(playerCapsule);

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(ToBtVector(origin));
        //transform.setRotation();

        float mass = 75.f;
        btVector3 localInertia;
        playerCapsule->calculateLocalInertia(mass, localInertia);

        btDefaultMotionState* dms = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, dms, playerCapsule, localInertia);
        rbInfo.m_friction = 0.1f;
        rbInfo.m_spinningFriction = 0.1f;
        rbInfo.m_rollingFriction = 0.1f;

        playerCapsuleBody = new btRigidBody(rbInfo);
        //playerCapsuleBody->setUserPointer(camera_p);
        playerCapsuleBody->setAngularFactor(btVector3(0.f, 0.f, 1.f));

        //playerCapsuleBody->setCollisionFlags(playerCapsuleBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        //playerCapsuleBody->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
        playerCapsuleBody->setActivationState(DISABLE_DEACTIVATION);

        g_context.world->addRigidBody(playerCapsuleBody);
    }

    void MovePlayer(bhVec3f const& vel)
    {
        //playerCapsuleBody->applyForce(ToBtVector(vel), btVector3());
        playerCapsuleBody->setLinearVelocity(ToBtVector(vel));
    }

    bhVec3f GetPlayerPosition()
    {
        return FromBtVector(playerCapsuleBody->getWorldTransform().getOrigin());
    }

    void AddStaticBody(char const* name, bhVec3f const& location)
    {
        auto result = g_context.namedCollisionShapes.find(name);
        assert(result != g_context.namedCollisionShapes.end());
        if (result == g_context.namedCollisionShapes.end())
        {
            return;
        }

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(ToBtVector(location));

        btVector3 localInertia(0, 0, 0);

        btDefaultMotionState* dms = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(0.f, dms, result->second, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);

        //body->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);

        //add the body to the dynamics world
        g_context.world->addRigidBody(body);
    }

    void AddStaticBodies(char const* name, std::vector<bhVec3f> const& locations, size_t startIndex, size_t numInds)
    {
        startIndex = bhMath::Min(startIndex, locations.size());
        numInds = bhMath::Min(numInds, locations.size() - startIndex);

        auto result = g_context.namedCollisionShapes.find(name);
        assert(result != g_context.namedCollisionShapes.end());
        if (result == g_context.namedCollisionShapes.end())
        {
            return;
        }

        for (size_t b = 0; b < numInds; ++b)
        {
            btTransform transform;
            transform.setIdentity();
            transform.setOrigin(ToBtVector(locations[startIndex + b]));

            btVector3 localInertia(0, 0, 0);

            btDefaultMotionState* dms = new btDefaultMotionState(transform);
            btRigidBody::btRigidBodyConstructionInfo rbInfo(0.f, dms, result->second, localInertia);
            btRigidBody* body = new btRigidBody(rbInfo);

            //body->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);

            //add the body to the dynamics world
            g_context.world->addRigidBody(body);
        }
    }

    void CreateCollisionShape_Box(char const* name, bhVec3f const& size)
    {
        btVector3 btSiz = ToBtVector(size * 0.5f);
        btCollisionShape* boxShape = new btBoxShape(btSiz);
        g_context.collisionShapes.push_back(boxShape);
        g_context.namedCollisionShapes[name] = boxShape;
    }

    void DeleteCollisionShape(char const* name)
    {
        auto result = g_context.namedCollisionShapes.find(name);
        if (result == g_context.namedCollisionShapes.end())
        {
            return;
        }
        g_context.collisionShapes.remove(result->second);
        g_context.namedCollisionShapes.erase(name);
    }
}
