#include "panel.h"
#include "display.h"
#include "parse_db_funcs.h"
#include "spotify_api.h"
#include "sqlite3.h"
#include "shared_defs.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static sqlite3* g_database = NULL;
static display_window* g_list_window = NULL;
static display_window* g_info_window = NULL;

//string helpers

static void safe_copy(char* dst, int dst_size, const char* src) {
    if (dst == NULL || dst_size <= 0) {
        return;
    }
    if (src == NULL) {
        dst[0] = '\0';
        return;
    }
    snprintf(dst, dst_size, "%s", src);
}

static void trim_whitespace_in_place(char* text) {
    if (text == NULL) return;

    int start = 0;
    while (text[start] != '\0' && isspace((unsigned char)text[start])) {
        start++;
    }

    if (start > 0) {
        memmove(text, text + start, strlen(text + start) + 1);
    }

    int end = (int)strlen(text) - 1;
    while (end >= 0 && isspace((unsigned char)text[end])) {
        text[end] = '\0';
        end--;
    }
}

static void format_ms_as_hms(long long ms, char* out, int out_size) {
    long long total_seconds = ms / 1000;
    long long hours = total_seconds / 3600;
    long long minutes = (total_seconds % 3600) / 60;
    long long seconds = total_seconds % 60;

    snprintf(out, out_size, "%lld:%02lld:%02lld", hours, minutes, seconds);
}

static bool parse_artist_from_list_line(const char* line, char* out_artist, int out_size) {
    if (line == NULL || out_artist == NULL || out_size <= 0) {
        return false;
    }
    out_artist[0] = '\0';

    char buffer[512];
    safe_copy(buffer, (int)sizeof(buffer), line);
    trim_whitespace_in_place(buffer);

    if (buffer[0] == '\0') {
        return false;
    }

    if (buffer[0] == '[') {
        const char* last_sep = NULL;
        const char* p = buffer;

        while ((p = strstr(p, " - ")) != NULL) {
            last_sep = p;
            p += 3;
        }

        if (last_sep == NULL) {
            return false;
        }

        safe_copy(out_artist, out_size, last_sep + 3);
        trim_whitespace_in_place(out_artist);
        return (out_artist[0] != '\0');
    }

    const char* after_number = buffer;
    while (isdigit((unsigned char)*after_number)) after_number++;
    if (*after_number == '.' && after_number[1] == ' ') {
        after_number += 2; 
    } else {
        after_number = buffer;
    }

    char temp[512];
    safe_copy(temp, (int)sizeof(temp), after_number);

    char* bracket = strstr(temp, " - [");
    if (bracket != NULL) {
        *bracket = '\0';
    }
    trim_whitespace_in_place(temp);

    char* last_sep = NULL;
    char* search = temp;
    while ((search = strstr(search, " - ")) != NULL) {
        last_sep = search;
        search += 3;
    }

    if (last_sep == NULL) {
        safe_copy(out_artist, out_size, temp);
        trim_whitespace_in_place(out_artist);
        return (out_artist[0] != '\0');
    }

    safe_copy(out_artist, out_size, last_sep + 3);
    trim_whitespace_in_place(out_artist);
    return (out_artist[0] != '\0');
}

static bool parse_song_from_list_line(
    const char* line,
    char* out_track,
    int out_track_size,
    char* out_album,
    int out_album_size,
    char* out_artist,
    int out_artist_size
) {
    if (line == NULL || out_track == NULL || out_album == NULL || out_artist == NULL) {
        return false;
    }

    out_track[0] = '\0';
    out_album[0] = '\0';
    out_artist[0] = '\0';

    char buffer[512];
    safe_copy(buffer, (int)sizeof(buffer), line);
    trim_whitespace_in_place(buffer);

    if (buffer[0] == '\0') {
        return false;
    }

    char* working = buffer;
    if (working[0] == '[') {
        char* closing = strstr(working, "] ");
        if (closing == NULL) {
            return false;
        }
        working = closing + 2;
    } else {
        while (isdigit((unsigned char)*working)) {
            working++;
        }
        if (*working == '.' && working[1] == ' ') {
            working += 2;
        }
    }

    char temp[512];
    safe_copy(temp, (int)sizeof(temp), working);

    char* plays_marker = strstr(temp, " - [");
    if (plays_marker != NULL) {
        *plays_marker = '\0';
    }

    char* first_sep = strstr(temp, " - ");
    if (first_sep == NULL) {
        return false;
    }
    *first_sep = '\0';

    char* second_part = first_sep + 3;
    char* second_sep = strstr(second_part, " - ");
    if (second_sep == NULL) {
        return false;
    }
    *second_sep = '\0';

    safe_copy(out_track, out_track_size, temp);
    safe_copy(out_album, out_album_size, second_part);
    safe_copy(out_artist, out_artist_size, second_sep + 3);

    trim_whitespace_in_place(out_track);
    trim_whitespace_in_place(out_album);
    trim_whitespace_in_place(out_artist);

    return out_track[0] != '\0' && out_album[0] != '\0' && out_artist[0] != '\0';
}

