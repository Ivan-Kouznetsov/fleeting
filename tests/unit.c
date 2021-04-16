#define MUNIT_ENABLE_ASSERT_ALIASES
#define TEST(name)  { (char *)#name, name, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }

#include "../post.h"
#include "munit.h"
#include <stdlib.h>
#include <string.h>
#include "long_strings.h"

#pragma region Helpers
/*
 * Helpers
 */

void reverse(char *s)
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char *s)
{
    int i, sign;

    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0) s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

char *add_num_to_string(char *s, int n)
{
    char num_string[STR_MAX_LEN] = "";
    itoa(n, num_string);
    strcat(s, num_string);
    return s;
}

void populate_board(MessageBoard *board, int additional)
{
    char title[STR_MAX_LEN] = "Title ";
    char text[STR_MAX_LEN] = "Text ";

    for (uint32_t i = 0; i < board->length + additional; i++) {
        MessageBoard_add_post(board, NONE, add_num_to_string(title, i), add_num_to_string(text, i), 1000 + i, 0);
        strcpy(title, "Title ");
        strcpy(text, "Text ");
    }
}

void populate_board_with_replies(MessageBoard *board, uint32_t token)
{
    char title[STR_MAX_LEN] = "Title ";
    char text[STR_MAX_LEN] = "Text ";

    for (uint32_t i = 0; i < board->length; i++) {
        MessageBoard_add_post(board, i ? i + POST_ID_BASELINE-1 : NONE, add_num_to_string(title, i), add_num_to_string(text, i),
            token, 0);
        strcpy(title, "Title ");
        strcpy(text, "Text ");
    }
}
#pragma endregion /* Helpers */
/*
 * Tests
 */


/*
 * Internal functions
 */

static MunitResult should_advance_to_next_post_position()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);

    for(uint32_t i=0;i< board_len; i++){
        assert_uint32(board.cursor, ==, i);
        assert_uint32(board.latest_post_number, ==, i + POST_ID_BASELINE);
        fl_advance_to_next_post_position(&board);
    }

    assert_uint32(board.cursor, ==, 0);
    assert_uint32(board.latest_post_number, ==, board_len + POST_ID_BASELINE);

    return MUNIT_OK;
}

static MunitResult should_delete_post_by_index()
{
    const uint32_t board_len = 10;
    const uint32_t delete_index = 5;
    MessageBoard board = MessageBoard_create(board_len);

    populate_board(&board, 0);

    fl_delete_post_by_index(board.posts, board.length, 5);

    for(uint32_t i= 0; i < board_len; i++){
        if (i==delete_index)
        {
            assert_uint32(board.posts[i].id, ==, NONE);
        }else{
            assert_uint32(board.posts[i].id, ==, i + POST_ID_BASELINE);
        }
    }

    return MUNIT_OK;
}

static MunitResult should_delete_posts_by_parent_id()
{
    const uint32_t board_len = 5;
    const uint32_t parent_id = 100;
    MessageBoard board = MessageBoard_create(board_len);

    MessageBoard_add_post(&board,0,EMPTY_STRING,EMPTY_STRING,NONE,NONE);
    MessageBoard_add_post(&board,parent_id,EMPTY_STRING,EMPTY_STRING,NONE,NONE);
    MessageBoard_add_post(&board,parent_id,EMPTY_STRING,EMPTY_STRING,NONE,NONE);
    MessageBoard_add_post(&board,parent_id,EMPTY_STRING,EMPTY_STRING,NONE,NONE);
    MessageBoard_add_post(&board,parent_id,EMPTY_STRING,EMPTY_STRING,NONE,NONE);

    fl_delete_posts_by_parent_id(board, parent_id);

    for(uint32_t i=0; i < board_len; i++){
        if (board.posts[i].parent_id == parent_id)
        {
            assert_uint32(board.posts[i].id, ==, NONE);
        }
    }

    return MUNIT_OK;
}

