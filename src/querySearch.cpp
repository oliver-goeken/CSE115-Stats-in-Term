#include "include/dataStruct.h"
#include <fstream>
#include <iostream>
#include <optional>
#include <chrono>
#include <string>
#include <sstream>
#include "include/json.hpp" //json tools library
using namespace std;

//vector<SongListen> songs;

vector<SongListen> searchSong(const Query& q, const vector<SongListen>& songs) {
    vector<SongListen> results;

    for (const auto& s : songs) {
        if (q.artist && s.artist != *q.artist) continue;
        if (q.album && s.album != *q.album)    continue;
        if (q.start && s.timestamp < *q.start) continue;
        if (q.end && s.timestamp > *q.end)     continue;

        results.push_back(s);
    }
    return results;
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

        s.name   = item.value("master_metadata_track_name", "");
        s.artist = item.value("master_metadata_album_artist_name", "");
        s.album  = item.value("master_metadata_album_album_name", "");
        s.timestamp = item.value("ts","");
        s.startReason = item.value("reason_start", "");
        s.endReason = item.value("reason_end", "");

        songs.push_back(move(s));
    }
}


