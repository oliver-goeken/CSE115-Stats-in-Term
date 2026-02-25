#include "dataStruct.h"
#include <fstream>
#include <iostream>
#include <optional>
#include <chrono>
#include <string>
#include <sstream>
#include <algorithm>
#include <regex>
#include <cstring>
#include "json.hpp" //json tools library
using namespace std;

// Case-insensitive strstr
const char* strcasestr_c(const char* haystack, const char* needle) {
    if (!*needle) return haystack;

    for (; *haystack; ++haystack) {
        const char* h = haystack;
        const char* n = needle;

        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
            ++h;
            ++n;
        }

        if (!*n) return haystack; // found match
    }

    return nullptr;
}


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
    
    q.name        = inputs[0];
    q.artist      = inputs[1];
    q.album       = inputs[2];
    q.start       = inputs[3];
    q.end         = inputs[4];
    q.startReason = inputs[5];
    q.endReason   = inputs[6];

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

void parseJson(const string& filename, vector<songListen>& songs) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Could not open JSON\n";
        throw runtime_error("Could not open JSON file: " + filename);
    }
    nlohmann::json j;
    file >> j;

    for (const auto& item : j) {
        songListen s;
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

vector<songListen> searchSong(const Query& q, const vector<songListen>& songs) {
    vector<songListen> results;

    for (const auto& s : songs) {

        if (!q.name  .empty() && strcasestr_c(s.name  .c_str(), q.name  .c_str())==nullptr) continue;
        if (!q.artist.empty() && strcasestr_c(s.artist.c_str(), q.artist.c_str())==nullptr) continue;
        if (!q.album .empty() && strcasestr_c(s.album .c_str(), q.album .c_str())==nullptr) continue;
        if (!q.start .empty() && s.timestamp         < q.start      ) continue;
        if (!q.end   .empty() && s.timestamp         > q.end        ) continue;
        if (!q.startReason.empty() && s.startReason != q.startReason) continue;
        if (!q.endReason  .empty() && s.endReason   != q.endReason  ) continue;

        results.push_back(s);
    }

    return results;
}

vector<songListen> all_songs;

EXPORT_C void load_all_song_listens(char* filename){
	parseJson(filename, all_songs);
}

EXPORT_C int display_window_add_all_song_listens(display_window* window){
	if (window->content != NULL){
		return -1;
	}

	if (all_songs.empty()){
		return -2;
	}

	window->content = vector_to_content_nodes(all_songs);

	return 0;
}

Query* get_query_from_command(char* command){
	Query* q = new Query;
	q->name = "";
	q->album = "";
	q->artist = "";
	q->end = "";
	q->endReason = "";
	q->start = "";
	q->startReason = "";

	char search_field[256] = {'\0'};
	char search_token[256] = {'\0'};

	int command_part = 0;
	int part_position = 0;

	for (int i = 0; i < 256 && command[i] != '\0'; i ++){
		if (command_part == 0 && command[i] == ' '){
			command_part = 1;
			part_position = 0;
			continue;
		}

		if (command_part == 0){
			search_field[part_position] = command[i];
		} else {
			search_token[part_position] = command[i];
		}

		part_position ++;
	}

	string str_search_field = search_field;
	string str_search_token = search_token;

	if (str_search_field == "song"){
		q->name = str_search_token;
	} else if (str_search_field == "album"){
		q->album = str_search_token;
	} else if (str_search_field == "artist"){
		q->artist = str_search_token;
	}

	return q;
}

display_window_content_node* vector_to_content_nodes(const vector<songListen>& songs){
	if (songs.empty()){
		return NULL;
	}

	display_window_content_node* initial = NULL;
	display_window_content_node* prev = NULL;

	for (const auto& song : songs){
		if (prev == NULL){
			initial = display_add_content_node(NULL);
			prev = initial;

			initial->selected = true;
		} else {
			prev = display_add_content_node(prev);
		}

		string string_data = song.name + " - " + song.artist;
		string new_name = song.name;
		string new_album = song.album;
		string new_artist = song.artist;
		string new_timestamp = song.timestamp;

		display_content_node_set_data(prev, string_data.data());
		prev->song_listen = display_new_song_listen(new_name.data(), new_album.data(), new_artist.data(), new_timestamp.data());
	}

	return initial;
}

EXPORT_C display_window_content_node* search_songs(char* search_query){
	Query* query = get_query_from_command(search_query);
	vector<songListen> search_results = searchSong(*query, all_songs);

	delete query;

	return vector_to_content_nodes(search_results);
}
