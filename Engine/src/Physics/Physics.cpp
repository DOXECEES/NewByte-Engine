#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>

#include <Jolt/Physics/Collision/ContactListener.h>

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include "Error/ErrorManager.hpp"
#include "Physics.hpp"

#include "Scripting/ScriptComponent.hpp"

#include <thread>
#include <vector>

namespace nb::Physics
{
    struct TriggerEvent
    {
        Ecs::EntityID triggerEntity;
        Ecs::EntityID otherEntity;
        bool          entered;
    };

    static std::vector<TriggerEvent> triggerEventQueue;
    static std::mutex                triggerMutex;

    class MyContactListener : public JPH::ContactListener
    {
    public:
        void OnContactAdded(
            const JPH::Body&            inBody1,
            const JPH::Body&            inBody2,
            const JPH::ContactManifold& inManifold,
            JPH::ContactSettings&       ioSettings
        ) override
        {
            Ecs::EntityID id1 = (Ecs::EntityID)inBody1.GetUserData();
            Ecs::EntityID id2 = (Ecs::EntityID)inBody2.GetUserData();

            std::lock_guard<std::mutex> lock(triggerMutex);
            if (inBody1.IsSensor())
            {
                triggerEventQueue.push_back({id1, id2, true});
            }
            if (inBody2.IsSensor())
            {
                triggerEventQueue.push_back({id2, id1, true});
            }
        }

        void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
        {
            
        }
    };

    static MyContactListener* globalContactListener = nullptr;



    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl()
        {
            objectToBroadPhase[Layers::NON_MOVING] = JPH::BroadPhaseLayer(0);
            objectToBroadPhase[Layers::MOVING]     = JPH::BroadPhaseLayer(1);
        }

        unsigned int GetNumBroadPhaseLayers() const override
        {
            return 2;
        }

        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
        {
            return objectToBroadPhase[inLayer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
        {
            switch ((JPH::BroadPhaseLayer::Type)inLayer)
            {
            case 0:
                return "NON_MOVING";

            case 1:
                return "MOVING";

            default:
                return "INVALID";
            }
        }
#endif

    private:
        JPH::BroadPhaseLayer objectToBroadPhase[Layers::NUM_LAYERS];
    };

    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(
            JPH::ObjectLayer inObject1,
            JPH::ObjectLayer inObject2
        ) const override
        {
            switch (inObject1)
            {
            case Layers::NON_MOVING:
                return inObject2 == Layers::MOVING;

            case Layers::MOVING:
                return true;

            default:
                return false;
            }
        }
    };

