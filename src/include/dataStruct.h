#include <string>
#include <chrono>
#include <vector>
#include <optional>

using namespace std;

struct SongListen {
    string name;
    string artist;
    string album;
    string timestamp;
    string startReason;
    string endReason;

};

struct Query {
    optional<std::string> artist;
    optional<std::string> album;
    optional<std::string> start;
    optional<std::string> end;
    optional<std::string> startReason;
    optional<std::string> endReason;

};

vector<SongListen> searchSong(const Query& q);

chrono::system_clock::time_point parseTimestamp(const std::string& ts);

void parseJson(const std::string& filename, std::vector<SongListen>& songs);