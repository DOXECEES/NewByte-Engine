#ifndef SRC_RENDERER_COLOR_HPP
#define SRC_RENDERER_COLOR_HPP

#include <NbCore.hpp>
#include <Alghorithm.hpp>

#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"

#include <cstdint>
#undef RGB

namespace nb
{
	namespace Renderer
	{
		struct HSV
		{
			constexpr HSV() noexcept = default;

			constexpr HSV(float hue, float saturation, float value) noexcept
				: hue(hue), saturation(saturation), value(value)
			{
				NB_ASSERT(hue >= 0.0f && hue <= 360.0f, "HUE MUST BE BETWEEN 0 AND 360 DEGREES");
				NB_ASSERT(saturation >= 0.0f && saturation <= 1.0f, "SATURATION MUST BE BETWEEN 0 AND 1");
				NB_ASSERT(value >= 0.0f && value <= 1.0f, "VALUE MUST BE BETWEEN 0 AND 1");
			}

			float hue			= 0.0f;
			float saturation	= 0.0f;
			float value			= 0.0f;

			NB_NODISCARD constexpr Math::Vector3<float> toVec3() noexcept
			{
				return Math::Vector3<float>(hue, saturation, value);
			}

			constexpr bool operator==(const HSV& other) const noexcept
			{
				return hue == other.hue &&
					saturation == other.saturation &&
					value == other.value;
			}

			constexpr bool operator!=(const HSV& other) const noexcept
			{
				return !(*this == other);
			}
		
		};

		struct RGB
		{
			constexpr RGB(uint8_t r, uint8_t g, uint8_t b) noexcept
				: r(r), g(g), b(b)
			{
				NB_ASSERT(r >= 0 && r <= 255, "RED MUST BE BETWEEN 0 AND 255");
				NB_ASSERT(g >= 0 && g <= 255, "GREEN MUST BE BETWEEN 0 AND 255");
				NB_ASSERT(b >= 0 && b <= 255, "BLUE MUST BE BETWEEN 0 AND 255");
			}

			uint8_t r = 0;
			uint8_t g = 0;
			uint8_t b = 0;

			NB_NODISCARD constexpr Math::Vector3<float> toVec3() noexcept
			{
				return Math::Vector3<float>(
					static_cast<float>(r) / 255.0f,
					static_cast<float>(g) / 255.0f,
					static_cast<float>(b) / 255.0f
				);
			}

			NB_NODISCARD constexpr Math::Vector3<int> toVec3Linear() noexcept
			{
				return Math::Vector3<int>(r, g, b);
			}

			constexpr bool operator==(const RGB& other) const noexcept
			{
				return r == other.r &&
					g == other.g &&
					b == other.b;
			}

			constexpr bool operator!=(const RGB& other) const noexcept
			{
				return !(*this == other);
			}
		};

		struct RGBA
		{
			constexpr RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept
				: r(r), g(g), b(b), alpha(a)
			{
				NB_ASSERT(r >= 0 && r <= 255, "RED MUST BE BETWEEN 0 AND 255");
				NB_ASSERT(g >= 0 && g <= 255, "GREEN MUST BE BETWEEN 0 AND 255");
				NB_ASSERT(b >= 0 && b <= 255, "BLUE MUST BE BETWEEN 0 AND 255");
				NB_ASSERT(alpha >= 0 && alpha <= 255, "ALPHA MUST BE BETWEEN 0 AND 255");
			}

			NB_NODISCARD constexpr Math::Vector4<float> toVec4() noexcept
			{
				return Math::Vector4<float>(
					static_cast<float>(r) / 255.0f,
					static_cast<float>(g) / 255.0f,
					static_cast<float>(b) / 255.0f,
					static_cast<float>(alpha) / 255.0f
				);
			}

			NB_NODISCARD constexpr Math::Vector4<int> toVec4Linear() noexcept
			{
				return Math::Vector4<int>(r, g, b, alpha);
			}

			uint8_t r		= 0;
			uint8_t g		= 0;
			uint8_t b		= 0;
			uint8_t alpha	= 255;

