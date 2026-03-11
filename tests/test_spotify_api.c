#include "unity.h"
#include "parse_db_funcs.h"
#include "spotify_api.h"

#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>

void setUp(void)
{
    unsetenv("SPOTIFY_ACCESS_TOKEN");
}

void tearDown(void)
{
    unsetenv("SPOTIFY_ACCESS_TOKEN");
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

void test_spotify_track_uri_get_id_rejects_empty_track_id(void)
{
    char track_id[64];

    TEST_ASSERT_FALSE(spotify_track_uri_get_id("spotify:track:", track_id, (int)sizeof(track_id)));
}

void test_spotify_track_uri_get_id_rejects_invalid_characters(void)
{
    char track_id[64];

    TEST_ASSERT_FALSE(spotify_track_uri_get_id("spotify:track:bad$id", track_id, (int)sizeof(track_id)));
}

void test_spotify_track_uri_get_id_rejects_too_small_buffer(void)
{
    char track_id[4];

    TEST_ASSERT_FALSE(spotify_track_uri_get_id("spotify:track:2Qm79stPtbB3SGjrY4ZN6M", track_id, (int)sizeof(track_id)));
}

void test_spotify_api_get_access_token_reads_environment_variable(void)
{
    TEST_ASSERT_EQUAL_INT(0, setenv("SPOTIFY_ACCESS_TOKEN", "token-123", 1));
    TEST_ASSERT_EQUAL_STRING("token-123", spotify_api_get_access_token());
}

void test_spotify_api_fetch_track_metadata_requires_token(void)
{
    spotify_track_metadata metadata;
    memset(&metadata, 0, sizeof(metadata));

    TEST_ASSERT_EQUAL_INT(
        SPOTIFY_API_ERR_NO_TOKEN,
        spotify_api_fetch_track_metadata("spotify:track:2Qm79stPtbB3SGjrY4ZN6M", &metadata)
    );
}

void test_spotify_api_fetch_track_metadata_with_token_rejects_invalid_arguments(void)
{
    spotify_track_metadata metadata;
    memset(&metadata, 0, sizeof(metadata));

    TEST_ASSERT_EQUAL_INT(
        SPOTIFY_API_ERR_INVALID_ARG,
        spotify_api_fetch_track_metadata_with_token(NULL, "spotify:track:2Qm79stPtbB3SGjrY4ZN6M", &metadata)
    );
    TEST_ASSERT_EQUAL_INT(
        SPOTIFY_API_ERR_INVALID_ARG,
        spotify_api_fetch_track_metadata_with_token("token", NULL, &metadata)
    );
    TEST_ASSERT_EQUAL_INT(
        SPOTIFY_API_ERR_INVALID_ARG,
        spotify_api_fetch_track_metadata_with_token("token", "spotify:track:2Qm79stPtbB3SGjrY4ZN6M", NULL)
    );
}

void test_spotify_api_fetch_track_metadata_with_token_rejects_invalid_uri(void)
{
    spotify_track_metadata metadata;
    memset(&metadata, 0, sizeof(metadata));

    TEST_ASSERT_EQUAL_INT(
        SPOTIFY_API_ERR_INVALID_URI,
        spotify_api_fetch_track_metadata_with_token("token", "spotify:album:2Qm79stPtbB3SGjrY4ZN6M", &metadata)
    );
}

void test_spotify_track_metadata_free_clears_allocated_fields(void)
{
    spotify_track_metadata metadata = {
        .track_id = strdup("id"),
        .track_name = strdup("name"),
        .album_name = strdup("album"),
        .artist_name = strdup("artist"),
        .track_uri = strdup("spotify:track:id"),
        .preview_url = strdup("https://example.test/preview"),
        .duration_ms = 123
    };

    spotify_track_metadata_free(&metadata);

    TEST_ASSERT_NULL(metadata.track_id);
    TEST_ASSERT_NULL(metadata.track_name);
    TEST_ASSERT_NULL(metadata.album_name);
    TEST_ASSERT_NULL(metadata.artist_name);
    TEST_ASSERT_NULL(metadata.track_uri);
    TEST_ASSERT_NULL(metadata.preview_url);
    TEST_ASSERT_EQUAL_INT(0, metadata.duration_ms);
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
    RUN_TEST(test_spotify_track_uri_get_id_rejects_empty_track_id);
    RUN_TEST(test_spotify_track_uri_get_id_rejects_invalid_characters);
    RUN_TEST(test_spotify_track_uri_get_id_rejects_too_small_buffer);
    RUN_TEST(test_spotify_api_get_access_token_reads_environment_variable);
    RUN_TEST(test_spotify_api_fetch_track_metadata_requires_token);
    RUN_TEST(test_spotify_api_fetch_track_metadata_with_token_rejects_invalid_arguments);
    RUN_TEST(test_spotify_api_fetch_track_metadata_with_token_rejects_invalid_uri);
    RUN_TEST(test_spotify_track_metadata_free_clears_allocated_fields);
    RUN_TEST(test_get_track_uri_for_song_returns_stored_uri);
    RUN_TEST(test_get_track_uri_for_song_returns_null_when_missing);

    return UNITY_END();
}