static MunitResult should_find_ultimate_parent_by_post_id()
{
    const uint32_t board_len = 5;
    MessageBoard board = MessageBoard_create(board_len);

    MessageBoard_add_post(&board,0,EMPTY_STRING,EMPTY_STRING,NONE,NONE);
    MessageBoard_add_post(&board,100,EMPTY_STRING,EMPTY_STRING,NONE,NONE);
    MessageBoard_add_post(&board,101,EMPTY_STRING,EMPTY_STRING,NONE,NONE);
    MessageBoard_add_post(&board,102,EMPTY_STRING,EMPTY_STRING,NONE,NONE);
    uint32_t new_id = MessageBoard_add_post(&board,103,EMPTY_STRING,EMPTY_STRING,NONE,NONE);

    uint32_t parent_id = fl_find_ultimate_parent_by_post_id(&board, new_id);

    assert_uint32(parent_id, ==, 100);

    return MUNIT_OK;
}

static MunitResult should_find_post_index_by_id()
{
     const uint32_t board_len = 5;
    MessageBoard board = MessageBoard_create(board_len);

    MessageBoard_add_post(&board,0,EMPTY_STRING,EMPTY_STRING,NONE,NONE);
    MessageBoard_add_post(&board,100,EMPTY_STRING,EMPTY_STRING,NONE,NONE);
    MessageBoard_add_post(&board,101,EMPTY_STRING,EMPTY_STRING,NONE,NONE);
    MessageBoard_add_post(&board,102,EMPTY_STRING,EMPTY_STRING,NONE,NONE);

    uint32_t index = fl_find_post_index_by_id(board.posts, board_len, 102);

    assert_uint32(index, ==, 2);

    return MUNIT_OK;
}

/*
 * Happy paths
 */

static MunitResult should_create_board()
{
    const uint32_t board_len = 1000;
    MessageBoard board = MessageBoard_create(board_len);

    assert_true(MessageBoard_is_ok(board));
    assert_uint32(board.length, ==, board_len);
    assert_uint32(board.cursor, ==, 0);

    for (uint32_t i = 0; i < board_len; i++) {
        assert_uint32(board.posts[i].id, ==, 0);
    }

    return MUNIT_OK;
}

static MunitResult should_delete_board()
{
    const uint32_t board_len = 1000;
    MessageBoard board = MessageBoard_create(board_len);
    MessageBoard_delete(&board);

    assert_ptr_null(board.posts);
    assert_uint32(board.length, ==, 0);

    return MUNIT_OK;
}

static MunitResult should_add_posts()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);
    populate_board(&board, 0);

    assert_string_equal(board.posts[0].title, "Title 0");
    assert_string_equal(board.posts[1].title, "Title 1");
    assert_string_equal(board.posts[2].title, "Title 2");
    assert_string_equal(board.posts[3].title, "Title 3");
    assert_string_equal(board.posts[4].title, "Title 4");
    assert_string_equal(board.posts[5].title, "Title 5");
    assert_string_equal(board.posts[6].title, "Title 6");
    assert_string_equal(board.posts[7].title, "Title 7");
    assert_string_equal(board.posts[8].title, "Title 8");
    assert_string_equal(board.posts[9].title, "Title 9");

    assert_uint32(board.posts[0].id, ==, 100);
    assert_uint32(board.posts[1].id, ==, 101);
    assert_uint32(board.posts[2].id, ==, 102);
    assert_uint32(board.posts[3].id, ==, 103);
    assert_uint32(board.posts[4].id, ==, 104);
    assert_uint32(board.posts[5].id, ==, 105);
    assert_uint32(board.posts[6].id, ==, 106);
    assert_uint32(board.posts[7].id, ==, 107);
    assert_uint32(board.posts[8].id, ==, 108);
    assert_uint32(board.posts[9].id, ==, 109);

    return MUNIT_OK;
}

static MunitResult should_delete_posts_by_index()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);
    populate_board(&board, 0);

    fl_delete_post_by_index(board.posts, board.length, 5);
    assert_uint32(board.posts[5].id, ==, NONE);

    return MUNIT_OK;
}

static MunitResult should_delete_posts_by_id()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);
    populate_board(&board, 0);

    MessageBoard_delete_post(board, 105);
    assert_uint32(board.posts[5].id, ==, NONE);

    return MUNIT_OK;
}

