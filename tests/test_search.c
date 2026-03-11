#include "unity.h"
#include "parse_db_funcs.h"

#include <sqlite3.h>

static sqlite3* g_db = NULL;

static void exec_sql(const char* sql)
{
    TEST_ASSERT_EQUAL_INT(SQLITE_OK, sqlite3_exec(g_db, sql, NULL, NULL, NULL));
}

static void seed_search_fixture(void)
{
    exec_sql(
        "INSERT INTO spotifyHistory (artist, track, album, ms_played, timestamp, track_uri) VALUES "
        "('Foo Fighters', 'Everlong', 'The Colour and the Shape', 100000, '2026-03-01T10:00:00', ''),"
        "('Foo Fighters', 'Everlong', 'The Colour and the Shape', 100000, '2026-03-02T10:00:00', ''),"
        "('Foo Fighters', 'Monkey Wrench', 'The Colour and the Shape', 100000, '2026-03-03T10:00:00', ''),"
        "('Foo Fighters', 'Walk', 'Wasting Light', 100000, '2026-03-04T10:00:00', ''),"
        "('Food House', 'Mos Thoser', 'Food House', 100000, '2026-03-05T10:00:00', ''),"
        "('Foolish', 'Run Away', 'Blue Noon', 100000, '2026-03-06T10:00:00', ''),"
        "('Bar Band', 'Foo Song', 'Live Cuts', 100000, '2026-03-07T10:00:00', ''),"
        "('Bar Band', 'Foo Song', 'Live Cuts', 100000, '2026-03-08T10:00:00', ''),"
        "('Another Artist', 'Other Track', 'Foo Album', 100000, '2026-03-09T10:00:00', ''),"
        "('Another Artist', 'Other Track 2', 'Foo Album', 100000, '2026-03-10T10:00:00', ''),"
        "('Another Artist', 'Other Track 2', 'Foo Album', 100000, '2026-03-11T10:00:00', ''),"
        "('Different Artist', 'Album Song', 'Foo Album', 100000, '2026-03-12T10:00:00', '');"
    );
}

void setUp(void)
{
    TEST_ASSERT_EQUAL_INT(SQLITE_OK, sqlite3_open(":memory:", &g_db));
    TEST_ASSERT_EQUAL_INT(0, create_db(g_db));
    seed_search_fixture();
}

void tearDown(void)
{
    if (g_db != NULL) {
        sqlite3_close(g_db);
        g_db = NULL;
    }
}

void test_search_artists_by_name_is_case_insensitive_and_ordered_by_play_count(void)
{
    artist_list list = search_artists_by_name(g_db, "foo", 3);

    TEST_ASSERT_EQUAL_INT(3, list.len);
    TEST_ASSERT_EQUAL_STRING("Foo Fighters", list.root[0].name);
    TEST_ASSERT_EQUAL_INT(4, list.root[0].num_plays);
    TEST_ASSERT_EQUAL_STRING("Food House", list.root[1].name);
    TEST_ASSERT_EQUAL_INT(1, list.root[1].num_plays);
    TEST_ASSERT_EQUAL_STRING("Foolish", list.root[2].name);
    TEST_ASSERT_EQUAL_INT(1, list.root[2].num_plays);

    free_artist_list(list);
}

void test_search_albums_by_name_matches_album_titles_and_respects_limit(void)
{
    album_list list = search_albums_by_name(g_db, "foo", 2);

    TEST_ASSERT_EQUAL_INT(2, list.len);
    TEST_ASSERT_EQUAL_STRING("Foo Album", list.root[0].name);
    TEST_ASSERT_EQUAL_STRING("Another Artist", list.root[0].artist);
    TEST_ASSERT_EQUAL_INT(3, list.root[0].num_plays);
    TEST_ASSERT_EQUAL_STRING("Foo Album", list.root[1].name);
    TEST_ASSERT_EQUAL_STRING("Different Artist", list.root[1].artist);
    TEST_ASSERT_EQUAL_INT(1, list.root[1].num_plays);

    free_album_list(list);
}

void test_search_tracks_by_name_matches_track_titles_and_orders_by_play_count(void)
{
    track_list list = search_tracks_by_name(g_db, "foo", 2);

    TEST_ASSERT_EQUAL_INT(1, list.len);
    TEST_ASSERT_EQUAL_STRING("Foo Song", list.root[0].name);
    TEST_ASSERT_EQUAL_STRING("Bar Band", list.root[0].artist);
    TEST_ASSERT_EQUAL_INT(2, list.root[0].num_plays);

    free_track_list(list);
}