			constexpr bool operator==(const RGBA& other) const noexcept
			{
				return r == other.r &&
					g == other.g &&
					b == other.b &&
					alpha == other.alpha;
			}

			constexpr bool operator!=(const RGBA& other) const noexcept
			{
				return !(*this == other);
			}
		};

		class Color
		{
			static constexpr int		HUE_SECTORS_COUNT			= 6;
			static constexpr float		HUE_SECTOR_SIZE_IN_DEGREES	= 60.0f;
			static constexpr uint8_t	MAX_LINEAR_RGBA_VALUE		= 255;

		public:

			NB_NODISCARD constexpr Math::Vector3<float> asVec3() const noexcept
			{
				return {
					static_cast<float>(rgba.r) / static_cast<float>(MAX_LINEAR_RGBA_VALUE),
					static_cast<float>(rgba.g) / static_cast<float>(MAX_LINEAR_RGBA_VALUE),
					static_cast<float>(rgba.b) / static_cast<float>(MAX_LINEAR_RGBA_VALUE)
				};
			}

			NB_NODISCARD constexpr Math::Vector4<float> asVec4() const noexcept
			{
				return {
					static_cast<float>(rgba.r) / static_cast<float>(MAX_LINEAR_RGBA_VALUE),
					static_cast<float>(rgba.g) / static_cast<float>(MAX_LINEAR_RGBA_VALUE),
					static_cast<float>(rgba.b) / static_cast<float>(MAX_LINEAR_RGBA_VALUE),
					static_cast<float>(rgba.alpha) / static_cast<float>(MAX_LINEAR_RGBA_VALUE)
				};
			}
		
			NB_NODISCARD constexpr RGB toRgb() const noexcept
			{
				return RGB(rgba.r, rgba.g, rgba.b);
			}

			NB_NODISCARD constexpr RGBA toRgba() const noexcept
			{
				return rgba;
			}

			NB_NODISCARD constexpr HSV toHsv() const noexcept
			{
				float hue			= 60.0f;
				float saturation	= 0.0f;
				float value			= 0.0f;

				float r = rgba.r;
				float g = rgba.g;
				float b = rgba.b;

				float maxChroma = nbstl::maxInRange(r, g, b);
				float minChroma = nbstl::minInRange(r, g, b);

				float deltaChroma = maxChroma - minChroma;

				if (deltaChroma != 0.0f)
				{
					if (maxChroma == r)
					{
						hue *= static_cast<float>(std::fmod(((g - b) / deltaChroma), HUE_SECTORS_COUNT));
					}
					else if (maxChroma == g)
					{
						hue *= (((b - r) / deltaChroma) + 2);
					}
					else if (maxChroma == b)
					{
						hue *= (((r - g) / deltaChroma) + 4);
					}
				}
				else
				{
					hue = 0.0f;
				}

				if (maxChroma != 0.0f) // epsilon??
				{
					saturation = deltaChroma / maxChroma;
				}

				value = maxChroma / MAX_LINEAR_RGBA_VALUE;

				return HSV(hue, saturation, value);
			}
		
			NB_NODISCARD constexpr static Color fromHsv(float h, float s, float v) noexcept
			{
				float r = 0;
				float g = 0;
				float b = 0;

				if (s == 0.0f)
				{
					r = v;
					g = v;
					b = v;
					return Color(
						static_cast<uint8_t>(r),
						static_cast<uint8_t>(g),
						static_cast<uint8_t>(b)
					);
				}

				float chroma = v * s;
				float hueSector = h / HUE_SECTOR_SIZE_IN_DEGREES;
				int sectorIndex = static_cast<int>(std::floor(hueSector)) % 6;

				float x = chroma * (1.0f - std::fabs(std::fmod(h / HUE_SECTOR_SIZE_IN_DEGREES, 2.0f) - 1.0f));

				switch (sectorIndex)
				{
				case 0:
				{
					r = chroma;
					g = x;
					b = 0;
					break;
				}
				case 1:
				{
					r = x;
					g = chroma;
					b = 0;
					break;
				}
				case 2:
				{
					r = 0;
					g = chroma;
					b = x;
					break;
				}
				case 3:
				{
					r = 0;
					g = x;
					b = chroma;
					break;
				}
				case 4:
				{
					r = x;
					g = 0;
					b = chroma;
					break;
				}
				case 5:
				{
					r = chroma;
					g = 0;
					b = x;
					break;
				}
				default:
					break;
				}

				float m = v - chroma;
				return Color(
					static_cast<uint8_t>((r + m) * MAX_LINEAR_RGBA_VALUE),
					static_cast<uint8_t>((g + m) * MAX_LINEAR_RGBA_VALUE),
					static_cast<uint8_t>((b + m) * MAX_LINEAR_RGBA_VALUE)
				);

			}

