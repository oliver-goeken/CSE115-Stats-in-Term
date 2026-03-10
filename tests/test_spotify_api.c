#include "unity.h"
#include "parse_db_funcs.h"
#include "spotify_api.h"

#include <sqlite3.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_spotify_track_uri_get_id_accepts_valid_uri(void)
{
    char track_id[64];

    TEST_ASSERT_TRUE(spotify_track_uri_get_id("spotify:track:2Qm79stPtbB3SGjrY4ZN6M", track_id, (int)sizeof(track_id)));
    TEST_ASSERT_EQUAL_STRING("2Qm79stPtbB3SGjrY4ZN6M", track_id);
}

void test_spotify_track_uri_get_id_rejects_invalid_uri(void)
{
    char track_id[64];

    TEST_ASSERT_FALSE(spotify_track_uri_get_id("spotify:album:2Qm79stPtbB3SGjrY4ZN6M", track_id, (int)sizeof(track_id)));
}

void test_get_track_uri_for_song_returns_stored_uri(void)
{
    sqlite3* db = NULL;
    TEST_ASSERT_EQUAL_INT(SQLITE_OK, sqlite3_open(":memory:", &db));
    TEST_ASSERT_EQUAL_INT(0, create_db(db));

    const char* insert_sql =
        "INSERT INTO spotifyHistory (artist, track, album, ms_played, timestamp, track_uri) "
        "VALUES ('Switchfoot', 'Meant to Live', 'The Beautiful Letdown', 120000, '2026-03-01T12:00:00', 'spotify:track:2Qm79stPtbB3SGjrY4ZN6M');";

    TEST_ASSERT_EQUAL_INT(SQLITE_OK, sqlite3_exec(db, insert_sql, NULL, NULL, NULL));

    char* track_uri = get_track_uri_for_song(db, "Meant to Live", "The Beautiful Letdown", "Switchfoot");
    TEST_ASSERT_NOT_NULL(track_uri);
    TEST_ASSERT_EQUAL_STRING("spotify:track:2Qm79stPtbB3SGjrY4ZN6M", track_uri);

    free(track_uri);
    sqlite3_close(db);
}

void test_get_track_uri_for_song_returns_null_when_missing(void)
{
    sqlite3* db = NULL;
    TEST_ASSERT_EQUAL_INT(SQLITE_OK, sqlite3_open(":memory:", &db));
    TEST_ASSERT_EQUAL_INT(0, create_db(db));

    char* track_uri = get_track_uri_for_song(db, "Missing", "Missing", "Missing");
    TEST_ASSERT_NULL(track_uri);

    sqlite3_close(db);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_spotify_track_uri_get_id_accepts_valid_uri);
    RUN_TEST(test_spotify_track_uri_get_id_rejects_invalid_uri);
    RUN_TEST(test_get_track_uri_for_song_returns_stored_uri);
    RUN_TEST(test_get_track_uri_for_song_returns_null_when_missing);

    return UNITY_END();
}