void test_search_helpers_return_empty_lists_for_no_match(void)
{
    artist_list artists = search_artists_by_name(g_db, "zzzzz", 5);
    album_list albums = search_albums_by_name(g_db, "zzzzz", 5);
    track_list tracks = search_tracks_by_name(g_db, "zzzzz", 5);

    TEST_ASSERT_EQUAL_INT(0, artists.len);
    TEST_ASSERT_NULL(artists.root);
    TEST_ASSERT_EQUAL_INT(0, albums.len);
    TEST_ASSERT_NULL(albums.root);
    TEST_ASSERT_EQUAL_INT(0, tracks.len);
    TEST_ASSERT_NULL(tracks.root);
}

void test_get_top_tracks_for_artist_limit_filters_artist_case_insensitively(void)
{
    track_list list = get_top_tracks_for_artist_limit(g_db, "foo fighters", 2);

    TEST_ASSERT_EQUAL_INT(2, list.len);
    TEST_ASSERT_EQUAL_STRING("Everlong", list.root[0].name);
    TEST_ASSERT_EQUAL_INT(2, list.root[0].num_plays);
    TEST_ASSERT_EQUAL_STRING("Monkey Wrench", list.root[1].name);
    TEST_ASSERT_EQUAL_INT(1, list.root[1].num_plays);

    free_track_list(list);
}

void test_get_top_albums_for_artist_limit_returns_artist_albums_by_play_count(void)
{
    album_list list = get_top_albums_for_artist_limit(g_db, "Foo Fighters", 2);

    TEST_ASSERT_EQUAL_INT(2, list.len);
    TEST_ASSERT_EQUAL_STRING("The Colour and the Shape", list.root[0].name);
    TEST_ASSERT_EQUAL_INT(3, list.root[0].num_plays);
    TEST_ASSERT_EQUAL_STRING("Wasting Light", list.root[1].name);
    TEST_ASSERT_EQUAL_INT(1, list.root[1].num_plays);

    free_album_list(list);
}

void test_get_top_tracks_for_album_limit_can_filter_by_album_only(void)
{
    track_list list = get_top_tracks_for_album_limit(g_db, "Foo Album", NULL, 3);

    TEST_ASSERT_EQUAL_INT(3, list.len);
    TEST_ASSERT_EQUAL_STRING("Other Track 2", list.root[0].name);
    TEST_ASSERT_EQUAL_STRING("Another Artist", list.root[0].artist);
    TEST_ASSERT_EQUAL_INT(2, list.root[0].num_plays);
    TEST_ASSERT_EQUAL_STRING("Album Song", list.root[1].name);
    TEST_ASSERT_EQUAL_STRING("Different Artist", list.root[1].artist);
    TEST_ASSERT_EQUAL_INT(1, list.root[1].num_plays);
    TEST_ASSERT_EQUAL_STRING("Other Track", list.root[2].name);
    TEST_ASSERT_EQUAL_STRING("Another Artist", list.root[2].artist);
    TEST_ASSERT_EQUAL_INT(1, list.root[2].num_plays);

    free_track_list(list);
}

void test_get_top_tracks_for_album_limit_can_filter_by_album_and_artist(void)
{
    track_list list = get_top_tracks_for_album_limit(g_db, "Foo Album", "Another Artist", 3);

    TEST_ASSERT_EQUAL_INT(2, list.len);
    TEST_ASSERT_EQUAL_STRING("Other Track 2", list.root[0].name);
    TEST_ASSERT_EQUAL_INT(2, list.root[0].num_plays);
    TEST_ASSERT_EQUAL_STRING("Other Track", list.root[1].name);
    TEST_ASSERT_EQUAL_INT(1, list.root[1].num_plays);

    free_track_list(list);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_search_artists_by_name_is_case_insensitive_and_ordered_by_play_count);
    RUN_TEST(test_search_albums_by_name_matches_album_titles_and_respects_limit);
    RUN_TEST(test_search_tracks_by_name_matches_track_titles_and_orders_by_play_count);
    RUN_TEST(test_search_helpers_return_empty_lists_for_no_match);
    RUN_TEST(test_get_top_tracks_for_artist_limit_filters_artist_case_insensitively);
    RUN_TEST(test_get_top_albums_for_artist_limit_returns_artist_albums_by_play_count);
    RUN_TEST(test_get_top_tracks_for_album_limit_can_filter_by_album_only);
    RUN_TEST(test_get_top_tracks_for_album_limit_can_filter_by_album_and_artist);

    return UNITY_END();
}