static MunitResult should_overwrite_posts_with_new_ids()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);

    populate_board(&board, 5);

    assert_string_equal(board.posts[0].title, "Title 10");
    assert_string_equal(board.posts[1].title, "Title 11");
    assert_string_equal(board.posts[2].title, "Title 12");
    assert_string_equal(board.posts[3].title, "Title 13");
    assert_string_equal(board.posts[4].title, "Title 14");
    assert_string_equal(board.posts[5].title, "Title 5");
    assert_string_equal(board.posts[6].title, "Title 6");
    assert_string_equal(board.posts[7].title, "Title 7");
    assert_string_equal(board.posts[8].title, "Title 8");
    assert_string_equal(board.posts[9].title, "Title 9");

    assert_uint32(board.posts[0].id, ==, 10 + POST_ID_BASELINE);
    assert_uint32(board.posts[1].id, ==, 11 + POST_ID_BASELINE);
    assert_uint32(board.posts[2].id, ==, 12 + POST_ID_BASELINE);
    assert_uint32(board.posts[3].id, ==, 13 + POST_ID_BASELINE);
    assert_uint32(board.posts[4].id, ==, 14 + POST_ID_BASELINE);
    assert_uint32(board.posts[5].id, ==, 5 + POST_ID_BASELINE);
    assert_uint32(board.posts[6].id, ==, 6 + POST_ID_BASELINE);
    assert_uint32(board.posts[7].id, ==, 7 + POST_ID_BASELINE);
    assert_uint32(board.posts[8].id, ==, 8 + POST_ID_BASELINE);
    assert_uint32(board.posts[9].id, ==, 9 + POST_ID_BASELINE);

    return MUNIT_OK;
}

static MunitResult should_fetch_latest_posts()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);

    populate_board(&board, 5);

    FetchResult result = MessageBoard_fetch_latest_posts(board, 0, 0);

    assert_string_equal(result.posts[0].title, "Title 14");
    assert_string_equal(result.posts[1].title, "Title 13");
    assert_string_equal(result.posts[2].title, "Title 12");
    assert_string_equal(result.posts[3].title, "Title 11");
    assert_string_equal(result.posts[4].title, "Title 10");
    assert_string_equal(result.posts[5].title, "Title 9");
    assert_string_equal(result.posts[6].title, "Title 8");
    assert_string_equal(result.posts[7].title, "Title 7");
    assert_string_equal(result.posts[8].title, "Title 6");
    assert_string_equal(result.posts[9].title, "Title 5");

    return MUNIT_OK;
}

static MunitResult should_skip_deleted_when_fetching_latest_posts()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);

    populate_board(&board, 0);
    MessageBoard_delete_post(board, 105);
    MessageBoard_delete_post(board, 106);

    FetchResult result = MessageBoard_fetch_latest_posts(board, 0, 0);
    assert_uint32(result.length, ==, 8);

    assert_string_equal(result.posts[0].title, "Title 9");
    assert_string_equal(result.posts[1].title, "Title 8");
    assert_string_equal(result.posts[2].title, "Title 7");
    assert_string_equal(result.posts[3].title, "Title 4");
    assert_string_equal(result.posts[4].title, "Title 3");
    assert_string_equal(result.posts[5].title, "Title 2");
    assert_string_equal(result.posts[6].title, "Title 1");
    assert_string_equal(result.posts[7].title, "Title 0");

    return MUNIT_OK;
}

static MunitResult should_fetch_reply()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);

    populate_board_with_replies(&board, NONE);


    FetchResult result = MessageBoard_fetch_latest_posts(board, 101, 0);
    assert_uint32(result.length, ==, 1);

    assert_string_equal(result.posts[0].title, "Title 2");


    return MUNIT_OK;
}

static MunitResult should_find_ultimate_parent()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);
    populate_board_with_replies(&board, NONE);

    uint32_t result = fl_find_ultimate_parent_by_post_id(&board, 108);

    assert_uint32(result, ==, 100);

    return MUNIT_OK;
}

static MunitResult should_set_is_op()
{
    const uint32_t board_len = 10;
    const uint32_t token = 777;
    const uint32_t reply_to = 105;
    MessageBoard board = MessageBoard_create(board_len);
    populate_board_with_replies(&board, token);

    MessageBoard_add_post(&board, reply_to, "OP reply", EMPTY_STRING, NONE, token);

    assert_true(board.posts[0].author_is_op);
    assert_string_equal(board.posts[0].title, "OP reply");

    for(uint32_t i=1; i<board_len;i++)
    {
       assert_false(board.posts[i].author_is_op);
    }

    return MUNIT_OK;
}

/*
 * Exception paths
 */