			NB_NODISCARD constexpr static Color fromRgb(uint8_t r, uint8_t g, uint8_t b) noexcept
			{
				return Color(r, g, b, MAX_LINEAR_RGBA_VALUE);
			}

			NB_NODISCARD constexpr static Color fromLinearRgb(float r, float g, float b) noexcept
			{
				return Color(
					static_cast<uint8_t>(r * MAX_LINEAR_RGBA_VALUE),
					static_cast<uint8_t>(g * MAX_LINEAR_RGBA_VALUE),
					static_cast<uint8_t>(b * MAX_LINEAR_RGBA_VALUE)
				);
			}

			NB_NODISCARD constexpr static Color fromRgba(uint8_t r, uint8_t g, uint8_t b, uint8_t alpha) noexcept
			{
				return Color(r, g, b, alpha);
			}

			NB_NODISCARD constexpr static Color fromLinearRgba(float r, float g, float b, float alpha) noexcept
			{
				return Color(
					static_cast<uint8_t>(r * MAX_LINEAR_RGBA_VALUE),
					static_cast<uint8_t>(g * MAX_LINEAR_RGBA_VALUE),
					static_cast<uint8_t>(b * MAX_LINEAR_RGBA_VALUE),
					static_cast<uint8_t>(alpha * MAX_LINEAR_RGBA_VALUE)
				);
			}

		private:

			constexpr explicit Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = MAX_LINEAR_RGBA_VALUE) noexcept
				: rgba(r, g, b, a) {}

			RGBA rgba;
		};

		namespace Colors
		{
			inline const Color RED = Color::fromRgb(255, 0, 0);
			inline const Color GREEN = Color::fromRgb(0, 255, 0);
			inline const Color BLUE = Color::fromRgb(0, 0, 255);
			inline const Color WHITE = Color::fromRgb(255, 255, 255);
			inline const Color BLACK = Color::fromRgb(0, 0, 0);
			inline const Color GRAY = Color::fromRgb(128, 128, 128);
			inline const Color LIGHT_GRAY = Color::fromRgb(192, 192, 192);
			inline const Color DARK_GRAY = Color::fromRgb(64, 64, 64);
			inline const Color YELLOW = Color::fromRgb(255, 255, 0);
			inline const Color ORANGE = Color::fromRgb(255, 165, 0);
			inline const Color CYAN = Color::fromRgb(0, 255, 255);
			inline const Color MAGENTA = Color::fromRgb(255, 0, 255);
			inline const Color PURPLE = Color::fromRgb(128, 0, 128);
			inline const Color BROWN = Color::fromRgb(139, 69, 19);
			inline const Color PINK = Color::fromRgb(255, 192, 203);
			inline const Color LIME = Color::fromRgb(50, 205, 50);
			inline const Color GOLD = Color::fromRgb(255, 215, 0);
			inline const Color SILVER = Color::fromRgb(192, 192, 192);
			inline const Color SKY_BLUE = Color::fromRgb(135, 206, 235);
			inline const Color VIOLET = Color::fromRgb(238, 130, 238);
			inline const Color BEIGE = Color::fromRgb(245, 245, 220);
			inline const Color OLIVE = Color::fromRgb(128, 128, 0);
			inline const Color NAVY = Color::fromRgb(0, 0, 128);
			inline const Color TEAL = Color::fromRgb(0, 128, 128);
		}

	};
};


#endif