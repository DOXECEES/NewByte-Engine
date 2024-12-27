#include "ObjLoader.hpp"

namespace nb
{
    namespace Loaders
    {
        Ref<Renderer::Mesh> ObjLoader::loadMesh(const std::filesystem::path &path) noexcept
        {
            std::ifstream file;
            file.open(path);
            if (!file.is_open())
            {
                return nullptr;
            }

            std::vector<Renderer::Vertex> vert;
            std::vector<nb::Math::Vector3<float>> verticies;
            std::vector<nb::Math::Vector2<float>> textureCoords;
            std::vector<nb::Math::Vector3<float>> normals;
            std::vector<uint32_t> indicies;

            std::unordered_map<Renderer::Vertex, uint32_t> uniqueVertices;

            while (!file.eof())
            {
                std::string str;
                std::getline(file, str);

                switch (str[0])
                {
                case '#':
                    // comment section
                    break;
                case 'v':
                {
                    std::istringstream ss(str);
                    std::string s;

                    switch (str[1])
                    {
                    case ' ':
                    {
                        verticies.emplace_back(parseVertex(str.substr(2)));
                        break;
                    }
                    case 't':
                    {
                        textureCoords.emplace_back(parseTextureCoords(str.substr(2)));
                        break;
                    }
                    case 'n':
                    {
                        normals.emplace_back(parseNormals(str.substr(2)));
                        break;
                    }
                    default:
                        break;
                    }
                    break;
                }
                case 'f':
                {
                    std::istringstream ss(str);
                    std::string s;

                    // vert index
                    while (ss >> s)
                    {
                        if (!std::isalpha(s[0]))
                        {
                            /// TODO: reimplement + add not only triangles + polygons + oth

                            size_t pos1 = s.find('/');
                            size_t pos2 = s.find('/', pos1 + 1);

                            uint32_t vertId = ~0L;
                            uint32_t texId = ~0L;
                            uint32_t normId = ~0L;

                            if (pos1 == std::string::npos)
                            {
                                // just vert
                                vertId = std::stoi(s) - 1;
                            }
                            else if (pos2 == std::string::npos)
                            {
                                // vert + texture
                                vertId = std::stoi(s.substr(0, pos1)) - 1;
                                texId = std::stoi(s.substr(pos1 + 1, pos2)) - 1;
                            }
                            else if (int diff = pos2 - pos1; diff == 1)
                            {
                                // vert + normales
                                vertId = std::stoi(s.substr(0, pos1)) - 1;
                                normId = std::stoi(s.substr(pos2 + 1)) - 1;
                            }
                            else
                            {
                                vertId = std::stoi(s.substr(0, pos1)) - 1;
                                texId = std::stoi(s.substr(pos1 + 1, pos2)) - 1;
                                normId = std::stoi(s.substr(pos2 + 1)) - 1;
                            }

                            Renderer::Vertex vertex = {
                                verticies[vertId],
                                normId != ~0L ? normals[normId] : Math::Vector3<float>(0.0f, 0.0f, 0.0f),
                                {0.0f, 0.0f, 0.0f},
                                texId != ~0L ? textureCoords[texId] : Math::Vector2<float>(0.0f, 0.0f)};

                            if (uniqueVertices.count(vertex) == 0)
                            {
                                uniqueVertices[vertex] = static_cast<uint32_t>(vert.size());
                                vert.push_back(vertex);
                            }

                            indicies.push_back(uniqueVertices[vertex]);
                        }
                    }
                    break;
                }
                default:
                    break;
                }
            }

            std::filesystem::path pathToMtl = path;
            pathToMtl.replace_extension(".mtl");

            std::vector<Renderer::Material> materials = loadMaterial(pathToMtl.string());

            return createRef<Renderer::Mesh>(std::move(vert), std::move(indicies), std::move(materials));
        }

        std::vector<Renderer::Material> ObjLoader::loadMaterial(const std::string &path) noexcept
        {
            std::ifstream mtlFile;
            mtlFile.open(path);
            if (!mtlFile.is_open())
            {
                assert("Failed to open mtl file");
                return {};
            }

            std::vector<Renderer::Material> materials;
            std::stringstream iss;
            iss << mtlFile.rdbuf();
            
            std::string token;

            while (iss >> token)
            {
                if(token == "newmtl")
                {
                    materials.emplace_back(Renderer::Material());
                }
                else if(token == "Ka")
                {
                    float r, g, b;
                    iss >> r >> g >> b;
                    materials.back().ambient = Math::Vector3<float>(r, g, b);
                }
                else if(token == "Kd")
                {
                    float r, g, b;
                    iss >> r >> g >> b;
                    materials.back().diffuse = Math::Vector3<float>(r, g, b);
                }
                else if(token == "Ks")
                {
                    float r, g, b;
                    iss >> r >> g >> b;
                    materials.back().specular = Math::Vector3<float>(r, g, b);
                }
                else if(token == "Ns")
                {
                    float n;
                    iss >> n;
                    materials.back().shininess = n;
                }
                else if(token == "d")
                {
                    float d;
                    iss >> d;
                    materials.back().dissolve = d;
                }
                else if(token == "illum")
                {
                    uint8_t illum;
                    iss >> illum;
                    materials.back().illuminationModel = illum;
                }
            }

            return materials;
        }

        Math::Vector3<float> ObjLoader::parseVertex(std::string_view str) noexcept
        {
            if (str.empty())
            {
                return Math::Vector3<float>();
            }

            std::istringstream iss(str.data());
            std::string value;

            iss >> value;
            float x = std::stof(value);
            iss >> value;
            float y = std::stof(value);
            iss >> value;
            float z = std::stof(value);

            return {x, y, z};
        }

        Math::Vector3<float> ObjLoader::parseNormals(std::string_view str) noexcept
        {
            return parseVertex(str);
        }

        Math::Vector2<float> ObjLoader::parseTextureCoords(std::string_view str) noexcept
        {
            if (str.empty())
            {
                return Math::Vector2<float>();
            }

            std::istringstream iss(str.data());
            std::string value;

            iss >> value;
            float x = std::stof(value);
            iss >> value;
            float y = std::stof(value);

            return {x, y};
        }
    };
};
