#include "include/dataStruct.h"
#include <fstream>
#include <iostream>
#include <optional>
#include <chrono>
#include <string>
#include <sstream>
#include <algorithm>
#include <regex>
#include "include/json.hpp" //json tools library
using namespace std;

bool contains(const vector<string>& arr, const string& value) {
    return find(arr.begin(), arr.end(), value) != arr.end();
}

Query getSortQuery() {
    Query q;
    string inputs [NumFIELDS];
    
    string temp;
    for (int i = 0; i < NumFIELDS; i++) {
        cout << prompts[i];
        getline(cin, temp);

        //Error checking for inputs.
        while (!validInput(prompts[i], temp)) {
            getline(cin, temp);
        }

        inputs[i] = temp;
    }
    
    if (inputs[0] != "") q.name        = inputs[0];
    if (inputs[1] != "") q.artist      = inputs[1];
    if (inputs[2] != "") q.album       = inputs[2];
    if (inputs[3] != "") q.start       = inputs[3];
    if (inputs[4] != "") q.end         = inputs[4];
    if (inputs[5] != "") q.startReason = inputs[5];
    if (inputs[6] != "") q.endReason   = inputs[6];

    return q;
}

bool validInput(string prompt, string input) {
    if (input == "") { return true; }
    if (prompt == "Start date: " || prompt == "End date: ") {
        static const regex iso8601_extended_regex(R"(^(?:[0-9]{4})-(?:0[1-9]|1[0-2])-(?:0[1-9]|[12][0-9]|3[01])T(?:[01][0-9]|2[0-3]):(?:[0-5][0-9]):(?:[0-5][0-9])Z$)");
        if (!regex_match(input, iso8601_extended_regex)) {
            cout << "Invalid date format. Please use 'YYYY-MM-DDTHH:MM:SSZ': ";
            return false;
        }
        else return true;
    }

    else if (prompt == "Start reason: ") {
        if (!contains(VALID_START_REASONS, input)) {
            cout << "Invalid start reason. Please try again: ";
            return false;
        }
        return true;
    }

    else if (prompt == "End reason: ") {
        if (!contains(VALID_END_REASONS, input)) {
            cout << "Invalid start reason. Please try again: ";
            return false;
        }
        return true;
    }

    else return true;
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

        songs.push_back(move(s));
    }
}

vector<SongListen> searchSong(const Query& q, const vector<SongListen>& songs) {
    vector<SongListen> results;

    for (const auto& s : songs) {

        if (q.name        && s.name        != *q.name       ) continue;
        if (q.artist      && s.artist      != *q.artist     ) continue;
        if (q.album       && s.album       != *q.album      ) continue;
        if (q.start       && s.timestamp    < *q.start      ) continue;
        if (q.end         && s.timestamp    > *q.end        ) continue;
        if (q.startReason && s.startReason != *q.startReason) continue;
        if (q.endReason   && s.endReason   != *q.endReason  ) continue;

        results.push_back(s);
    }

    return results;
}
