#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// call these inside main of stats.c ? 
// initialize_info_panel(list_window, info_window, records_array, number_of_records)
/*
    prob have to keep struct in stats?
*/

typedef struct {
    char *artist_name;
    char *timestamp;
    long milliseconds_played;
} ListeningRecord;

typedef struct {
    char *artist_name;
    long total_milliseconds;
    long total_plays;
    time_t first_time_listened;
} ArtistSummary;

static ArtistSummary *artist_list = NULL;
static int artist_count = 0;

static display_window *list_window_ptr = NULL;
static display_window *info_window_ptr = NULL;

//helpers

static char *duplicate_string(const char *text) {
    char *copy = malloc(strlen(text) + 1);
    strcpy(copy, text);
    return copy;
}

static bool convert_timestamp_to_time(const char *timestamp, time_t *result) {

    struct tm time_info;
    memset(&time_info, 0, sizeof(time_info));

    if (strlen(timestamp) < 19)
        return false;

    time_info.tm_year = atoi(timestamp) - 1900;
    time_info.tm_mon  = atoi(timestamp + 5) - 1;
    time_info.tm_mday = atoi(timestamp + 8);
    time_info.tm_hour = atoi(timestamp + 11);
    time_info.tm_min  = atoi(timestamp + 14);
    time_info.tm_sec  = atoi(timestamp + 17);

    *result = mktime(&time_info);
    return true;
}

static void convert_milliseconds_to_hms(long milliseconds, char *output) {

    long total_seconds = milliseconds / 1000;

    long hours = total_seconds / 3600;
    long minutes = (total_seconds % 3600) / 60;
    long seconds = total_seconds % 60;

    sprintf(output, "%ld:%02ld:%02ld", hours, minutes, seconds);
}

// artist summary

static int find_artist_position(const char *artist_name) {

    for (int i = 0; i < artist_count; i++) {
        if (strcmp(artist_list[i].artist_name, artist_name) == 0)
            return i;
    }

    return -1;
}

void create_artist_summary_table(ListeningRecord *records, int record_count) {

    artist_list = malloc(sizeof(ArtistSummary) * record_count);
    artist_count = 0;

    for (int i = 0; i < record_count; i++) {

        if (records[i].artist_name == NULL)
            continue;

        int index = find_artist_position(records[i].artist_name);

        time_t listen_time;
        convert_timestamp_to_time(records[i].timestamp, &listen_time);

        if (index == -1) {

            artist_list[artist_count].artist_name =
                duplicate_string(records[i].artist_name);

            artist_list[artist_count].total_milliseconds =
                records[i].milliseconds_played;

            artist_list[artist_count].total_plays = 1;
            artist_list[artist_count].first_time_listened = listen_time;

            artist_count++;

        } else {

            artist_list[index].total_milliseconds +=
                records[i].milliseconds_played;

            artist_list[index].total_plays++;

            if (listen_time < artist_list[index].first_time_listened) {
                artist_list[index].first_time_listened = listen_time;
            }
        }
    }
}

// top 10 artists

void show_top_10_artists() {

    display_terminate_window_contents(list_window_ptr);
    display_terminate_window_contents(info_window_ptr);

    display_window_add_content_node(list_window_ptr,
        "top 10 artists");

    for (int i = 0; i < artist_count && i < 10; i++) {

        char time_text[64];
        convert_milliseconds_to_hms(
            artist_list[i].total_milliseconds,
            time_text);

        char line[256];
        sprintf(line, "%d) %s [%s]",
            i + 1,
            artist_list[i].artist_name,
            time_text);

        display_window_add_content_node(list_window_ptr, line);
    }

    display_window_add_content_node(info_window_ptr,
        "choose artist");
}

// artist information

void show_artist_details(const char *artist_name) {

    display_terminate_window_contents(info_window_ptr);

    for (int i = 0; i < artist_count; i++) {

        if (strcmp(artist_list[i].artist_name, artist_name) == 0) {

            char time_text[64];
            convert_milliseconds_to_hms(
                artist_list[i].total_milliseconds,
                time_text);

            char date_text[64];
            strftime(date_text, sizeof(date_text),
                "%Y-%m-%d",
                gmtime(&artist_list[i].first_time_listened));

            display_window_add_content_node(info_window_ptr,
                "artist summary");
        
            display_window_add_content_node(info_window_ptr,
                artist_list[i].artist_name);

            char line1[128];
            sprintf(line1, "total time listened: %s", time_text);
            display_window_add_content_node(info_window_ptr, line1);

            char line2[128];
            sprintf(line2, "total plays: %ld",
                artist_list[i].total_plays);
            display_window_add_content_node(info_window_ptr, line2);

            char line3[128];
            sprintf(line3, "first time listened: %s", date_text);
            display_window_add_content_node(info_window_ptr, line3);

            return;
        }
    }
}

//initialize the info panel with the list of artists and their details

void initialize_info_panel(
    display_window *list_window,
    display_window *info_window,
    ListeningRecord *records,
    int record_count) {

    list_window_ptr = list_window;
    info_window_ptr = info_window;

    create_artist_summary_table(records, record_count);

    show_top_10_artists();
}