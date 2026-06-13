#ifndef SDK_SCENE_SNAPPING_HPP
#define SDK_SCENE_SNAPPING_HPP

#include <Math/Vector3.hpp>
#include <Error/ErrorManager.hpp>
#include <Math/Quaternion.hpp>

namespace sdk
{
    struct GridSnapping
    {
        float translationFactor = 1.0f;
        float rotationFactor = 15.0f;
        float scaleFactor = 1.0f;
        
        bool translationEnabled = false;
        bool rotationEnabled = false;
        bool scaleEnabled = false;
    };


    enum class SnappingType
    {
        None,
        Grid
    };

    class Snapper
    {
    public:
    
        void setGridSnapping(const GridSnapping& snapping) noexcept { gridSnapping = snapping; }
        nb::Math::Vector3<float> calculateTranslation(const nb::Math::Vector3<float>& translation) const noexcept
        {
            switch(snappingType)
            {
                case SnappingType::Grid:
                {
                    if(!gridSnapping.translationEnabled)
                    {
                        return translation;
                    }   

                    const float invGridSnapValue = 1.0f / gridSnapping.translationFactor;

                    return {
                        std::round(translation.x * invGridSnapValue) * gridSnapping.translationFactor,
                        std::round(translation.y * invGridSnapValue) * gridSnapping.translationFactor,
                        std::round(translation.z * invGridSnapValue) * gridSnapping.translationFactor
                    };
                }
                default:
                {
                    nb::Error::ErrorManager::instance()
                        .report(nb::Error::Type::WARNING, "Unknown snapping type");
                    return translation;
                }
            }  
        }

        nb::Math::Quaternion<float> calculateRotation(const nb::Math::Quaternion<float>& rotation) const noexcept
        {
            switch(snappingType)
            {
                case SnappingType::Grid:
                {
                    if(!gridSnapping.rotationEnabled)
                    {
                        return rotation;
                    }   

                    auto euler = rotation.toEulerXYZ();
                    const float radStep = nb::Math::toRadians(gridSnapping.rotationFactor);
                    const float invRadStep = 1.0f / radStep;

                    return nb::Math::Quaternion<float>::eulerToQuaternionXYZ(
                        std::round(euler.x * invRadStep) * radStep,
                        std::round(euler.y * invRadStep) * radStep,
                        std::round(euler.z * invRadStep) * radStep
                    );

                }
                default:
                {
                    nb::Error::ErrorManager::instance()
                        .report(nb::Error::Type::WARNING, "Unknown snapping type");
                    return rotation;
                }
            }  
        }

        nb::Math::Vector3<float> calculateScale(const nb::Math::Vector3<float>& scale) const noexcept
        {
            switch(snappingType)
            {
                case SnappingType::Grid:
                {
                    if(!gridSnapping.scaleEnabled)
                    {
                        return scale;
                    }   

                    const float invScaleSnap = 1.0f / gridSnapping.scaleFactor;
            
                    return {
                        (std::max)(0.01f, std::round(scale.x * invScaleSnap) * gridSnapping.scaleFactor),
                        (std::max)(0.01f, std::round(scale.y * invScaleSnap) * gridSnapping.scaleFactor),
                        (std::max)(0.01f, std::round(scale.z * invScaleSnap) * gridSnapping.scaleFactor)
                    };

                }
                default:
                {
                    nb::Error::ErrorManager::instance()
                        .report(nb::Error::Type::WARNING, "Unknown snapping type");
                    return scale;
                }
            }  
        }

        void setSnappingType(SnappingType type) noexcept { snappingType = type; }

        SnappingType getSnappingType() const noexcept { return snappingType; }
        const GridSnapping& getGridSnapping() const noexcept { return gridSnapping; }
        
    private:
    
        SnappingType snappingType = SnappingType::Grid;
        GridSnapping gridSnapping = {};
    
    };    
    
};


#endif
