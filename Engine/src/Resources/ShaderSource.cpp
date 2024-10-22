#include "ShaderSource.hpp"

nb::Resource::ShaderSource::ShaderSource(const std::string &str) noexcept
    :source(std::move(str))
{
}