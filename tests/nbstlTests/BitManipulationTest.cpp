#include "gtest/gtest.h"
#include <BitManipulation.hpp>

TEST(BitManipulation, LowBitEqualOneInULL)
{
	bool res = nbstl::getLowBit(1ULL);
	EXPECT_EQ(1, res);
}

TEST(BitManipulation, LowBitEqualZeroInULL)
{
	bool res = nbstl::getLowBit(0ULL);
	EXPECT_EQ(0, res);
}

TEST(BitManipulation, LowBitEqualOneInUchar)
{
	bool res = nbstl::getLowBit(uint8_t(1));
	EXPECT_EQ(1, res);
}

TEST(BitManipulation, LowBitEqualZeroInUchar)
{
	bool res = nbstl::getLowBit(uint8_t(0));
	EXPECT_EQ(0, res);
}

TEST(BitManipulation, HiBitEqualOneInULL)
{
	bool res = nbstl::getHighBit(uint16_t(0x8000));
	EXPECT_EQ(1, res);
}

TEST(BitManipulation, HiBitEqualZeroInShort)
{
	bool res = nbstl::getHighBit(0xFFFE);
	EXPECT_EQ(0, res);
}

TEST(BitManipulation, HiBitEqualOneInUchar)
{
	bool res = nbstl::getHighBit(uint8_t(255));
	EXPECT_EQ(1, res);
}

TEST(BitManipulation, HiBitEqualZeroInUchar)
{
	bool res = nbstl::getHighBit(uint8_t(0));
	EXPECT_EQ(0, res);
}
