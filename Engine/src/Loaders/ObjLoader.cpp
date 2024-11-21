#include "ObjLoader.hpp"

Ref<nb::Renderer::Mesh> nb::Loaders::ObjLoader::loadMesh(const std::filesystem::path& path) noexcept
{
    std::ifstream file;
    file.open(path);
    if(!file.is_open())
    {
        return nullptr;
    }

    std::vector<Renderer::Vertex> vert;
    std::vector<nb::Math::Vector3<float>> verticies;
    std::vector<nb::Math::Vector3<float>> textureCoords;
    std::vector<nb::Math::Vector3<float>> normals;
    std::vector<uint32_t> indicies;

    std::unordered_map<Renderer::Vertex, uint32_t> uniqueVertices;

    while(!file.eof())
    {
        std::string str;
        std::getline(file, str);

        switch (str[0])
        {
        case '#':
            //std::cout << "Comment : " + str << std::endl;
            break;
        case 'v':
            switch (str[1])
            {
            case ' ':
            {
                //std::cout << "vertex : " + str << std::endl;
                std::istringstream ss(str);
                std::string s;
             
                while(ss >> s)
                {
                    if(!std::isalpha(s[0]))
                    {
                        auto x = std::stof(s);

                        ss >> s;
                        auto y = std::stof(s);

                        ss >> s;
                        auto z = std::stof(s);
                        
                        verticies.push_back({x,y,z});
                    }
                }

                break;
            }
            case 't':
            {
                //std::cout << "texture : " + str << std::endl;
                std::istringstream ss(str);
                std::string s;
             
                while(ss >> s)
                {
                    if(!std::isalpha(s[0]))
                    {
                        auto x = std::stof(s);

                        ss >> s;
                        auto y = std::stof(s);

                        ss >> s;
                        auto z = std::stof(s);
                        textureCoords.push_back({x,y,z});
                    }
                }
                break;
            }
            case 'n':
            {
                //std::cout << "normal : " + str << std::endl;
                std::istringstream ss(str);
                std::string s;
             
                while(ss >> s)
                {
                    if(!std::isalpha(s[0]))
                    {
                        auto x = std::stof(s);

                        ss >> s;
                        auto y = std::stof(s);

                        ss >> s;
                        auto z = std::stof(s);
                        normals.push_back({x,y,z});
                    }
                }
                break;
            }
            default:
                break;
            }
            break;
        
        case 'f':
        {
            //std::cout << "Face : " + str << std::endl;
            std::istringstream ss(str);
            std::string s;
            
            // vert index
            while(ss >> s)
            {
                if(!std::isalpha(s[0]))
                {
                    size_t pos1 = s.find('/');
                    size_t pos2 = s.find('/', pos1 + 1);

                    uint32_t vertIdx = std::stoi(s.substr(0, pos1)) - 1;
                    //texture index
                    uint32_t textureIdx = std::stoi(s.substr(pos1 + 1, pos2 - pos1 - 1)) - 1;
                    //normal index
                    uint32_t normIdx = std::stoi(s.substr(pos2 + 1)) - 1;

                    

                    Renderer::Vertex vertex = {
                        verticies[vertIdx],
                        //texCoords.size() > texIdx ? texCoords[texIdx] : glm::vec2(0.0f, 0.0f),
                        normals[normIdx]
                    };

                    if (uniqueVertices.count(vertex) == 0) {
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

    return createRef<Renderer::Mesh>(std::move(vert), std::move(indicies));
}