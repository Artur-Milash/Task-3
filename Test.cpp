#include <gtest/gtest.h>
#include "Code_reader.h"

TEST(Code_reader_tests, Basic_count) {
	Code_reader test{ {"For_test"}, {"Test cases"}};
	test.check();

	ASSERT_EQ(test.get_total(), 816);
}

TEST(Code_reader_tests, Simple_comments) {
	Code_reader test{ {"For_test/Test cases/Simple_case.cpp"}, {} };

	test.check();

	ASSERT_EQ(test.get_code_amount(), 4);
	ASSERT_EQ(test.get_comment_amount(), 5);
	ASSERT_EQ(test.get_blank_amount(), 4);
}

TEST(Code_reader_tests, Priority_simple_comments_CODE) {
	Code_reader test{ {"For_test/Test cases/Priority_case.cpp"}, {} };
	test.set_priority(CODE_READER_CODE_PRIORITY);
	test.check();

	ASSERT_EQ(test.get_code_amount(), 4);
	ASSERT_EQ(test.get_comment_amount(), 3);
	ASSERT_EQ(test.get_blank_amount(), 2);
}

TEST(Code_reader_tests, Priority_simple_comments_COMMENT) {
	Code_reader test{ {"For_test/Test cases/Priority_case.cpp"}, {} };
	test.set_priority(CODE_READER_COMMENT_PRIORITY);
	test.check();

	ASSERT_EQ(test.get_code_amount(), 3);
	ASSERT_EQ(test.get_comment_amount(), 4);
	ASSERT_EQ(test.get_blank_amount(), 2);
}

TEST(Code_reader_tests, Priority_simple_comments_MIXED) {
	Code_reader test{ {"For_test/Test cases/Priority_case.cpp"}, {} };
	test.set_priority(CODE_READER_MIXED_PRIORITY);
	test.check();

	ASSERT_EQ(test.get_code_amount(), 4);
	ASSERT_EQ(test.get_comment_amount(), 4);
	ASSERT_EQ(test.get_blank_amount(), 2);
}

TEST(Code_reader_tests, Priority_large_comments_CODE) {
	Code_reader test{ {"For_test/Test cases/Priority_case_large.cpp"}, {} };
	test.set_priority(CODE_READER_CODE_PRIORITY);
	test.check();

	ASSERT_EQ(test.get_code_amount(), 4);
	ASSERT_EQ(test.get_comment_amount(), 5);
	ASSERT_EQ(test.get_blank_amount(), 2);
}

TEST(Code_reader_tests, Priority_large_comments_COMMENT) {
	Code_reader test{ {"For_test/Test cases/Priority_case_large.cpp"}, {} };
	test.set_priority(CODE_READER_COMMENT_PRIORITY);
	test.check();

	ASSERT_EQ(test.get_code_amount(), 3);
	ASSERT_EQ(test.get_comment_amount(), 6);
	ASSERT_EQ(test.get_blank_amount(), 2);
}

TEST(Code_reader_tests, Priority_large_comments_MIXED) {
	Code_reader test{ {"For_test/Test cases/Priority_case_large.cpp"}, {} };
	test.set_priority(CODE_READER_MIXED_PRIORITY);
	test.check();

	ASSERT_EQ(test.get_code_amount(), 4);
	ASSERT_EQ(test.get_comment_amount(), 6);
	ASSERT_EQ(test.get_blank_amount(), 2);
}


int main(int args, char** argv) {
	testing::InitGoogleTest(&args, argv);
	return RUN_ALL_TESTS();
}