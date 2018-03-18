#ifndef COMMS_H
#define COMMS_H

#include <stdio.h>
#include <stdlib.h>
#include "shared.h"


/*
 * ===========================================================================
 * COMMS header file.
 * ===========================================================================
 */

/* Communications constants */
// Buffer for max parameters/information that accompany message.
#define MAX_PARAMS 3
// Message max including \n and \0
#define MSG_MAX_LEN 15

/* Valid moves */
#define VALID_MOVES "vlhs$d"
#define MOVE_V 'v'
#define MOVE_H 'h'
#define SHOOT_L 'l'
#define SHOOT_S 's'
#define LOOT '$'
#define DRY 'd'
#define DIR_LEFT '-'
#define DIR_RIGHT '+'
#define NO_TARGET '-'

/* Messages from Hub */
#define GAME_OVER "game_over"
#define NEW_ROUND "round"
#define GET_ACTION "yourturn"
#define ORDERED "ordered"
#define EXECUTE "execute"
#define GET_DIR "h?"
#define GET_S_TARGET "s?"
#define GET_L_TARGET "l?"
#define TELL_HMOVE "hmove"
#define TELL_VMOVE "vmove"
#define TELL_LONG "long"
#define TELL_SHORT "short"
#define TELL_LOOT "looted"
#define TELL_DRY "driedout"

/* Messages from player */
#define PLAY "play"
#define GO_DIR "sideways"
#define AIM_SHORT "target_short"
#define AIM_LONG "target_long"

/*
 * ===========================================================================
 * Commuications functions
 * ===========================================================================
 */

/*
 * Sends a message to a destination with specific message body
 * and optional parameters (i.e., NULL or otherwise).
 *
 * @param *to       destination of this message
 * @param body      message body defined in comms protocol
 * @param params    information accompanying message, if any
 */
void send_message(FILE *to, char *body, char *params);

/*
 * Bulk sends a message to all players.
 *
 * @param *game     the state of the game according to the hub.
 */
void message_all(Game *game, char *message, char *params);

/*
 * Checks if message is valid message and has correct number of params.
 * Note - only checks FORMAT of message, not whether the provided params
 *          are valid.
 *
 * @param message   the message received.
 */
bool hub_message_valid(char message[]);

/*
 * Checks if the player sent a valid message.
 * Note - checks for FORMAT only, does not check parameters.
 *
 * @param message       the message sent by the player.
 */
bool player_message_valid(char message[]);

/*
 * Checks if an action is legal.
 *
 * @param *game     current game state.
 * @param id        id of this player's action
 * @return true if legal action, else false.
 */
bool move_is_legal(Game *game, int id);

#endif
