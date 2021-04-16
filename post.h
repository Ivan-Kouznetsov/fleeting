#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define STR_MAX_LEN 2048
#define MAX_RESULTS (uint32_t)100
#define POST_ID_BASELINE (uint32_t)100
#define NONE (uint32_t)0
#define INVALID (uint32_t)-1
#define EMPTY_STRING ""

/*
 * Types
 */
typedef struct Post {
    uint32_t id;
    uint32_t parent_id;
    uint32_t token;
    time_t timestamp;
    char title[STR_MAX_LEN];
    char text[STR_MAX_LEN];
    bool author_is_op;
} Post;

typedef struct MessageBoard {
    Post *posts;
    uint32_t length;
    uint32_t cursor;
    uint32_t latest_post_number;
} MessageBoard;

typedef struct FetchResult {
    Post posts[MAX_RESULTS];
    uint32_t length;
} FetchResult;

/*
 * Internal
 */

void fl_advance_to_next_post_position(MessageBoard *board);
void fl_delete_post_by_index(Post *posts, uint32_t length, uint32_t i);
void fl_delete_posts_by_parent_id(MessageBoard board, uint32_t parent_id);
uint32_t fl_find_ultimate_parent_by_post_id(MessageBoard *board, uint32_t id);
uint32_t fl_find_post_index_by_id(Post *posts, uint32_t length, uint32_t id);

/*
 * API
 */

// functions that make changes
MessageBoard MessageBoard_create(uint32_t length);
void MessageBoard_delete(MessageBoard *board);
void MessageBoard_delete_post(MessageBoard board, uint32_t id);
void MessageBoard_delete_post_with_replies(MessageBoard board, uint32_t id);
uint32_t MessageBoard_add_post(MessageBoard *board, uint32_t parent_id, char title[], char text[], uint32_t new_post_token,
    uint32_t reply_token_attempt);

// read-only functions
bool MessageBoard_is_ok(MessageBoard board);
FetchResult MessageBoard_fetch_latest_posts(MessageBoard board, uint32_t parent_id, uint32_t offset);
