#include <gtest/gtest.h>

TEST(SimpleTest, TrueIsAlwaysAndForeverWillBeTrue)
{
	EXPECT_EQ(true, true);
}

int main(int ac, char* av[])
{
	testing::InitGoogleTest(&ac, av);
	return RUN_ALL_TESTS();
}