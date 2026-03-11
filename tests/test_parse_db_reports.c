#include "unity.h"
#include "parse_db_funcs.h"

#include <sqlite3.h>
#include <stdlib.h>

static sqlite3* g_db = NULL;

static void exec_sql(const char* sql)
{
    TEST_ASSERT_EQUAL_INT(SQLITE_OK, sqlite3_exec(g_db, sql, NULL, NULL, NULL));
}

static void seed_report_fixture(void)
{
    exec_sql(
        "INSERT INTO spotifyHistory (artist, track, album, ms_played, timestamp, track_uri) VALUES "
        "('Alpha', 'Song A1', 'Album A', 100000, '2026-03-01T10:00:00', ''),"
        "('Alpha', 'Song A1', 'Album A', 100000, '2026-03-02T10:00:00', ''),"
        "('Alpha', 'Song A2', 'Album A', 100000, '2026-03-03T10:00:00', ''),"
        "('Beta', 'Song B1', 'Album B', 100000, '2026-03-04T10:00:00', ''),"
        "('Beta', 'Song B1', 'Album B', 100000, '2026-03-05T10:00:00', ''),"
        "('Gamma', 'Song G1', 'Album G', 100000, '2026-03-06T10:00:00', ''),"
        "('Delta', 'Song D1', 'Album D', 100000, '2026-03-07T10:00:00', '');"
    );
}

void setUp(void)
{
    TEST_ASSERT_EQUAL_INT(SQLITE_OK, sqlite3_open(":memory:", &g_db));
    TEST_ASSERT_EQUAL_INT(0, create_db(g_db));
    seed_report_fixture();
}

void tearDown(void)
{
    if (g_db != NULL) {
        sqlite3_close(g_db);
        g_db = NULL;
    }
}

void test_get_top_artists_limit_returns_highest_play_counts(void)
{
    artist_list list = get_top_artists_limit(g_db, 2);

    TEST_ASSERT_EQUAL_INT(2, list.len);
    TEST_ASSERT_EQUAL_STRING("Alpha", list.root[0].name);
    TEST_ASSERT_EQUAL_INT(3, list.root[0].num_plays);
    TEST_ASSERT_EQUAL_STRING("Beta", list.root[1].name);
    TEST_ASSERT_EQUAL_INT(2, list.root[1].num_plays);

    free_artist_list(list);
}

void test_get_bottom_artists_limit_returns_lowest_play_counts_with_tiebreak(void)
{
    artist_list list = get_bottom_artists_limit(g_db, 2);

    TEST_ASSERT_EQUAL_INT(2, list.len);
    TEST_ASSERT_EQUAL_STRING("Delta", list.root[0].name);
    TEST_ASSERT_EQUAL_INT(1, list.root[0].num_plays);
    TEST_ASSERT_EQUAL_STRING("Gamma", list.root[1].name);
    TEST_ASSERT_EQUAL_INT(1, list.root[1].num_plays);

    free_artist_list(list);
}

void test_get_top_albums_limit_returns_highest_play_counts(void)
{
    album_list list = get_top_albums_limit(g_db, 2);

    TEST_ASSERT_EQUAL_INT(2, list.len);
    TEST_ASSERT_EQUAL_STRING("Album A", list.root[0].name);
    TEST_ASSERT_EQUAL_STRING("Alpha", list.root[0].artist);
    TEST_ASSERT_EQUAL_INT(3, list.root[0].num_plays);
    TEST_ASSERT_EQUAL_STRING("Album B", list.root[1].name);
    TEST_ASSERT_EQUAL_STRING("Beta", list.root[1].artist);
    TEST_ASSERT_EQUAL_INT(2, list.root[1].num_plays);

    free_album_list(list);
}

void test_get_bottom_albums_limit_returns_lowest_play_counts_with_tiebreak(void)
{
    album_list list = get_bottom_albums_limit(g_db, 2);

    TEST_ASSERT_EQUAL_INT(2, list.len);
    TEST_ASSERT_EQUAL_STRING("Album D", list.root[0].name);
    TEST_ASSERT_EQUAL_STRING("Delta", list.root[0].artist);
    TEST_ASSERT_EQUAL_INT(1, list.root[0].num_plays);
    TEST_ASSERT_EQUAL_STRING("Album G", list.root[1].name);
    TEST_ASSERT_EQUAL_STRING("Gamma", list.root[1].artist);
    TEST_ASSERT_EQUAL_INT(1, list.root[1].num_plays);

    free_album_list(list);
}