static bool query_artist_summary(
    sqlite3* db,
    const char* artist,
    long long* out_total_plays,
    long long* out_total_ms,
    char* out_first_ts,
    int out_first_ts_size
) {
    if (db == NULL || artist == NULL || artist[0] == '\0') {
        return false;
    }

    if (out_total_plays) *out_total_plays = 0;
    if (out_total_ms) *out_total_ms = 0;
    if (out_first_ts && out_first_ts_size > 0) out_first_ts[0] = '\0';

    const char* sql =
        "SELECT COUNT(*) AS plays, "
        "       COALESCE(SUM(ms_played), 0) AS total_ms, "
        "       COALESCE(MIN(timestamp), '') AS first_ts "
        "FROM spotifyHistory "
        "WHERE artist = ?;";

    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, artist, -1, SQLITE_TRANSIENT);

    bool ok = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        if (out_total_plays) *out_total_plays = sqlite3_column_int64(stmt, 0);
        if (out_total_ms) *out_total_ms = sqlite3_column_int64(stmt, 1);

        const unsigned char* first_ts = sqlite3_column_text(stmt, 2);
        if (out_first_ts && out_first_ts_size > 0) {
            safe_copy(out_first_ts, out_first_ts_size, (const char*)first_ts);
        }
        ok = true;
    }

    sqlite3_finalize(stmt);
    return ok;
}

