#ifndef SRC_LOADERS_OBJLOADER_HPP
#define SRC_LOADERS_OBJLOADER_HPP

#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>

#include "../Math/Vector3.hpp"

#include "../Renderer/Mesh.hpp"
#include "../Core.hpp"

namespace nb
{
    namespace Loaders
    {
        class ObjLoader
        {
        public:
            // ObjLoader() = default;
            // ~ObjLoader() = default;
            // ObjLoader(const ObjLoader&) = delete;
            // ObjLoader& operator=(const ObjLoader&) = delete;
            // ObjLoader(ObjLoader&&) = delete;
            // ObjLoader& operator=(ObjLoader&&) = delete;

            static Ref<Renderer::Mesh> loadMesh(const std::filesystem::path &path) noexcept;

        private:
        };
    };
};

namespace std
{
    template <>
    struct hash<nb::Renderer::Vertex>
    {
        size_t operator()(const nb::Renderer::Vertex &vertex) const
        {
            return ((hash<float>()(vertex.position.x) ^
                     (hash<float>()(vertex.position.y) << 1)) >>
                    1) ^
                   (hash<float>()(vertex.position.z) << 1);
        }
    };
}

#endif