void test_get_top_tracks_limit_returns_highest_play_counts(void)
{
    track_list list = get_top_tracks_limit(g_db, 2);

    TEST_ASSERT_EQUAL_INT(2, list.len);
    TEST_ASSERT_EQUAL_STRING("Song A1", list.root[0].name);
    TEST_ASSERT_EQUAL_STRING("Album A", list.root[0].album);
    TEST_ASSERT_EQUAL_STRING("Alpha", list.root[0].artist);
    TEST_ASSERT_EQUAL_INT(2, list.root[0].num_plays);
    TEST_ASSERT_EQUAL_STRING("Song B1", list.root[1].name);
    TEST_ASSERT_EQUAL_STRING("Album B", list.root[1].album);
    TEST_ASSERT_EQUAL_STRING("Beta", list.root[1].artist);
    TEST_ASSERT_EQUAL_INT(2, list.root[1].num_plays);

    free_track_list(list);
}

void test_get_bottom_tracks_limit_returns_lowest_play_counts_with_tiebreak(void)
{
    track_list list = get_bottom_tracks_limit(g_db, 2);

    TEST_ASSERT_EQUAL_INT(2, list.len);
    TEST_ASSERT_EQUAL_STRING("Song A2", list.root[0].name);
    TEST_ASSERT_EQUAL_STRING("Album A", list.root[0].album);
    TEST_ASSERT_EQUAL_STRING("Alpha", list.root[0].artist);
    TEST_ASSERT_EQUAL_INT(1, list.root[0].num_plays);
    TEST_ASSERT_EQUAL_STRING("Song D1", list.root[1].name);
    TEST_ASSERT_EQUAL_STRING("Album D", list.root[1].album);
    TEST_ASSERT_EQUAL_STRING("Delta", list.root[1].artist);
    TEST_ASSERT_EQUAL_INT(1, list.root[1].num_plays);

    free_track_list(list);
}

void test_get_listening_history_limit_returns_most_recent_rows_first(void)
{
    song_list list = get_listening_history_limit(g_db, 3);

    TEST_ASSERT_EQUAL_INT(3, list.num_songs);
    TEST_ASSERT_EQUAL_STRING("Delta", list.songs[0].artist);
    TEST_ASSERT_EQUAL_STRING("Song D1", list.songs[0].track);
    TEST_ASSERT_EQUAL_STRING("2026-03-07T10:00:00", list.songs[0].timestamp);
    TEST_ASSERT_EQUAL_STRING("Gamma", list.songs[1].artist);
    TEST_ASSERT_EQUAL_STRING("Beta", list.songs[2].artist);

    free_song_list(&list);
}

void test_report_limits_can_exceed_row_count(void)
{
    artist_list artists = get_top_artists_limit(g_db, 99);
    album_list albums = get_top_albums_limit(g_db, 99);
    track_list tracks = get_top_tracks_limit(g_db, 99);
    song_list history = get_listening_history_limit(g_db, 99);

    TEST_ASSERT_EQUAL_INT(4, artists.len);
    TEST_ASSERT_EQUAL_INT(4, albums.len);
    TEST_ASSERT_EQUAL_INT(5, tracks.len);
    TEST_ASSERT_EQUAL_INT(7, history.num_songs);

    free_artist_list(artists);
    free_album_list(albums);
    free_track_list(tracks);
    free_song_list(&history);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_get_top_artists_limit_returns_highest_play_counts);
    RUN_TEST(test_get_bottom_artists_limit_returns_lowest_play_counts_with_tiebreak);
    RUN_TEST(test_get_top_albums_limit_returns_highest_play_counts);
    RUN_TEST(test_get_bottom_albums_limit_returns_lowest_play_counts_with_tiebreak);
    RUN_TEST(test_get_top_tracks_limit_returns_highest_play_counts);
    RUN_TEST(test_get_bottom_tracks_limit_returns_lowest_play_counts_with_tiebreak);
    RUN_TEST(test_get_listening_history_limit_returns_most_recent_rows_first);
    RUN_TEST(test_report_limits_can_exceed_row_count);

    return UNITY_END();
}
