#include "unity.h"
#include "cli.h"

#include <string.h>

static void reset_cli_options(void)
{
    CLI_OPTIONS = (cli_options) {
        .db_path = "spotifyHistory.db",
        .json_path = "",
        .search_kind = NULL,
        .search_query = NULL,
        .recent_count = 0,
        .album_count = 0,
        .album_bottom_count = 0,
        .song_count = 0,
        .song_bottom_count = 0,
        .artist_count = 0,
        .artist_bottom_count = 0,
        .search_limit = 0
    };
}

void setUp(void)
{
    reset_cli_options();
}

void tearDown(void)
{
}

void test_handle_args_returns_zero_for_help(void)
{
    char* argv[] = {"stats", "--help"};

    TEST_ASSERT_EQUAL_INT(0, handle_args(2, argv));
}

void test_handle_args_sets_custom_db_and_json_paths(void)
{
    char* argv[] = {"stats", "--db", "temp.db", "--json", "data/history.json"};

    TEST_ASSERT_EQUAL_INT(-1, handle_args(5, argv));
    TEST_ASSERT_EQUAL_STRING("temp.db", CLI_OPTIONS.db_path);
    TEST_ASSERT_EQUAL_STRING("data/history.json", CLI_OPTIONS.json_path);
}

void test_handle_args_sets_search_mode_for_songs(void)
{
    char* argv[] = {"stats", "--search", "songs", "10", "foo"};

    TEST_ASSERT_EQUAL_INT(-1, handle_args(5, argv));
    TEST_ASSERT_EQUAL_STRING("songs", CLI_OPTIONS.search_kind);
    TEST_ASSERT_EQUAL_INT(10, CLI_OPTIONS.search_limit);
    TEST_ASSERT_EQUAL_STRING("foo", CLI_OPTIONS.search_query);
}

void test_handle_args_sets_search_mode_for_find_alias(void)
{
    char* argv[] = {"stats", "--find", "albums", "5", "foo"};

    TEST_ASSERT_EQUAL_INT(-1, handle_args(5, argv));
    TEST_ASSERT_EQUAL_STRING("albums", CLI_OPTIONS.search_kind);
    TEST_ASSERT_EQUAL_INT(5, CLI_OPTIONS.search_limit);
    TEST_ASSERT_EQUAL_STRING("foo", CLI_OPTIONS.search_query);
}

void test_handle_args_sets_search_mode_for_short_alias(void)
{
    char* argv[] = {"stats", "-f", "s", "3", "foo"};

    TEST_ASSERT_EQUAL_INT(-1, handle_args(5, argv));
    TEST_ASSERT_EQUAL_STRING("songs", CLI_OPTIONS.search_kind);
    TEST_ASSERT_EQUAL_INT(3, CLI_OPTIONS.search_limit);
    TEST_ASSERT_EQUAL_STRING("foo", CLI_OPTIONS.search_query);
}

void test_handle_args_sets_history_mode(void)
{
    char* argv[] = {"stats", "-h", "7"};

    TEST_ASSERT_EQUAL_INT(-1, handle_args(3, argv));
    TEST_ASSERT_EQUAL_INT(7, CLI_OPTIONS.recent_count);
}

void test_handle_args_sets_artists_mode(void)
{
    char* argv[] = {"stats", "--artists", "4"};

    TEST_ASSERT_EQUAL_INT(-1, handle_args(3, argv));
    TEST_ASSERT_EQUAL_INT(4, CLI_OPTIONS.artist_count);
}

void test_handle_args_sets_albums_bottom_mode(void)
{
    char* argv[] = {"stats", "-A", "2"};

    TEST_ASSERT_EQUAL_INT(-1, handle_args(3, argv));
    TEST_ASSERT_EQUAL_INT(2, CLI_OPTIONS.album_bottom_count);
}

void test_handle_args_sets_songs_bottom_mode(void)
{
    char* argv[] = {"stats", "--songs-bottom", "9"};

    TEST_ASSERT_EQUAL_INT(-1, handle_args(3, argv));
    TEST_ASSERT_EQUAL_INT(9, CLI_OPTIONS.song_bottom_count);
}

void test_handle_args_rejects_missing_db_path(void)
{
    char* argv[] = {"stats", "--db"};

    TEST_ASSERT_EQUAL_INT(2, handle_args(2, argv));
}

void test_handle_args_rejects_invalid_search_type(void)
{
    char* argv[] = {"stats", "--search", "artists", "5", "foo"};

    TEST_ASSERT_EQUAL_INT(2, handle_args(5, argv));
}

void test_handle_args_rejects_invalid_search_count(void)
{
    char* argv[] = {"stats", "--search", "songs", "0", "foo"};

    TEST_ASSERT_EQUAL_INT(2, handle_args(5, argv));
}

void test_handle_args_rejects_unknown_option(void)
{
    char* argv[] = {"stats", "--bogus"};

    TEST_ASSERT_EQUAL_INT(2, handle_args(2, argv));
}

void test_handle_args_rejects_multiple_print_modes(void)
{
    char* argv[] = {"stats", "-h", "5", "-r", "5"};

    TEST_ASSERT_EQUAL_INT(2, handle_args(5, argv));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_handle_args_returns_zero_for_help);
    RUN_TEST(test_handle_args_sets_custom_db_and_json_paths);
    RUN_TEST(test_handle_args_sets_search_mode_for_songs);
    RUN_TEST(test_handle_args_sets_search_mode_for_find_alias);
    RUN_TEST(test_handle_args_sets_search_mode_for_short_alias);
    RUN_TEST(test_handle_args_sets_history_mode);
    RUN_TEST(test_handle_args_sets_artists_mode);
    RUN_TEST(test_handle_args_sets_albums_bottom_mode);
    RUN_TEST(test_handle_args_sets_songs_bottom_mode);
    RUN_TEST(test_handle_args_rejects_missing_db_path);
    RUN_TEST(test_handle_args_rejects_invalid_search_type);
    RUN_TEST(test_handle_args_rejects_invalid_search_count);
    RUN_TEST(test_handle_args_rejects_unknown_option);
    RUN_TEST(test_handle_args_rejects_multiple_print_modes);

    return UNITY_END();
}
