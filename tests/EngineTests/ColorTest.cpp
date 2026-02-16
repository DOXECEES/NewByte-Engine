#include "gtest/gtest.h"
#include <Renderer/Color.hpp>

using namespace nb::Renderer;
TEST(ColorHSV, DefaultConstructor)
{
    HSV color;
    EXPECT_FLOAT_EQ(color.hue, 0.0f);
    EXPECT_FLOAT_EQ(color.saturation, 0.0f);
    EXPECT_FLOAT_EQ(color.value, 0.0f);
}

TEST(ColorHSV, NegativeHue_Asserts) {
    EXPECT_DEATH(HSV(-1.0f, 1.0f, 0.2f), ".*ASSERT FAILED:.*");
}

TEST(ColorHSV, HueGreaterThan360_Asserts) {
    EXPECT_DEATH(HSV(521.0f, 1.0f, 1.0f), ".*ASSERT FAILED:.*");
}

TEST(ColorHSV, SaturationGreaterThan1_Asserts) {
    EXPECT_DEATH(HSV(125.0f, 5.0f, 1.0f), ".*ASSERT FAILED:.*");
}

TEST(ColorHSV, NegativeSaturation_Asserts) {
    EXPECT_DEATH(HSV(125.0f, -5.0f, 1.0f), ".*ASSERT FAILED:.*");
}

TEST(ColorHSV, ValueGreaterThan1_Asserts) {
    EXPECT_DEATH(HSV(125.0f, 1.0f, 28.0f), ".*ASSERT FAILED:.*");
}

TEST(ColorHSV, NegativeValue_Asserts) {
    EXPECT_DEATH(HSV(125.0f, 1.0f, -28.0f), ".*ASSERT FAILED:.*");
}

TEST(ColorHSV, ToVec3)
{
    HSV color = { 45.0f, 0.34f, 0.33f };
    nb::Math::Vector3<float> expected = { 45.0f, 0.34f, 0.33f };
    ASSERT_EQ(color.toVec3(), expected);
}

TEST(ColorRGB, ToVec3)
{
    RGB color = { 124, 33, 44 };
    nb::Math::Vector3<float> expected = { 124.0f / 255.0f, 33.0f / 255.0f, 44.0f / 255.0f };
    ASSERT_EQ(color.toVec3(), expected);
}

TEST(ColorRGB, ToVec3Linear)
{
    RGB color = { 124, 33, 44 };
    nb::Math::Vector3<int> expected = { 124, 33, 44 };
    ASSERT_EQ(color.toVec3Linear(), expected);
}

TEST(ColorRGBA, ToVec4Linear)
{
    RGBA color = { 124, 33, 44, 255 };
    nb::Math::Vector4<int> expected = { 124, 33, 44, 255 };
    ASSERT_EQ(color.toVec4Linear(), expected);

}

TEST(ColorRGBA, ToVec4)
{
    RGBA color = { 124, 33, 44, 255 };
    nb::Math::Vector4<float> expected = { 124.0f / 255.0f, 33.0f / 255.0f, 44.0f / 255.0f, 255.0f / 255.0f };
    ASSERT_EQ(color.toVec4(), expected);
}

TEST(ColorClass, toRgb)
{
    RGB expected = { 146, 142, 224 };
    Color color = Color::fromRgb(146, 142, 224);
    ASSERT_EQ(color.toRgb(), expected);
}

TEST(ColorClass, toHsv)
{
    HSV expected = { 60.0f, 0.3f, 0.40f };
    Color color = Color::fromRgb(102, 102, 71);
    
    HSV result = color.toHsv();

    constexpr float epsilon = 0.01f; // 1% допуск

    EXPECT_NEAR(result.hue, expected.hue, epsilon);
    EXPECT_NEAR(result.saturation, expected.saturation, epsilon);
    EXPECT_NEAR(result.value, expected.value, epsilon);
}

TEST(ColorClass, fromHsv)
{
    RGB expected = { 102, 102, 71 };
    Color color = Color::fromHsv(60.0f, 0.30f, 0.40f);

    RGB result = color.toRgb();


    ASSERT_EQ(result, expected);
}

TEST(ColorClass, fromRgb)
{
    RGB expected = { 102, 102, 71 };
    Color color = Color::fromRgb( 102, 102, 71 );

    RGB result = color.toRgb();


    ASSERT_EQ(result, expected);
}

TEST(ColorClass, fromLinearRgb)
{
    RGB expected = { 102, 102, 71 };
    Color color = Color::fromLinearRgb(102.0f / 255.0f, 102.0f / 255.0f, 71.0f / 255.0f);

    RGB result = color.toRgb();


    ASSERT_EQ(result, expected);
}

TEST(ColorClass, fromRgba)
{
    RGB expected = { 102, 102, 71};
    Color color = Color::fromRgba(102, 102, 71,255);

    RGB result = color.toRgb();


    ASSERT_EQ(result, expected);
}

TEST(ColorClass, fromLinearRgba)
{
    RGB expected = { 102, 102, 71 };
    Color color = Color::fromLinearRgba(102.0f / 255.0f, 102.0f / 255.0f, 71.0f / 255.0f, 255.0f / 255.0f);

    RGB result = color.toRgb();


    ASSERT_EQ(result, expected);
}

TEST(ColorClass, toRgba)
{
    RGBA expected = { 102, 102, 71, 126 };
    Color color = Color::fromLinearRgba(102.0f / 255.0f, 102.0f / 255.0f, 71.0f / 255.0f, 126.0f / 255.0f);

    RGBA result = color.toRgba();


    ASSERT_EQ(result, expected);
}

TEST(ColorClass, asVec3)
{
    nb::Math::Vector3<float> expected = { 102.0f / 255.0f, 102.0f / 255.0f, 71.0f / 255.0f };
    Color color = Color::fromRgb(102, 102, 71);
    auto result = color.asVec3();

    ASSERT_EQ(result, expected);
}

TEST(ColorClass, asVec4)
{
    nb::Math::Vector4<float> expected = { 102.0f / 255.0f, 102.0f / 255.0f, 71.0f / 255.0f, 241.0f / 255.0f };
    Color color = Color::fromRgba(102, 102, 71, 241);
    auto result = color.asVec4();

    ASSERT_EQ(result, expected);
}
