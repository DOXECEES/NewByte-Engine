#ifndef NBSTL_SRC_UUID_HPP
#define NBSTL_SRC_UUID_HPP

#include <cstdint>
#include <array>
#include <random>
#include <climits>
#include <algorithm>
#include <functional>

namespace nbstl
{
	class Uuid
	{
	public:

		static Uuid generate() noexcept
		{
			std::array<uint16_t, 8> bytes;
			
			using random_range =  std::independent_bits_engine<
    											std::default_random_engine, 16, uint16_t>;

			random_range rr; 
			std::generate(bytes.begin(), bytes.end(),std::ref(rr));

			Uuid uuid;

			uuid = std::bit_cast<Uuid>(bytes);
			
			uuid.timeHiAndVersion = ((uuid.timeHiAndVersion & 0x0FFF) | 0x4000);
			
			uint8_t hi = (getHiBytes(uuid.clockSequense) >> 8) & 0xFF;
			hi = (hi & 0x3F) | 0x80;
			uuid.clockSequense = (hi << 8) | getLowBytes(uuid.clockSequense);

			return uuid;
		}

		static std::string toString(Uuid uuid) noexcept
		{
			std::stringstream stream;
			stream << std::hex << std::setw(8) << std::setfill('0') << uuid.timeLow << "-";
			stream << std::hex << std::setw(4) << std::setfill('0') << uuid.timeMid << "-";
			stream << std::hex << std::setw(4) << std::setfill('0') << uuid.timeHiAndVersion << "-";
			stream << std::hex << std::setw(4) << std::setfill('0') << uuid.clockSequense << "-";
			stream << std::hex << std::setw(12) << std::setfill('0') << (((uint64_t)uuid.nodeHigh << 32) | (uint64_t)uuid.nodeLow);

			return stream.str();
		}


		static uint8_t getHiBytes(const uint16_t value) noexcept
		{
			return (value >> 8) & 0xFF;
		}

		static uint8_t getLowBytes(const uint16_t value) noexcept
		{
			return value & 0x00FF;
		}

	private:

		uint32_t timeLow;
		uint16_t timeMid;
		uint16_t timeHiAndVersion;
		uint16_t clockSequense;
		uint16_t nodeHigh;
		uint32_t nodeLow;
	};

};

#endif