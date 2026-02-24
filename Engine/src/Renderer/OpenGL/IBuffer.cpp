// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "IBuffer.hpp"

namespace nb::OpenGl
{
	GLuint IBuffer::getId() const noexcept
	{
		return buffer;
	}
};


