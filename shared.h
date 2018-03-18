#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>

/*
 * ===========================================================================
 * Hub and Player Process common definitions and functions
 * ===========================================================================
 */

/* Player Constraints */
#define MIN_PLAYERS 2
#define MAX_PLAYERS 26

/* Train Constraints */
#define MIN_CARRIAGES 3

/* Game Constants */
#define MAX_ROUNDS 15

/* Typedef Structs for readability */
typedef struct PlayerInfo Player;
typedef struct GameInfo Game;
typedef struct Posn Position;

/* Represents position of a player, where x = horizontal, y = vertical */
struct Posn {
    int x;
    int y;
};

/* Main Game Struct */
struct GameInfo {
    // Base game params
    int numPlayers;
    unsigned int seed;
    int numCarriages;
    int round;
    // Game Phase, true if we are in execute phase.
    bool execute;
    // An array of player structs
    Player **players;
    // The Train
    int *train;
};

/* Player Data */
struct PlayerInfo {
    // Player params
    int id;
    char symbol;
    // Player status
    Position pos;
    int hits;
    int loot;
    // Player orders, index 0 == order type, index 1 == target/direction
    char orders[2];
    // New orders for hub to track orders received.
    char newOrders[2];
    // Player ID
    pid_t pid;
    // Player pipe end file descriptors
    FILE *input;
    FILE *output;
};

/*
 * ===========================================================================
 * Common Setup Functions
 * ===========================================================================
 */

/*
 * Creates the game.
 *
 * @param numPlayers    number of players for this game.
 * @param numCarriages  max number of carriages for this game.
 * @param seed          game seed for setup.
 * @returns pointer to game, setup with players.
 */
Game *make_game(int numPlayers, int numCarriages, unsigned int seed);

/*
 * Sets up player information struct for storing in game struct.
 *
 * @param *game     game struct holding parameters of game.
 * @param thisID    the id of this player.
 * @return pointer to a player.
 */
Player *make_player(Game *game, int thisID);

void free_structs();

/*
 * ===========================================================================
 * Common Utility/Admin functions
 * ===========================================================================
 */
/*
 * Handles exit statuses.
 *
 * @params exitStatus   the exit code associated with exit handler call.
 */
void handle_exit(int exitStatus);

/*
 * Checks if a string is a number.
 *
 * @param *arg  the string we are validating.
 * @returns true if string is a number,false otherwise.
 */
bool arg_is_number(char *arg);

/*
 * Get number of digits of a number.
 *
 * @param num   a positive integer of any size
 * @return the number of digits in num.
 */
int num_digits(int num);


#endif
