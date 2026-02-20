#include "dataStruct.h"
#include <fstream>
#include <iostream>
#include <optional>
#include <chrono>
#include <string>
#include <sstream>
#include "json.hpp" //json tools library
using namespace std;

//vector<SongListen> songs;

vector<SongListen> search(const Query& q, const vector<SongListen>& songs) {
    vector<SongListen> results;

    for (const auto& s : songs) {
        if (q.artist && s.artist != *q.artist) continue;
        if (q.album && s.album != *q.album) continue;
        if (q.start && s.timestamp < *q.start) continue;
        if (q.end && s.timestamp > *q.end) continue;

        results.push_back(s);
    }
    return results;
}

void parseJson(const string& filename, vector<SongListen>& songs) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Could not open JSON file: " + filename);
    }

    nlohmann::json j;
    file >> j;

    for (const auto& item : j) {
        SongListen s;

        s.name   = item.value("master_metadata_track_name", "");
        s.artist = item.value("master_metadata_album_artist_name", "");
        s.album  = item.value("master_metadata_album_album_name", "");

        string ts = item.value("ts", "");
        s.timestamp = parseTimestamp(ts);

        songs.push_back(move(s));
    }
}

chrono::system_clock::time_point parseTimestamp(const string& ts) {
    tm tm = {};
    istringstream ss(ts);
    ss >> get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    cout<<ss.str()<<"\n";
    auto time_c = timegm(&tm);
    return chrono::system_clock::from_time_t(time_c);
}
