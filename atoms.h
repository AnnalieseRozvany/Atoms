#ifndef ATOMS_H
#define ATOMS_H

#include <stdint.h>

#define MAX_LINE 255
#define MAX_PLAYERS 6
#define MIN_PLAYERS 2
#define MAX_WIDTH 255
#define MAX_HEIGHT 255
#define MIN_WIDTH 2
#define MIN_HEIGHT 2

typedef struct move_t move_t;
typedef struct player_t player_t;
typedef struct game_t game_t;
typedef struct grid_t grid_t;
typedef struct save_file_t save_file_t;

struct save_t {
  char* filename; // name of file, filenames
  save_file_t* data; //file data
};
struct move_t {
  int x; // x-coordinate
  int y; // y-coordinate
  move_t* parent; // last move?
  move_t* extra; // ??
  player_t* old_owner; //NULL if unoccupied
};
struct player_t {
  char* colour; // colour of player
  int grids_owned; // number of grids owned
  char lost; // whether the player has lost
};
struct game_t {
  move_t* moves; // array of moves of entire game??
};
struct grid_t {
  char owner; // owner of grid
  int atom_count; // count of atoms
};
struct save_file_t {
    uint8_t width; // width of grid
    uint8_t height; // height of grid
    uint8_t no_players; // number of players
    uint32_t* raw_move_data; // move data
};

#endif
