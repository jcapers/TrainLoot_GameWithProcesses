#ifndef PLAYER_H
#define PLAYER_H

#include "shared.h"

/*
 * ===========================================================================
 * Shared player header file
 * ===========================================================================
 */

/* Exit codes */
#define EXIT_SUCCESS 0
#define WRONG_ARGS 1
#define INVALID_PLAYERS 2
#define INVALID_ID 3
#define INVALID_WIDTH 4
#define INVALID_SEED 5
#define COMMS_ERROR 6

/*
 * ===========================================================================
 * Player Startup Functions
 * ===========================================================================
 */
/*
 * Startup function to be called by main of all player types.
 * - Checks arguments for valid arguments, exits if failure detected.
 *
 * @param argc      argument count of cmd line input
 * @param **argv    vector of argument values from cmdline
 */
void startup_check(int argc, char **argv);

/*
 * Common main function called by all players to start.
 *
 * @param argc      count of arguments provided at exec
 * @param argv      list/vector of argument values, strings.
 */
void player_main(int argc, char **argv);

/*
 * ===========================================================================
 * Player Game Functions
 * ===========================================================================
 */
/*
 * Updates long shot for player shooting at target.
 *
 * @param *game     Current game state.
 * @param player    shooter id
 * @param target    target id
 */
void update_shot_l(Game *game, int player, int target);

/*
 * Updates short shot for player shooting at target.
 *
 * @param *game     Current game state.
 * @param player    shooter id
 * @param target    target id
 */
void update_shot_s(Game *game, int player, int target);

/*
 * Update function for looting.
 *
 * @param *game     the player's view of the game state.
 * @param player    the player's id
 */
void update_loot(Game *game, int player);

/*
 * Updates the move based on information parsed by update_state function.
 *
 * @param *game     the player's view of the game state.
 * @param player    the player's id to be updated.
 * @param direction the direction the player is moving in.
 */
void update_move(Game *game, int player, char direction);

/*
 * Updates the state of the game for the players, based on
 * declared action by hub in execute phase.
 *
 * @param *game     the player's view of the game state.
 * @param message   the message sent by hub informing on executed action.
 */
void update_state(Game *game, char message[]);

/*
 * Player chooses a direction or target according to hub request.
 *
 * @param *game     the player's view of the game state.
 * @param id        this player's id
 * @param message   the request from the hub
 */
void describe_action(Game *game, int id, char message[]);

/*
 * Player chooses a move based on its strategy.
 *
 * @param *game     player's view of the game state.
 * @param id        this player's id
 */
void choose_move(Game *game, int id);

/*
 * Directs message from hub to an action performed by player.
 * Assumes the format of the message fits protocol.
 *
 * @param message       the message from the hub
 * @param *game         the player's view of the game state
 * @param id            the id of this player
 */
void action_message(Game *game, char message[], int id);

/*
 * Main player game loop to receive and handle messages.
 *
 * @param *game     game struct with player view of game state
 * @param id        the id of this player
 */
void player_game_loop(Game *game, int id);

/*
 * Checks if another player is in the same carriage as player.
 *
 * @param *game     current game state
 * @param id        player we are checking against
 */
bool player_here(Game *game, int id);

/*
 * Finds a short target.
 *
 * @param *game     current game state.
 * @param id        id of player taking shot.
 * @return player id, in integer form.
 */
int select_short(Game *game, int id);

/*
 * Checks if there is a long target.
 *
 * @param *game     current game state
 * @param id        player we are checking against
 * @return true if long target found, else false
 */
bool has_long_target(Game *game, int id);

/*
 * Finds a long target.
 *
 * @param *game     current game state.
 * @param id        id of player takingg shot.
 * @return player id, in integer form.
 */
int select_long(Game *game, int id);

#endif
