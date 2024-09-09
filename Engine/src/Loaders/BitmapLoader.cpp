#include "BitmapLoader.hpp"

#include "../Debug.hpp"

nb::Loaders::BitmapLoader::BitmapLoader(std::string_view path)
    : file(path.data())
{
    BMPHeader header;
    readToStruct(file, header);
    Debug::debug(header.header);
    Debug::debug(header.fileSize);
    Debug::debug(header.reserved);
    Debug::debug(header.dataOffset);

    BMPInfoHeader info;
    readToStruct(file, info);
    Debug::debug(info.size);
}