static int query_top_songs_for_artist(
    sqlite3* db,
    const char* artist,
    char song_names[][256],
    long long song_plays[],
    int max_songs
) {
    if (db == NULL || artist == NULL || artist[0] == '\0') {
        return 0;
    }

    const char* sql =
        "SELECT track, COUNT(*) AS plays "
        "FROM spotifyHistory "
        "WHERE artist = ? "
        "GROUP BY track "
        "ORDER BY plays DESC, track ASC "
        "LIMIT ?;";

    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, artist, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, max_songs);

    int count = 0;
    while (count < max_songs && sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* track = sqlite3_column_text(stmt, 0);
        long long plays = sqlite3_column_int64(stmt, 1);

        safe_copy(song_names[count], 256, (const char*)track);
        song_plays[count] = plays;

        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

// info panel drawing

static void draw_info_default(void) {
    if (g_info_window == NULL) return;

    display_window_destroy_content_nodes(g_info_window);
    display_new_text_content_node(g_info_window, "Select item on left for details");
    display_new_text_content_node(g_info_window, "--------------------------------");
    //display_new_text_content_node(g_info_window, "Then details will appear");
}

static void draw_artist_info(const char* artist) {
    if (g_info_window == NULL || g_database == NULL) return;

    display_window_destroy_content_nodes(g_info_window);

    char title[300];
    snprintf(title, sizeof(title), "Artist: %s", artist);
    display_new_text_content_node(g_info_window, title);

    //info stuff
    long long total_plays = 0;
    long long total_ms = 0;
    char first_ts[64];
    first_ts[0] = '\0';

    if (!query_artist_summary(g_database, artist, &total_plays, &total_ms, first_ts, (int)sizeof(first_ts))) {
        display_new_text_content_node(g_info_window, "Could not query artist summary.");
        return;
    }

    char total_time_text[64];
    format_ms_as_hms(total_ms, total_time_text, (int)sizeof(total_time_text));

    char plays_line[128];
    snprintf(plays_line, sizeof(plays_line), "Total plays: %lld", total_plays);
    display_new_text_content_node(g_info_window, plays_line);

    char time_line[128];
    snprintf(time_line, sizeof(time_line), "Total listening time: %s", total_time_text);
    display_new_text_content_node(g_info_window, time_line);

    // first_ts looks like "2026-02-05T07:27:51Z"
    char first_line[160];
    if (first_ts[0] == '\0') {
        snprintf(first_line, sizeof(first_line), "First time listened: (unknown)");
    } else {
        snprintf(first_line, sizeof(first_line), "First time listened: %s", first_ts);
    }
    display_new_text_content_node(g_info_window, first_line);

    // Top songs
    display_new_text_content_node(g_info_window, "");
    display_new_text_content_node(g_info_window, "Top songs:");
    //display_new_text_content_node(g_info_window, "---------");

    char top_song_names[5][256];
    long long top_song_plays[5];
    int top_count = query_top_songs_for_artist(g_database, artist, top_song_names, top_song_plays, 5);

    if (top_count <= 0) {
        display_new_text_content_node(g_info_window, "(No songs found for this artist)");
        return;
    }

    for (int i = 0; i < top_count; i++) {
        char line[320];
        snprintf(line, sizeof(line), "%d. %s  [%lld plays]", i + 1, top_song_names[i], top_song_plays[i]);
        display_new_text_content_node(g_info_window, line);
    }
}

static void draw_song_info(const char* track, const char* album, const char* artist) {
    if (g_info_window == NULL || g_database == NULL) return;

    display_window_destroy_content_nodes(g_info_window);

    char title[300];
    snprintf(title, sizeof(title), "Song: %s", track);
    display_new_text_content_node(g_info_window, title);

    char album_line[320];
    snprintf(album_line, sizeof(album_line), "Album: %s", album);
    display_new_text_content_node(g_info_window, album_line);

    char artist_line[320];
    snprintf(artist_line, sizeof(artist_line), "Artist: %s", artist);
    display_new_text_content_node(g_info_window, artist_line);

    char* track_uri = get_track_uri_for_song(g_database, track, album, artist);
    if (track_uri == NULL) {
        display_new_text_content_node(g_info_window, "Spotify metadata: no stored track URI");
        return;
    }

    spotify_track_metadata metadata;
    int rc = spotify_api_fetch_track_metadata(track_uri, &metadata);
    free(track_uri);

    if (rc == SPOTIFY_API_ERR_NO_TOKEN) {
        //display_new_text_content_node(g_info_window, "Spotify metadata: set SPOTIFY_ACCESS_TOKEN");
        //display_new_text_content_node(g_info_window, "");
        return;
    }

    if (rc != SPOTIFY_API_OK) {
        display_new_text_content_node(g_info_window, "Spotify metadata: lookup failed");
        return;
    }

    display_new_text_content_node(g_info_window, "");
    display_new_text_content_node(g_info_window, "Spotify metadata:");

    if (metadata.track_name != NULL) {
        char name_line[320];
        snprintf(name_line, sizeof(name_line), "Name: %s", metadata.track_name);
        display_new_text_content_node(g_info_window, name_line);
    }

    if (metadata.album_name != NULL) {
        char spotify_album_line[320];
        snprintf(spotify_album_line, sizeof(spotify_album_line), "Album: %s", metadata.album_name);
        display_new_text_content_node(g_info_window, spotify_album_line);
    }

    if (metadata.artist_name != NULL) {
        char spotify_artist_line[320];
        snprintf(spotify_artist_line, sizeof(spotify_artist_line), "Artist: %s", metadata.artist_name);
        display_new_text_content_node(g_info_window, spotify_artist_line);
    }

    if (metadata.duration_ms > 0) {
        char duration_text[64];
        format_ms_as_hms(metadata.duration_ms, duration_text, (int)sizeof(duration_text));

        char duration_line[128];
        snprintf(duration_line, sizeof(duration_line), "Duration: %s", duration_text);
        display_new_text_content_node(g_info_window, duration_line);
    }

    spotify_track_metadata_free(&metadata);
}

void panel_init(sqlite3* database, display_window* list_window, display_window* info_window) {
    g_database = database;
    g_list_window = list_window;
    g_info_window = info_window;

    draw_info_default();
}

void panel_on_selection_changed(void) {
    if (g_list_window == NULL || g_info_window == NULL) {
        return;
    }

	if (display_screen_get_selected_window_node(display_get_current_screen())->display_window == g_info_window){
		return;
	} else if (display_screen_get_selected_window_node(display_get_current_screen())->display_window != LIST_WINDOW){
		display_window_set_visibility(g_info_window, WINDOW_HIDDEN);
		return;
	} else {
		display_window_set_visibility(g_info_window, WINDOW_VISIBLE);
	}

    display_content_node* selected_node = display_get_selected_content_node();
    if (selected_node == NULL || selected_node->data == NULL || selected_node->data->text_data == NULL) {
        draw_info_default();
        return;
    }

    const char* selected_text = selected_node->data->text_data;

    char track[256];
    char album[256];
    char artist[256];
    if (parse_song_from_list_line(selected_text, track, (int)sizeof(track), album, (int)sizeof(album), artist, (int)sizeof(artist))) {
        draw_song_info(track, album, artist);
        return;
    }

    if (!parse_artist_from_list_line(selected_text, artist, (int)sizeof(artist))) {
        draw_info_default();
        return;
    }

    draw_artist_info(artist);
}
