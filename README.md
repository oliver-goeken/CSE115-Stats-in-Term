# spotistats

Terminal-based Spotify listening stats viewer written in C.

The project imports Spotify extended streaming history JSON into SQLite, provides a small ncurses TUI for browsing listening data, and exposes report/search commands from the command line.

## Features

- Import Spotify extended streaming history JSON into a local SQLite database
- Launch a terminal UI to browse artists and song info
- Print recent listening history from the command line
- Print top or bottom artists, albums, and songs from the command line
- Search for the closest matching artist name and print that artist's top songs or albums
- Show Spotify track metadata in the info panel when a stored `spotify:track:{id}` URI is available

## Project Layout

- `src/`: application code
- `include/`: public headers for app modules
- `lib/`: vendored dependencies (`sqlite3`, `cJSON`)
- `tests/`: Unity-based tests
- `data/`: sample Spotify export files
- `examples/`: example json files to parse
- `doc/release`: release documents for product release
- `doc/scrum`: scrum documents for product release

## Requirements

- `clang`
- `make`
- `ncurses`
- `curl`

Optional for Spotify metadata in the info panel:

- `SPOTIFY_ACCESS_TOKEN` environment variable set to a valid Spotify Web API bearer token

## Building

Build the main binary:

```bash
make
```

This produces:

```bash
./stats
```

Clean build outputs:

```bash
make clean
```

Remove build outputs, log file, and the default database file:

```bash
make full-clean
```

## Running

Default behavior launches the TUI:

```bash
./stats
```

Use a custom database:

```bash
./stats --db temp.db
```

Import Spotify history JSON before launching:

```bash
./stats --json data/Streaming_History_Audio_2024-2026_8.json
```

Import into a custom database:

```bash
./stats --db temp.db --json data/Streaming_History_Audio_2024-2026_8.json
```

See the built-in help:

```bash
./stats --help
```

## Command Line Reports

Recent listening history:

```bash
./stats -h 10
./stats --history 10
```

Top and bottom artists:

```bash
./stats -r 10
./stats --artists 10
./stats -R 10
./stats --artists-bottom 10
```

Top and bottom albums:

```bash
./stats -a 10
./stats --albums 10
./stats -A 10
./stats --albums-bottom 10
```

Top and bottom songs:

```bash
./stats -s 10
./stats --songs 10
./stats -S 10
./stats --songs-bottom 10
```

These report modes are mutually exclusive. Pick one per invocation.

## Search

The MVP search path resolves the closest matching artist name, then prints either that artist's top songs or top albums.

Examples:

```bash
./stats --search songs 10 "foo"
./stats --search albums 5 "foo"
./stats --find songs 10 "foo"
./stats -f s 10 "foo"
./stats -f a 5 "foo"
```

Behavior:

- `songs` or `s` prints top songs for the best matching artist
- `albums` or `a` prints top albums for the best matching artist
- the query match is case-insensitive
- if no artist matches, the command exits with an error message

## TUI Notes

- Running `./stats` launches the ncurses interface
- the info panel updates as the current selection changes
- selecting a song row shows basic song details from the local database
- if the selected song has a stored `track_uri`, the app also attempts to fetch Spotify track metadata

If `SPOTIFY_ACCESS_TOKEN` is not set, the info panel shows a fallback message instead of Spotify metadata.

## Spotify Metadata

Spotify metadata lookup is implemented in a reusable module so other parts of the project can call it later.

Current behavior:

- reads the stored `spotify:track:{id}` URI from the database
- extracts the track ID from the URI
- calls `https://api.spotify.com/v1/tracks/{id}` using `curl`
- parses the JSON response with `cJSON`

To enable it:

```bash
export SPOTIFY_ACCESS_TOKEN='your_token_here'
./stats
```

## Testing

Build the test binaries:

```bash
make test
```

Run the current tests:

```bash
./tests/test_display
./tests/test_spotify_api
```

## Notes

- The default database path is `spotifyHistory.db`
- The app writes logs to `stats.log`
- JSON import expects Spotify extended streaming history files with audio entries that include `spotify_track_uri`
