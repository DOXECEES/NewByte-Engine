#include "IArchive.hpp"

namespace nb::Serialize
{
    IArchive::IArchive(const std::filesystem::path& pathToFile) noexcept 
        : path(pathToFile)
    {
    }
    void IArchive::setMode(Mode newMode) noexcept
    {
        mode = newMode;
    }

    bool IArchive::isRead() const noexcept
    {
        return mode == Mode::READ;
    }

    bool IArchive::isWrite() const noexcept
    {
        return mode == Mode::WRITE;
    }

}

