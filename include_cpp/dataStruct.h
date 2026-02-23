#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#ifdef __cplusplus
  #define EXPORT_C extern "C"
#else
  #define EXPORT_C
#endif

// A LOT OF THE C WRAPPER STUFF HERE IS HEAVILY INSPIRED BY https://caiorss.github.io/C-Cpp-Notes/CwrapperToQtLibrary.html
#ifdef __cplusplus
#include <string>
#include <chrono>
#include <vector>
#include <optional>

extern "C" {
#include "display.h"
}

using namespace std;

const vector<string> VALID_START_REASONS = {
    "playbtn", "trackdone", "clickrow", "appload"
};

const vector<string> VALID_END_REASONS = {
    "trackdone", "endplay", "remote", "unexpected-exit-while-paused"
};


struct SongListen {
    string name;
    string artist;
    string album;
    string timestamp;
    string startReason;
    string endReason;

};

struct Query {
    optional<string> name;
    optional<string> artist;
    optional<string> album;
    optional<string> start;
    optional<string> end;
    optional<string> startReason;
    optional<string> endReason;

};

vector<SongListen> searchSong(const Query& q, const vector<SongListen>& songs);

Query getSortQuery(const string& input);

void parseJson(const string& filename, vector<SongListen>& songs);

string getInput();

#endif // end of CPP

EXPORT_C void wrap_parseJson(const char* filename);
EXPORT_C void wrap_get_strings(display_window* window);
EXPORT_C void wrap_search_command(char* command, display_window* window);

#endif
