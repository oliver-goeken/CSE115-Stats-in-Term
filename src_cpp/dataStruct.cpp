#include "dataStruct.h"
#include <fstream>
#include <iostream>
#include <optional>
#include <chrono>
#include <string>
#include <sstream>
#include <algorithm>
#include "json.hpp" //json tools library
					
using namespace std;

const int NumFIELDS = 7;

vector<SongListen> allSongs;

bool contains(const vector<string>& arr, const string& value) {
    return find(arr.begin(), arr.end(), value) != arr.end();
}

Query getSortQuery(const string& input) {
    Query q;

    vector<string> tokens;
    string token;
    stringstream ss(input);

    // Split input using commas
    while (getline(ss, token, ',')) {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \t\n\r"));
        token.erase(token.find_last_not_of(" \t\n\r") + 1);
        tokens.push_back(token);
    }

    // Ensure at least NumFIELDS fields
    while (tokens.size() < NumFIELDS) {
        tokens.push_back("");
    }

    // Populate Query fields if non-empty
    if (!tokens[0].empty()) q.name   = tokens[0];
    if (!tokens[1].empty()) q.artist = tokens[1];
    if (!tokens[2].empty()) q.album  = tokens[2];
    if (!tokens[3].empty()) q.start  = tokens[3];
    if (!tokens[4].empty()) q.end    = tokens[4];

    // Validate startReason
    if (!tokens[5].empty()) {
        if (contains(VALID_START_REASONS, tokens[5])) {
            q.startReason = tokens[5];
        }
        else {
            cerr << "Warning: Invalid startReason '" << tokens[5] << "' ignored.\n";
        }
    }

    // Validate endReason
    if (!tokens[6].empty()) {
        if (contains(VALID_END_REASONS, tokens[6])) {
            q.endReason = tokens[6];
        }
        else {
            cerr << "Warning: Invalid endReason '" << tokens[6] << "' ignored.\n";
        }
    }

    return q;
}

void parseJson(const string& filename, vector<SongListen>& songs) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Could not open JSON\n";
        throw runtime_error("Could not open JSON file: " + filename);
    }
    nlohmann::json j;
    file >> j;

    for (const auto& item : j) {
        SongListen s;
        if (item["master_metadata_track_name"].is_null()) continue;
        s.name        = item.value("master_metadata_track_name", "");
        s.artist      = item.value("master_metadata_album_artist_name", "");
        s.album       = item.value("master_metadata_album_album_name", "");
        s.timestamp   = item.value("ts","");
        s.startReason = item.value("reason_start", "");
        s.endReason   = item.value("reason_end", "");

        songs.push_back(std::move(s));
    }
}

EXPORT_C void wrap_parseJson(const char* filename){
	string cpp_filename = filename;
	parseJson(cpp_filename, allSongs);
}

EXPORT_C void wrap_get_strings(display_window* window){
	for (const auto& song : allSongs){
		string song_data = song.name + " - " + song.artist;

		char* c_song_data = song_data.data();
		display_window_add_content_node(window, c_song_data);
	}
}

vector<SongListen> searchSong(const Query& q, const vector<SongListen>& songs) {
    vector<SongListen> results;

    for (const auto& s : songs) {

        if (q.name   && s.name     != *q.name)        continue;
        if (q.artist && s.artist   != *q.artist)      continue;
        if (q.album  && s.album    != *q.album)       continue;
        if (q.start  && s.timestamp < *q.start)       continue;
        if (q.end    && s.timestamp > *q.end)         continue;
        if (q.startReason && s.startReason != *q.startReason) continue;
        if (q.endReason   && s.endReason   != *q.endReason)   continue;

        results.push_back(s);
    }

    return results;
}

string getInput() {
    string name, artist, album, start, end, startReason, endReason;

    cout << "Song name: ";
    getline(cin, name);

    cout << "Artist: ";
    getline(cin, artist);

    cout << "Album: ";
    getline(cin, album);

    cout << "Start date: ";
    getline(cin, start);

    cout << "End date: ";
    getline(cin, end);

    cout << "Start reason: ";
    getline(cin, startReason);

    cout << "End reason: ";
    getline(cin, endReason);

    string combined = name + "," + artist + "," + album + "," +
        start + "," + end + "," + startReason + "," + endReason;

    return combined;
}
