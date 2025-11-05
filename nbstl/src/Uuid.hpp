#ifndef NBSTL_SRC_UUID_HPP
#define NBSTL_SRC_UUID_HPP

#include <array>
#include <cstdint>
#include <cstring>
#include <random>
#include <string>
#include <functional>

namespace nbstl
{
	class Uuid
	{
	public:
		std::array<uint8_t, 16> bytes{};

		static Uuid generate() noexcept
		{
			Uuid uuid;
			std::random_device rd;
			std::mt19937_64 gen(rd());
			std::uniform_int_distribution<uint64_t> dist;

			uint64_t high = dist(gen);
			uint64_t low = dist(gen);
			std::memcpy(uuid.bytes.data(), &high, 8);
			std::memcpy(uuid.bytes.data() + 8, &low, 8);

			// version (4)
			uuid.bytes[6] = (uuid.bytes[6] & 0x0F) | 0x40;
			// variant (RFC 4122)
			uuid.bytes[8] = (uuid.bytes[8] & 0x3F) | 0x80;

			return uuid;
		}

		std::string toString() const noexcept
		{
			char str[37];
			std::snprintf(str, sizeof(str),
				"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
				bytes[0], bytes[1], bytes[2], bytes[3],
				bytes[4], bytes[5],
				bytes[6], bytes[7],
				bytes[8], bytes[9],
				bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]);
			return std::string(str);
		}

		bool operator==(const Uuid& other) const noexcept
		{
			return std::memcmp(bytes.data(), other.bytes.data(), 16) == 0;
		}

		bool operator!=(const Uuid& other) const noexcept
		{
			return !(*this == other);
		}
	};
}

namespace std
{
	template <>
	struct hash<nbstl::Uuid>
	{
		size_t operator()(const nbstl::Uuid& uuid) const noexcept
		{
			const uint64_t* data = reinterpret_cast<const uint64_t*>(uuid.bytes.data());
			return std::hash<uint64_t>{}(data[0]) ^ (std::hash<uint64_t>{}(data[1]) << 1);
		}
	};
}

#endif
