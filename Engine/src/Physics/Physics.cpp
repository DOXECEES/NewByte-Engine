#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include "Error/ErrorManager.hpp"
#include "Physics.hpp"

#include <thread>
#include <vector>

namespace nb::Physics
{
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
        auto& t  = scene.getComponent<TransformComponent>(entityId);
        auto& c  = scene.getComponent<Collider>(entityId);

        float sx = c.halfSize.x * t.scale.x;
        float sy = c.halfSize.y * t.scale.y;
        float sz = c.halfSize.z * t.scale.z;

        if (sx <= 0.0f || sy <= 0.0f || sz <= 0.0f)
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::FATAL, "Invalid collider size (<= 0)")
                .with("entity", (uint64_t)entityId)
                .with("sx", sx)
                .with("sy", sy)
                .with("sz", sz);

            return;
        }

        JPH::BoxShapeSettings shapeSettings(JPH::Vec3(sx, sy, sz));

        auto shapeResult = shapeSettings.Create();
        if (shapeResult.HasError())
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::FATAL, "Jolt shape creation failed")
                .with("entity", (uint64_t)entityId)
                .with("error", shapeResult.GetError().c_str());

            return;
        }

        JPH::ShapeRefC shape = shapeResult.Get();

        JPH::EMotionType motionType =
            rb.isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;

        JPH::ObjectLayer layer = rb.isStatic ? Layers::NON_MOVING : Layers::MOVING;

        JPH::BodyCreationSettings settings(
            shape, JPH::RVec3(t.position.x, t.position.y, t.position.z),
            JPH::Quat(t.rotation.x, t.rotation.y, t.rotation.z, t.rotation.w), motionType, layer
        );

        if (!rb.isStatic)
        {
            settings.mOverrideMassProperties =
                JPH::EOverrideMassProperties::CalculateMassAndInertia;
        }

        settings.mFriction    = rb.friction;
        settings.mRestitution = rb.restitution;

        JPH::BodyInterface& bodyInterface = physicsSystem->GetBodyInterface();

        JPH::Body* body = bodyInterface.CreateBody(settings);
        if (!body)
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::FATAL, "CreateBody returned nullptr")
                .with("entity", (uint64_t)entityId);

            return;
        }

        rb.bodyID = body->GetID();

        bodyInterface.AddBody(rb.bodyID, JPH::EActivation::Activate);

        if (!bodyInterface.IsAdded(rb.bodyID))
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::FATAL, "Body NOT added after AddBody")
                .with("entity", (uint64_t)entityId)
                .with("bodyIndexAndSeq", (uint64_t)rb.bodyID.GetIndexAndSequenceNumber());
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

        auto view = scene.getEntitiesWith<Rigidbody, TransformComponent, Collider>();

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
                    bodyInterface.SetPositionAndRotation(
                        rb.bodyID, JPH::RVec3(tc.position.x, tc.position.y, tc.position.z),
                        JPH::Quat(-tc.rotation.x, -tc.rotation.y, -tc.rotation.z, tc.rotation.w),
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

        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::INFO, "Physics step finished")
            .with("dt", dt);

        for (auto entity : view)
        {
            auto& rb = scene.getComponent<Rigidbody>(entity.id);
            auto& tc = scene.getComponent<TransformComponent>(entity.id);

            if (rb.isStatic || rb.bodyID.IsInvalid())
            {
                continue;
            }

            if (!bodyInterface.IsAdded(rb.bodyID))
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::FATAL, "Body exists in ECS but not added in Jolt")
                    .with("entity", (uint64_t)entity.id)
                    .with("bodyIndexAndSeq", (uint64_t)rb.bodyID.GetIndexAndSequenceNumber());

                continue;
            }

            if (!bodyInterface.IsActive(rb.bodyID))
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::INFO, "Body is sleeping (inactive)")
                    .with("entity", (uint64_t)entity.id)
                    .with("bodyIndexAndSeq", (uint64_t)rb.bodyID.GetIndexAndSequenceNumber());

                continue;
            }

            JPH::RVec3 pos;
            JPH::Quat  rot;

            bodyInterface.GetPositionAndRotation(rb.bodyID, pos, rot);

            JPH::Vec3 vel = bodyInterface.GetLinearVelocity(rb.bodyID);
            JPH::Vec3 ang = bodyInterface.GetAngularVelocity(rb.bodyID);

            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::INFO, "Body state after sim")
                .with("entity", (uint64_t)entity.id)
                .with("bodyIndexAndSeq", (uint64_t)rb.bodyID.GetIndexAndSequenceNumber())
                .with("px", (float)pos.GetX())
                .with("py", (float)pos.GetY())
                .with("pz", (float)pos.GetZ())
                .with("vx", vel.GetX())
                .with("vy", vel.GetY())
                .with("vz", vel.GetZ())
                .with("wx", ang.GetX())
                .with("wy", ang.GetY())
                .with("wz", ang.GetZ());

            tc.position = {(float)pos.GetX(), (float)pos.GetY(), (float)pos.GetZ()};

            tc.rotation = {
                -(float)rot.GetX(), -(float)rot.GetY(), -(float)rot.GetZ(), (float)rot.GetW()
            };

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