#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "comms.h"

/*
 * ===========================================================================
 * CSSE2310 Assignment 3
 * COMMS - Main communications protocols and functions
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
void send_message(FILE *to, char *body, char *params) {
    if (params == NULL) {
        fprintf(to, "%s\n", body);
        fflush(to);
    } else {
        fprintf(to, "%s%s\n", body, params);
        fflush(to);
    }
}

/*
 * Bulk sends a message to all players.
 *
 * @param *game     the state of the game according to the hub.
 */
void message_all(Game *game, char *message, char *params) {
    for (int i = 0; i < game->numPlayers; i++) {
        send_message(game->players[i]->input, message, params);
    }
}

/*
 * Checks if message is valid message and has correct number of params.
 * Note - only checks FORMAT of message, not whether the provided params
 *          are valid.
 *
 * @param message   the message received.
 */
bool hub_message_valid(char message[]) {
    // Check message format for messages without parameters
    if (strcmp(message, GAME_OVER) == 0
            || strcmp(message, NEW_ROUND) == 0
            || strcmp(message, GET_ACTION) == 0
            || strcmp(message, EXECUTE) == 0
            || strcmp(message, GET_DIR) == 0
            || strcmp(message, GET_S_TARGET) == 0
            || strcmp(message, GET_L_TARGET) == 0) {
        return true;
    }

    // Check message format for messages with extra parameters
    if (strstr(message, ORDERED) != NULL &&
            strlen(message) == strlen(ORDERED) + 2) {
        return true;
    } else if (strstr(message, TELL_HMOVE) != NULL &&
            strlen(message) == strlen(TELL_HMOVE) + 2) {
        return true;
    } else if (strstr(message, TELL_VMOVE) != NULL &&
            strlen(message) == strlen(TELL_VMOVE) + 1) {
        return true;
    } else if (strstr(message, TELL_LONG) != NULL &&
            strlen(message) == strlen(TELL_LONG) + 2) {
        return true;
    } else if (strstr(message, TELL_SHORT) != NULL &&
            strlen(message) == strlen(TELL_SHORT) + 2) {
        return true;
    } else if (strstr(message, TELL_LOOT) != NULL &&
            strlen(message) == strlen(TELL_LOOT) + 1) {
        return true;
    } else if (strstr(message, TELL_DRY) != NULL &&
            strlen(message) == strlen(TELL_DRY) + 1) {
        return true;
    }

    // Message not valid
    return false;
}

/*
 * Checks if the player sent a valid message.
 * Note - checks for FORMAT only, does not check parameter legality.
 *
 * @param message       the message sent by the player.
 */
bool player_message_valid(char message[]) {
    /* Check message format and extra parameters exists.
     * All messages have one param. E.g., PlayX.
     */
    if (strstr(message, PLAY) != NULL &&
            strlen(message) == strlen(PLAY) + 1) {
        return true;
    } else if (strstr(message, GO_DIR) != NULL &&
            strlen(message) == strlen(GO_DIR) + 1) {
        return true;
    } else if (strstr(message, AIM_SHORT) != NULL &&
            strlen(message) == strlen(AIM_SHORT) + 1) {
        return true;
    } else if (strstr(message, AIM_LONG) != NULL &&
            strlen(message) == strlen(AIM_LONG) + 1) {
        return true;
    }

    // Message not valid for protocols
    return false;
}

/*
 * Checks if an action is legal.
 *
 * @param *game     current game state.
 * @param id        id of this player's action
 * @return true if legal action, else false.
 */
bool move_is_legal(Game *game, int id) {
    // Order for checking
    char order = game->players[id]->newOrders[0];
    char param = game->players[id]->newOrders[1];
    // Players current position
    Position pos = game->players[id]->pos;

    if (order == MOVE_H && param == DIR_LEFT && pos.x != 0) {
        // Player is not moving off leftmost carriage
        return true;
    } else if (order == MOVE_H && param == DIR_RIGHT &&
            pos.x != game->numCarriages - 1) {
        // Player not moving off rightmost carriage
        return true;
    } else if ((order == SHOOT_S || order == SHOOT_L) && param == NO_TARGET) {
        return true;
    } else if (order == SHOOT_S &&
            game->players[param - 'A']->pos.x == pos.x &&
            param != game->players[id]->symbol) {
        // Short target is in same carriage, and not itself.
        return true;
    } else if (order == SHOOT_L && pos.y == 0
            && game->players[param - 'A']->pos.y == 0
            && param != game->players[id]->symbol
            && (game->players[param - 'A']->pos.x == pos.x - 1
            || game->players[param - 'A']->pos.x == pos.x + 1)) {
        // Long target, on lower level is one carriage away
        return true;
    } else if (order == SHOOT_L && pos.y == 1
            && game->players[param - 'A']->pos.y == 1
            && param != game->players[id]->symbol
            && game->players[param - 'A']->pos.x != pos.x) {
        // Long target upper level, not in same carriage, any distance fine.
        Position tempPos, targetPos;
        for (int i = 0; i < game->numPlayers; i++) {
            tempPos = game->players[i]->pos;
            targetPos = game->players[param - 'A']->pos;
            if (i != id && i != (param - 'A') && tempPos.y == 1) {
                if ((targetPos.x > pos.x && targetPos.x <= tempPos.x)
                        || (targetPos.x < pos.x && targetPos.x >= tempPos.x)) {
                    return true;
                }
            }
        }
    } else if (order == MOVE_V || order == LOOT || order == DRY) {
        // These orders have no params, thus should be legal
        return true;
    }
    // Move illegal if here
    return false;
}