static MunitResult should_disallow_strings_longer_than_max_length()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);

    assert_int(strlen(THREE_THOUSAND_BYTE_STR),==,3000);

    uint32_t title_too_long_id = MessageBoard_add_post(&board,NONE,THREE_THOUSAND_BYTE_STR,EMPTY_STRING,NONE,NONE);
    uint32_t body_too_long_id = MessageBoard_add_post(&board,NONE,EMPTY_STRING, THREE_THOUSAND_BYTE_STR,NONE,NONE);

    assert_uint32(title_too_long_id, ==, INVALID);
    assert_uint32(body_too_long_id, ==, INVALID);

    return MUNIT_OK;
}

static MunitResult should_disallow_nonexistent_parent_id()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);

    uint32_t nonexistent_parent_id = MessageBoard_add_post(&board,10000,EMPTY_STRING,EMPTY_STRING,NONE,NONE);

    assert_uint32(nonexistent_parent_id, ==, INVALID);

    return MUNIT_OK;
}

static MunitResult should_see_deleted_board_as_not_ok()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);
    MessageBoard_delete(&board);

    assert_false(MessageBoard_is_ok(board));

    return MUNIT_OK;
}

static MunitResult should_not_crash_when_allocating_100M_post_board()
{
    const uint32_t board_len = 100000000;
    MessageBoard board = MessageBoard_create(board_len);

    assert_false(MessageBoard_is_ok(board));

    return MUNIT_OK;
}

static MunitResult should_not_crash_when_allocating_0_post_board()
{
    const uint32_t board_len = 0;
    MessageBoard board = MessageBoard_create(board_len);

    assert_false(MessageBoard_is_ok(board));
    assert_ptr_null(board.posts);
    return MUNIT_OK;
}

static MunitResult should_not_double_free_board()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);
    MessageBoard_delete(&board);
    MessageBoard_delete(&board);

    return MUNIT_OK;
}

static MunitResult should_not_add_post_when_board_is_invalid()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);
    MessageBoard_delete(&board);

    assert_false(MessageBoard_is_ok(board));
    assert_uint32(MessageBoard_add_post(&board,NONE,EMPTY_STRING,EMPTY_STRING,NONE,NONE), ==, INVALID);

    return MUNIT_OK;
}

static MunitResult should_not_fetch_when_board_is_invalid()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);
    MessageBoard_delete(&board);

    assert_false(MessageBoard_is_ok(board));
    FetchResult result = MessageBoard_fetch_latest_posts(board,NONE,0);
    assert_uint32(result.length, ==, 0);

    return MUNIT_OK;
}

static MunitResult should_not_delete_when_invalid_board_is_used()
{
    const uint32_t board_len = 10;
    MessageBoard board = MessageBoard_create(board_len);
    MessageBoard_delete(&board);

    assert_false(MessageBoard_is_ok(board));
    MessageBoard_delete_post(board, 100);
    MessageBoard_delete_post_with_replies(board, 100);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] =
    {
      TEST(should_advance_to_next_post_position),
      TEST(should_delete_post_by_index),
      TEST(should_delete_posts_by_parent_id),
      TEST(should_find_ultimate_parent_by_post_id),
      TEST(should_find_post_index_by_id),
      TEST(should_create_board),
      TEST(should_delete_board),
      TEST(should_add_posts),
      TEST(should_delete_posts_by_index),
      TEST(should_delete_posts_by_id),
      TEST(should_overwrite_posts_with_new_ids),
      TEST(should_fetch_latest_posts),
      TEST(should_skip_deleted_when_fetching_latest_posts),
      TEST(should_fetch_reply),
      TEST(should_find_ultimate_parent),
      TEST(should_set_is_op),
      TEST(should_disallow_strings_longer_than_max_length),
      TEST(should_disallow_nonexistent_parent_id),
      TEST(should_see_deleted_board_as_not_ok),
      TEST(should_not_crash_when_allocating_100M_post_board),
      TEST(should_not_crash_when_allocating_0_post_board),
      TEST(should_not_double_free_board),
      TEST(should_not_add_post_when_board_is_invalid),
      TEST(should_not_fetch_when_board_is_invalid),
      TEST(should_not_delete_when_invalid_board_is_used)
    };

#pragma region Run suite
static const MunitSuite test_suite = { (char *)"", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE };

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, (void *)"µnit", argc, argv);
}
#pragma endregion /* Run suite */
