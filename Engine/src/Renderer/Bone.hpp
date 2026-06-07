#pragma once
#include "Math/Matrix/Matrix.hpp"
#include "Math/Matrix/Transformation.hpp"
#include "Math/Vector3.hpp"
#include <assimp/anim.h>
#include <string>
#include <vector>

namespace nb::Renderer
{
    struct KeyPosition
    {
        Math::Vector3<float> position;
        float                timeStamp;
    };

    struct KeyRotation
    {
        aiQuaternion rotation;
        float        timeStamp;
    };

    struct KeyScale
    {
        Math::Vector3<float> scale;
        float                timeStamp;
    };

    class Bone
    {
    public:
        Bone(
            const std::string& name,
            int                ID,
            const aiNodeAnim*  channel
        )
            : m_Name(name)
            , m_ID(ID)
            , m_LocalTransform(Math::Mat4<float>::identity())
        {
            m_NumPositions = channel->mNumPositionKeys;
            for (unsigned int i = 0; i < m_NumPositions; ++i)
            {
                aiVector3D p = channel->mPositionKeys[i].mValue;
                float      t = (float)channel->mPositionKeys[i].mTime;
                m_Positions.push_back({{p.x, p.y, p.z}, t});
            }

            m_NumRotations = channel->mNumRotationKeys;
            for (unsigned int i = 0; i < m_NumRotations; ++i)
            {
                aiQuaternion r = channel->mRotationKeys[i].mValue;
                float        t = (float)channel->mRotationKeys[i].mTime;
                m_Rotations.push_back({r, t});
            }

            m_NumScalings = channel->mNumScalingKeys;
            for (unsigned int i = 0; i < m_NumScalings; ++i)
            {
                aiVector3D s = channel->mScalingKeys[i].mValue;
                float      t = (float)channel->mScalingKeys[i].mTime;
                m_Scales.push_back({{s.x, s.y, s.z}, t});
            }
        }

        void Update(float animationTime)
        {
            Math::Mat4<float> translation = InterpolatePosition(animationTime);
            Math::Mat4<float> rotation    = InterpolateRotation(animationTime);
            Math::Mat4<float> scale       = InterpolateScaling(animationTime);
            m_LocalTransform              = translation * rotation * scale;
        }

        const Math::Mat4<float>& GetLocalTransform() const
        {
            return m_LocalTransform;
        }
        const std::string& GetBoneName() const
        {
            return m_Name;
        }
        int GetBoneID() const
        {
            return m_ID;
        }

    private:
        // ИСПРАВЛЕНО: возвращаем последний валидный индекс, а не 0
        int GetPositionIndex(float animationTime) const
        {
            for (int i = 0; i < m_NumPositions - 1; ++i)
            {
                if (animationTime < m_Positions[i + 1].timeStamp)
                {
                    return i;
                }
            }
            return m_NumPositions - 1; // было return 0
        }
        int GetRotationIndex(float animationTime) const
        {
            for (int i = 0; i < m_NumRotations - 1; ++i)
            {
                if (animationTime < m_Rotations[i + 1].timeStamp)
                {
                    return i;
                }
            }
            return m_NumRotations - 1; // было return 0
        }
        int GetScaleIndex(float animationTime) const
        {
            for (int i = 0; i < m_NumScalings - 1; ++i)
            {
                if (animationTime < m_Scales[i + 1].timeStamp)
                {
                    return i;
                }
            }
            return m_NumScalings - 1; // было return 0
        }

        float GetScaleFactor(
            float last,
            float next,
            float animationTime
        ) const
        {
            float mid  = animationTime - last;
            float diff = next - last;
            if (diff < 1e-6f)
            {
                return 0.0f; // защита от деления на ноль
            }
            return mid / diff;
        }

        Math::Mat4<float> InterpolatePosition(float animationTime)
        {
            if (m_NumPositions == 1)
            {
                return Math::translate(Math::Mat4<float>::identity(), m_Positions[0].position);
            }

            int p0 = GetPositionIndex(animationTime);
            // Если мы на последнем кадре — возвращаем его без интерполяции
            if (p0 >= m_NumPositions - 1)
            {
                return Math::translate(Math::Mat4<float>::identity(), m_Positions[p0].position);
            }

            int   p1 = p0 + 1;
            float t =
                GetScaleFactor(m_Positions[p0].timeStamp, m_Positions[p1].timeStamp, animationTime);
            Math::Vector3<float> pos = m_Positions[p0].position +
                                       (m_Positions[p1].position - m_Positions[p0].position) * t;
            return Math::translate(Math::Mat4<float>::identity(), pos);
        }

        Math::Mat4<float> InterpolateRotation(float animationTime)
        {
            if (m_NumRotations == 1)
            {
                return ConvertAiMatrix3ToNb(m_Rotations[0].rotation.GetMatrix());
            }

            int r0 = GetRotationIndex(animationTime);
            if (r0 >= m_NumRotations - 1)
            {
                return ConvertAiMatrix3ToNb(m_Rotations[r0].rotation.GetMatrix());
            }

            int   r1 = r0 + 1;
            float t =
                GetScaleFactor(m_Rotations[r0].timeStamp, m_Rotations[r1].timeStamp, animationTime);

            aiQuaternion finalRot;
            aiQuaternion::Interpolate(
                finalRot, m_Rotations[r0].rotation, m_Rotations[r1].rotation, t
            );
            finalRot.Normalize();
            return ConvertAiMatrix3ToNb(finalRot.GetMatrix());
        }

        Math::Mat4<float> InterpolateScaling(float animationTime)
        {
            if (m_NumScalings == 1)
            {
                return Math::scale(Math::Mat4<float>::identity(), m_Scales[0].scale);
            }

            int s0 = GetScaleIndex(animationTime);
            if (s0 >= m_NumScalings - 1)
            {
                return Math::scale(Math::Mat4<float>::identity(), m_Scales[s0].scale);
            }

            int   s1 = s0 + 1;
            float t = GetScaleFactor(m_Scales[s0].timeStamp, m_Scales[s1].timeStamp, animationTime);
            Math::Vector3<float> s =
                m_Scales[s0].scale + (m_Scales[s1].scale - m_Scales[s0].scale) * t;
            return Math::scale(Math::Mat4<float>::identity(), s);
        }

        // ИСПРАВЛЕНО: транспонирование как в Animation.hpp (row-major Assimp → column-major Mat4)
        // ПРАВИЛЬНАЯ версия (оригинал, транспонирует как ConvertMatrixToNbFormat):
        Math::Mat4<float> ConvertAiMatrix3ToNb(const aiMatrix3x3& from)
        {
            Math::Mat4<float> to = Math::Mat4<float>::identity();
            to[0][0]             = from.a1;
            to[1][0]             = from.a2;
            to[2][0]             = from.a3;
            to[0][1]             = from.b1;
            to[1][1]             = from.b2;
            to[2][1]             = from.b3;
            to[0][2]             = from.c1;
            to[1][2]             = from.c2;
            to[2][2]             = from.c3;
            return to;
        }

        std::vector<KeyPosition> m_Positions;
        std::vector<KeyRotation> m_Rotations;
        std::vector<KeyScale>    m_Scales;
        int                      m_NumPositions = 0;
        int                      m_NumRotations = 0;
        int                      m_NumScalings  = 0;

        Math::Mat4<float> m_LocalTransform;
        std::string       m_Name;
        int               m_ID;
    };
} // namespace nb::Renderer