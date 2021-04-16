#include "post.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * Internal
 */

void fl_advance_to_next_post_position(MessageBoard *board)
{
    if (board->cursor + 1 == board->length) {
        board->cursor = 0;
    } else {
        board->cursor++;
    }
    board->latest_post_number++;
}

void fl_delete_post_by_index(Post *posts, uint32_t length, uint32_t i)
{
    if (i >= length) return;

    posts[i].id = NONE;
    posts[i].parent_id = NONE;
    posts[i].timestamp = NONE;
    strcpy(posts[i].title, EMPTY_STRING);
    strcpy(posts[i].text, EMPTY_STRING);
}

void fl_delete_posts_by_parent_id(MessageBoard board, uint32_t parent_id)
{
    if (parent_id == NONE) return;

    for (uint32_t i = 0; i < board.length; i++) {
        if (board.posts[i].parent_id == parent_id) {
            fl_delete_posts_by_parent_id(board, board.posts[i].id);
            fl_delete_post_by_index(board.posts, board.length, i);
        }
    }
}

uint32_t fl_find_post_index_by_id(Post *posts, uint32_t length, uint32_t id)
{
    for (uint32_t i = 0; i < length; i++) {
        if (posts[i].id == id) return i;
    }

    return INVALID;
}

uint32_t fl_find_ultimate_parent_by_post_id(MessageBoard *board, uint32_t id)
{
    for (uint32_t i = 0; i < board->length; i++) {
        if (board->posts[i].id == id && board->posts[i].parent_id != NONE) {
            return fl_find_ultimate_parent_by_post_id(board, board->posts[i].parent_id);
        }
    }

    return id;
}

/*
 * API
 */


/* Returns a board with a pointer to dynamically allocated memory which is freed by calling MessageBoard_delete(my_board).
 * Like other memory allocated to this program, the board will also be automatically deallocated when the program exits
 * See https://stackoverflow.com/questions/654754/what-really-happens-when-you-dont-free-after-malloc
 * Use MessageBoard_is_ok(MessageBoard board) to check that it was allocated and not deallocated
 */
MessageBoard MessageBoard_create(uint32_t length)
{
    MessageBoard board;
    board.cursor = 0;
    board.latest_post_number = POST_ID_BASELINE;
    board.length = length;

    if (length==0)
    {
        board.posts=NULL;
        return board;
    }

    // see http://c-faq.com/malloc/mallocnocast.html
    board.posts = malloc(sizeof(Post) * length);
    if (board.posts!=NULL)
    {
        for (uint32_t i = 0; i < length; i++) {
            board.posts[i].id = NONE;
        }
    }
    return board;
}

bool MessageBoard_is_ok(MessageBoard board) { return board.posts != NULL && board.length > 0; }

void MessageBoard_delete(MessageBoard *board)
{
    if (board->posts!=NULL) free(board->posts);
    board->posts = NULL;
    board->length = 0;
    board->cursor = 0;
}

void MessageBoard_delete_post(MessageBoard board, uint32_t id)
{
    if (!MessageBoard_is_ok(board)) return;
    for (uint32_t i = 0; i < board.length; i++) {
        if (board.posts[i].id == id) {
            fl_delete_post_by_index(board.posts, board.length, i);
            return;
        }
    }
}

void MessageBoard_delete_post_with_replies(MessageBoard board, uint32_t id)
{
    if (!MessageBoard_is_ok(board)) return;
    for (uint32_t i = 0; i < board.length; i++) {
        if (board.posts[i].id == id) {
            fl_delete_post_by_index(board.posts, board.length, i);
            fl_delete_posts_by_parent_id(board, id);
            return;
        }
    }
}

uint32_t MessageBoard_add_post(MessageBoard *board, uint32_t parent_id, char title[STR_MAX_LEN], char text[STR_MAX_LEN],
    uint32_t new_post_token, uint32_t reply_token_attempt)
{
    if (strlen(title) > STR_MAX_LEN || strlen(text) > STR_MAX_LEN || !MessageBoard_is_ok(*board)) return INVALID;

    uint32_t index = board->cursor;
    bool is_op = false;

    if (parent_id != NONE) {
        uint32_t parent_index
            = fl_find_post_index_by_id(board->posts, board->length, fl_find_ultimate_parent_by_post_id(board, parent_id));
        if (parent_index == INVALID) return INVALID;
        is_op = (reply_token_attempt == board->posts[parent_index].token);
    }

    strcpy(board->posts[index].title, title);
    strcpy(board->posts[index].text, text);
    board->posts[index].parent_id = parent_id;
    if (parent_id == NONE) board->posts[index].token = new_post_token;
    board->posts[index].id = board->latest_post_number;
    board->posts[index].timestamp = time(NULL);
    board->posts[index].author_is_op = is_op;

    fl_advance_to_next_post_position(board);

    return board->posts[index].id;
}

FetchResult MessageBoard_fetch_latest_posts(MessageBoard board, uint32_t parent_id, uint32_t offset)
{
    FetchResult result;
    result.length = 0;

    if (offset > board.length || !MessageBoard_is_ok(board)) return result;

    for (uint32_t i = 0; i < MAX_RESULTS; i++) {
        fl_delete_post_by_index(result.posts, MAX_RESULTS, i);
    }

    uint32_t result_index = 0;
    int64_t i = board.cursor - offset;
    if (i < 0) i = board.length + i;

    do {
        i--;
        if (i == -1) i = board.length;

        if (board.posts[i].id != NONE && board.posts[i].parent_id == parent_id) {
            result.posts[result_index].author_is_op = board.posts[i].author_is_op;
            result.posts[result_index].id = board.posts[i].id;
            result.posts[result_index].parent_id = board.posts[i].parent_id;
            result.posts[result_index].timestamp = board.posts[i].timestamp;

            strcpy(result.posts[result_index].title, board.posts[i].title);
            strcpy(result.posts[result_index].text, board.posts[i].text);

            result_index++;
        }
    } while (result_index < MAX_RESULTS && i != board.cursor);
    result.length = result_index;
    return result;
}