    class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(
            JPH::ObjectLayer     inLayer1,
            JPH::BroadPhaseLayer inLayer2
        ) const override
        {
            if (inLayer1 == Layers::NON_MOVING)
            {
                return inLayer2 == JPH::BroadPhaseLayer(1);
            }

            if (inLayer1 == Layers::MOVING)
            {
                return true;
            }

            return false;
        }
    };

    enum class CmdType
    {
        Force,
        Torque,
        Impulse
    };

    struct PhysicsCmd
    {
        CmdType     type;
        JPH::BodyID id;
        JPH::Vec3   v;
    };

    static std::vector<PhysicsCmd> commandQueue;

    static PhysicsSystem* instance = nullptr;

    static void applyCommandQueue(JPH::BodyInterface& bodyInterface)
    {
        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::INFO, "applyCommandQueue begin")
            .with("count", (uint64_t)commandQueue.size());

        for (const auto& cmd : commandQueue)
        {
            if (cmd.id.IsInvalid())
            {
                nb::Error::ErrorManager::instance().report(
                    nb::Error::Type::WARNING, "PhysicsCmd ignored: invalid BodyID"
                );

                continue;
            }

            bool added  = bodyInterface.IsAdded(cmd.id);
            bool active = false;

            if (added)
            {
                active = bodyInterface.IsActive(cmd.id);
            }

            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::INFO, "PhysicsCmd apply")
                .with("type", (int)cmd.type)
                .with("bodyIndexAndSeq", (uint64_t)cmd.id.GetIndexAndSequenceNumber())
                .with("added", added)
                .with("active", active)
                .with("vx", cmd.v.GetX())
                .with("vy", cmd.v.GetY())
                .with("vz", cmd.v.GetZ());

            if (!added)
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::WARNING, "PhysicsCmd ignored: body not added")
                    .with("bodyIndexAndSeq", (uint64_t)cmd.id.GetIndexAndSequenceNumber());

                continue;
            }

            switch (cmd.type)
            {
            case CmdType::Force:
                bodyInterface.AddForce(cmd.id, cmd.v, JPH::EActivation::Activate);
                break;

            case CmdType::Torque:
                bodyInterface.AddTorque(cmd.id, cmd.v, JPH::EActivation::Activate);
                break;

            case CmdType::Impulse:
                bodyInterface.AddImpulse(cmd.id, cmd.v);
                break;
            }
        }

        commandQueue.clear();

        nb::Error::ErrorManager::instance().report(nb::Error::Type::INFO, "applyCommandQueue end");
    }

    PhysicsSystem& PhysicsSystem::getInstance()
    {
        return *instance;
    }

    PhysicsSystem::PhysicsSystem()
    {
        if (instance != nullptr)
        {
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::FATAL, "PhysicsSystem created twice! This will break BodyID logic."
            );

            return;
        }

        instance = this;

        nb::Error::ErrorManager::instance().report(nb::Error::Type::INFO, "PhysicsSystem created");
    }

    PhysicsSystem::~PhysicsSystem()
    {
        nb::Error::ErrorManager::instance().report(
            nb::Error::Type::INFO, "PhysicsSystem destroyed"
        );

        if (physicsSystem)
        {
            delete physicsSystem;
        }

        if (jobSystem)
        {
            delete jobSystem;
        }

        if (tempAllocator)
        {
            delete tempAllocator;
        }

        if (globalContactListener)
        {
            delete globalContactListener;
        }

        delete (BPLayerInterfaceImpl*)bpInterface;
        delete (ObjectLayerPairFilterImpl*)objFilter;
        delete (ObjectVsBroadPhaseLayerFilterImpl*)bpFilter;

        if (instance == this)
        {
            instance = nullptr;
        }
    }

    void PhysicsSystem::init()
    {
        nb::Error::ErrorManager::instance().report(
            nb::Error::Type::INFO, "PhysicsSystem init start"
        );

        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);

        uint32_t threadCount = std::thread::hardware_concurrency();
        if (threadCount > 1)
        {
            threadCount -= 1;
        }

        jobSystem = new JPH::JobSystemThreadPool(
            JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, threadCount
        );

        bpInterface = new BPLayerInterfaceImpl();
        objFilter   = new ObjectLayerPairFilterImpl();
        bpFilter    = new ObjectVsBroadPhaseLayerFilterImpl();

        physicsSystem = new JPH::PhysicsSystem();

        physicsSystem->Init(
            1024, 0, 1024, 1024, *(BPLayerInterfaceImpl*)bpInterface,
            *(ObjectVsBroadPhaseLayerFilterImpl*)bpFilter, *(ObjectLayerPairFilterImpl*)objFilter
        );

        globalContactListener = new MyContactListener();
        physicsSystem->SetContactListener(globalContactListener);


        physicsSystem->SetGravity(JPH::Vec3(0, -9.81f, 0));

        isInitialized = true;

        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::INFO, "PhysicsSystem Init done")
            .with("gravityY", -9.81f);
    }

    void PhysicsSystem::clear()
    {
        if (!isInitialized || !physicsSystem)
        {
            return;
        }

        JPH::BodyInterface& bodyInterface = physicsSystem->GetBodyInterface();

        JPH::BodyIDVector allBodies;
        physicsSystem->GetBodies(allBodies);

        if (!allBodies.empty())
        {
            bodyInterface.RemoveBodies(allBodies.data(), (int)allBodies.size());
            bodyInterface.DestroyBodies(allBodies.data(), (int)allBodies.size());
        }

        commandQueue.clear();

        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::INFO, "PhysicsSystem cleared")
            .with("bodiesRemoved", (uint64_t)allBodies.size());
    }

    void PhysicsSystem::createRigidbody(
        Ecs::EntityID entityId,
        Scene&        scene
    )
    {
        auto& rb = scene.getComponent<Rigidbody>(entityId);
        auto& tc = scene.getComponent<TransformComponent>(entityId);
        auto& c  = scene.getComponent<Collider>(entityId);

        nb::Math::Vector3<float> worldScale = nb::Math::getScaleFromModelMatrix(tc.worldMatrix);

        JPH::ShapeRefC shape;
        switch (c.type)
        {
        case ColliderType::BOX:
        {
            JPH::BoxShapeSettings settings(
                JPH::Vec3(
                    std::max(0.01f, c.halfSize.x * worldScale.x),
                    std::max(0.01f, c.halfSize.y * worldScale.y),
                    std::max(0.01f, c.halfSize.z * worldScale.z)
                )
            );
            shape = settings.Create().Get();
            break;
        }
        case ColliderType::SPHERE:
        {
            float maxScale = std::max({worldScale.x, worldScale.y, worldScale.z});
            JPH::SphereShapeSettings settings(std::max(0.01f, c.radius * maxScale));
            shape = settings.Create().Get();
            break;
        }
        case ColliderType::CAPSULE:
        {
            JPH::CapsuleShapeSettings settings(
                c.height * 0.5f * worldScale.y, c.radius * worldScale.x
            );
            shape = settings.Create().Get();
            break;
        }
        }

        if (c.offset.squaredLength() > 0.0001f)
        {
            JPH::RotatedTranslatedShapeSettings offsetSettings(
                JPH::Vec3(c.offset.x, c.offset.y, c.offset.z), JPH::Quat::sIdentity(), shape
            );
            shape = offsetSettings.Create().Get();
        }

        JPH::EMotionType motionType =
            rb.isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
        JPH::ObjectLayer layer = rb.isStatic ? Layers::NON_MOVING : Layers::MOVING;

        nb::Math::Vector3<float>    wPos = nb::Math::getPositionFromModelMatrix(tc.worldMatrix);
        nb::Math::Quaternion<float> wRot = nb::Math::getRotationFromModelMatrix(tc.worldMatrix);

        JPH::BodyCreationSettings settings(
            shape, JPH::RVec3(wPos.x, wPos.y, wPos.z),
            JPH::Quat(-wRot.x, -wRot.y, -wRot.z, wRot.w), 
            motionType, layer
        );

        settings.mUserData = (JPH::uint64)entityId;
        settings.mIsSensor = rb.isTrigger;

        if (!rb.isStatic)
        {
            settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
            JPH::MassProperties mp           = shape->GetMassProperties();
            mp.ScaleToMass(rb.mass);
            settings.mMassPropertiesOverride = mp;
        }

        settings.mFriction    = rb.friction;
        settings.mRestitution = rb.restitution;

        JPH::BodyInterface& bodyInterface = physicsSystem->GetBodyInterface();
        JPH::Body*          body          = bodyInterface.CreateBody(settings);

        if (body)
        {
            rb.bodyID = body->GetID();
            bodyInterface.AddBody(rb.bodyID, JPH::EActivation::Activate);
        }
    }

    void PhysicsSystem::update(
        Scene& scene,
        float  dt
    )
    {
        if (!isInitialized)
        {
            return;
        }

        auto                view = scene.getEntitiesWith<Rigidbody, TransformComponent, Collider>();
        JPH::BodyInterface& bodyInterface = physicsSystem->GetBodyInterface();

        for (auto entity : view)
        {
            auto& rb = scene.getComponent<Rigidbody>(entity.id);
            auto& tc = scene.getComponent<TransformComponent>(entity.id);

            if (rb.bodyID.IsInvalid())
            {
                createRigidbody(entity.id, scene);
            }

            if (tc.physicsDirty)
            {
                if (!rb.bodyID.IsInvalid() && bodyInterface.IsAdded(rb.bodyID))
                {
                    nb::Math::Vector3<float> wPos =
                        nb::Math::getPositionFromModelMatrix(tc.worldMatrix);
                    nb::Math::Quaternion<float> wRot =
                        nb::Math::getRotationFromModelMatrix(tc.worldMatrix);

                    bodyInterface.SetPositionAndRotation(
                        rb.bodyID, JPH::RVec3(wPos.x, wPos.y, wPos.z),
                        JPH::Quat(-wRot.x, -wRot.y, -wRot.z, wRot.w), /
                        JPH::EActivation::Activate
                    );
                }
                tc.physicsDirty = false;
            }
        }

        if (scene.isPaused)
        {
            return;
        }

        applyCommandQueue(bodyInterface);

        physicsSystem->Update(dt, 1, tempAllocator, jobSystem);

        {
            std::lock_guard<std::mutex> lock(triggerMutex);
            for (const auto& event : triggerEventQueue)
            {
                if (scene.hasComponent<nb::Script::ScriptComponent>(event.triggerEntity))
                {
                    auto& script =
                        scene.getComponent<nb::Script::ScriptComponent>(event.triggerEntity);

                    if (event.entered)
                    {
                        script.call("onTriggerEnter", event.otherEntity);
                    }
                    else
                    {
                        script.call("onTriggerExit", event.otherEntity);
                    }
                }
            }
            triggerEventQueue.clear();
        }

        for (auto entity : view)
        {
            auto& rb = scene.getComponent<Rigidbody>(entity.id);
            auto& tc = scene.getComponent<TransformComponent>(entity.id);

            if (rb.isStatic || rb.bodyID.IsInvalid() || !bodyInterface.IsActive(rb.bodyID))
            {
                continue;
            }

            JPH::RVec3 pos;
            JPH::Quat  rot;
            bodyInterface.GetPositionAndRotation(rb.bodyID, pos, rot);

            nb::Math::Vector3<float> W_pos((float)pos.GetX(), (float)pos.GetY(), (float)pos.GetZ());
            nb::Math::Quaternion<float> W_rot(
                -(float)rot.GetX(), -(float)rot.GetY(), -(float)rot.GetZ(), (float)rot.GetW()
            );

            nb::Node currentNode = scene.getNode(entity.id);
            auto     parentOpt   = currentNode.getParent();

            if (parentOpt.has_value())
            {
                auto& ptc = parentOpt->getComponent<TransformComponent>();

                nb::Math::Vector3<float> pWPos =
                    nb::Math::getPositionFromModelMatrix(ptc.worldMatrix);
                nb::Math::Quaternion<float> pWRot =
                    nb::Math::getRotationFromModelMatrix(ptc.worldMatrix);
                nb::Math::Vector3<float> pWScale =
                    nb::Math::getScaleFromModelMatrix(ptc.worldMatrix);

                nb::Math::Vector3<float> deltaPos       = W_pos - pWPos;
                nb::Math::Vector3<float> unrotatedDelta = pWRot.conjugate() * deltaPos;

                tc.position.x = (pWScale.x != 0) ? (unrotatedDelta.x / pWScale.x) : 0.0f;
                tc.position.y = (pWScale.y != 0) ? (unrotatedDelta.y / pWScale.y) : 0.0f;
                tc.position.z = (pWScale.z != 0) ? (unrotatedDelta.z / pWScale.z) : 0.0f;

                tc.rotation = W_rot * pWRot.conjugate();
            }
            else
            {
                tc.position = W_pos;
                tc.rotation = W_rot;
            }

            tc.rotation.normalize();
            tc.eulerAngle = tc.rotation.toEulerXYZ();
            tc.lastEuler  = tc.eulerAngle;

            tc.dirty = true;
        }
    }

    JPH::BodyInterface& PhysicsSystem::getBodyInterface()
    {
        return physicsSystem->GetBodyInterface();
    }

    void Rigidbody::addForce(const Math::Vector3<float>& f)
    {
        if (bodyID.IsInvalid())
        {
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::WARNING, "addForce ignored: invalid BodyID"
            );

            return;
        }

        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::INFO, "addForce queued")
            .with("bodyIndexAndSeq", (uint64_t)bodyID.GetIndexAndSequenceNumber())
            .with("fx", f.x)
            .with("fy", f.y)
            .with("fz", f.z);

        commandQueue.push_back({CmdType::Force, bodyID, JPH::Vec3(f.x, f.y, f.z)});
    }

    void Rigidbody::addTorque(const Math::Vector3<float>& t)
    {
        if (bodyID.IsInvalid())
        {
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::WARNING, "addTorque ignored: invalid BodyID"
            );

            return;
        }

        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::INFO, "addTorque queued")
            .with("bodyIndexAndSeq", (uint64_t)bodyID.GetIndexAndSequenceNumber())
            .with("tx", t.x)
            .with("ty", t.y)
            .with("tz", t.z);

        commandQueue.push_back({CmdType::Torque, bodyID, JPH::Vec3(t.x, t.y, t.z)});
    }

    void Rigidbody::applyImpulse(const Math::Vector3<float>& impulse)
    {
        if (bodyID.IsInvalid())
        {
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::WARNING, "applyImpulse ignored: invalid BodyID"
            );

            return;
        }

        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::INFO, "applyImpulse queued")
            .with("bodyIndexAndSeq", (uint64_t)bodyID.GetIndexAndSequenceNumber())
            .with("ix", impulse.x)
            .with("iy", impulse.y)
            .with("iz", impulse.z);

        commandQueue.push_back(
            {CmdType::Impulse, bodyID, JPH::Vec3(impulse.x, impulse.y, impulse.z)}
        );
    }
} // namespace nb::Physics