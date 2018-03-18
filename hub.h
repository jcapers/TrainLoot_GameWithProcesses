#ifndef HUB_H
#define HUB_H

#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include "shared.h"

/*
 * ===========================================================================
 * Hub header file
 * ===========================================================================
 */

/* Exit codes */
#define EXIT_SUCCESS 0
#define WRONG_ARGS 1
#define INVALID_ARG 2
#define PROCESS_FAIL 3
#define PLAYER_CLOSED 4
#define PROTOCOL_ERROR 5
#define ILLEGAL_MOVE 6
#define GOT_SIGINT 9

/* Pipe ends */
#define READ 0
#define WRITE 1

/*
 * ===========================================================================
 * Hub handler functions
 * ===========================================================================
 */
/*
 * Attempts to exit players cleanly, otherwise sends SIGKILL.
 *
 * @param exitStatus    the exit status being handled.
 */
void exit_clean_up(int exitStatus);

/*
 * Checks if player exited cleanly.
 *
 * @param player    the id of the player.
 * @return true if player exited, false otherwise.
 */
bool player_exit(int player);

/*
 * Handler for signals. Only handles sig int for this program.
 *
 * @param sig   the signal being handled.
 */
void handle_sig(int sig);

/*
 * ===========================================================================
 * Hub Functions
 * ===========================================================================
 */

/*
 * Sets up process for child, then execs if pipes established.
 *
 * @param input         input to player pipe
 * @param output        output from player pipe
 * @param *playerPath   path for player file.
 * @param *game         game struct with game data
 * @param id            the id of this player
 */
void setup_child_fork(int input[], int output[], char *playerPath, Game *game,
        int id);

/*
 * Sets up parent process after fork.
 *
 * @param input    input to player pipe
 * @param output   output from player pipe
 * @param *game    game structure and data
 * @param id       player ID per position in player array
 * @param pid      player process PID
 */
void setup_parent_fork(int input[], int output[], Game *game, int pos,
        pid_t pid);

/*
 * Setup pipes for players and execs player process.
 *
 * @param *game         struct representing the Game.
 * @param id            id of player in array of players.
 * @param *playerPaths  array of player paths for exec
 */
void setup_process(Game *game, int id, char *playerPaths[]);

/*
 * Checks that all players are ready.
 * Players ready if they send the '!' signal.
 *
 * @param *game     the game data struct
 * @return  true if all players have sent '!' ready signal, else false.
 */
bool players_ready(Game *game);

/*
 * Initialises game, after checking arguments are correct.
 *
 * @param argc      count of arguments provided
 * @param argv      array of pointers to arguments provided
 * @return Game struct with game info and players.
 */
Game *init_args(int argc, char **argv);

/*
 * ===========================================================================
 * Hub game functions
 * ===========================================================================
 */
/*
 * Handles dry out instruction, sends execution phase message when done.
 *
 * @param *game     current game state.
 * @param id        player id we are handling.
 */
void handle_dryout(Game *game, int id);

/*
 * Handles looting instructions. Sends execution phase message when done.
 *
 * @param *game     current game state.
 * @param id        player id we are handling.
 */
void handle_loot(Game *game, int id);

/*
 * Handles shooting instructions. Sends execution phase instructions once
 * finalised.
 *
 * @param *game     the current game state.
 * @param id        player id we are handling
 */
void handle_shot(Game *game, int id);

/*
 * Handles movement instructions. Sends execution phase instructions once
 * finalised.
 *
 * @param *game     the current game state.
 * @param id        player id we are handling.
 */
void handle_movement(Game *game, int id);

/*
 * Gathers/seeks further instructions.
 *
 * @param *game     the hub's game state.
 * @param id        the player we are requesting additional info from.
 */
void gather_instructions(Game *game, int id);

/*
 * Execution phase, hub examines orders and requests more information
 * if appropriate, before sending instructions to players + updating state.
 *
 * @param *game     the hub's view of game state.
 */
void execution_phase(Game *game);

/*
 * Requests an order from the player, non execution phase.
 * Expects some kind of playX order, then sends order to players.
 *
 * @param *game     the current game state according to the hub.
 */
void request_player_action(Game *game);

/*
 * Hub game loop to run game.
 *
 * @param *game     the game state and data
 */
void hub_game_loop(Game *game);

/*
 * Reports the winners of the game.
 *
 * @param *game     game state according to hub.
 */
void determine_winners(Game *game);

/*
 * Prints the current game state teo terminal/stdout.
 *
 * @param *game     game data that state will be drawn from.
 */
void print_game_state(Game *game);


#endif
