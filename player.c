#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "player.h"
#include "comms.h"

/*
 * ===========================================================================
 * 2310 Assignment 3
 * Common Player Functions
 * ===========================================================================
 */

/*
 * ===========================================================================
 * Player handler functions
 * ===========================================================================
 */
/*
 * Handles exit statuses.
 *
 * @params exitStatus   the exit code associated with exit handler call.
 */
void handle_exit(int exitStatus) {
    switch(exitStatus) {
        case WRONG_ARGS:
            // Wrong number of arguments
            fprintf(stderr, "Usage: player pcount myid width seed\n");
            break;

        case INVALID_PLAYERS:
            // Invalid number of players
            fprintf(stderr, "Invalid player count\n");
            break;

        case INVALID_ID:
            // Invalid player ID
            fprintf(stderr, "Invalid player ID\n");
            break;

        case INVALID_WIDTH:
            // Invalid number of carriages
            fprintf(stderr, "Invalid width\n");
            break;

        case INVALID_SEED:
            // Invalid seed number
            fprintf(stderr, "Invalid seed\n");
            break;

        case COMMS_ERROR:
            // Pipe closed or invalid message
            fprintf(stderr, "Communication Error\n");
            break;
    }
    exit(exitStatus);
}

/*
 * ===========================================================================
 * Player Game functions
 * ===========================================================================
 */
/*
 * Dries out a player.
 *
 * @param *game     current game state.
 * @param player    player id to be dried out.
 */
void update_dry(Game *game, int player) {
    char symbol = game->players[player]->symbol;
    // Dry out
    game->players[player]->hits = 0;
    fprintf(stderr, "%c dries off\n", symbol);

    // Update orders
    game->players[player]->orders[0] = DRY;

}

/*
 * Updates long shot for player shooting at target.
 *
 * @param *game     Current game state.
 * @param player    shooter id
 * @param target    target id
 */
void update_shot_l(Game *game, int player, int target) {
    char pSymbol = game->players[player]->symbol, tSymbol;
    Position playerPos, targetPos, tempPos;
    int noShot, level;
    if (target != -1) {
        // Handle long
        tSymbol = game->players[target]->symbol;
        playerPos = game->players[player]->pos;
        targetPos = game->players[target]->pos;

        if (playerPos.y == 0 && targetPos.y == 0) {
            level = 0;
        } else if (playerPos.y == 1 && targetPos.y == 1) {
            level = 1;
        }

        if (level == 0 && playerPos.x != targetPos.x &&
                (targetPos.x == playerPos.x + 1
                || targetPos.x == playerPos.x - 1) && targetPos.x >= 0
                && targetPos.x < game->numCarriages) {
            // Has shot on lower level
            noShot = 0;
        } else if (level == 1 && playerPos.x != targetPos.x) {
            // Has shot on upper level
            noShot = 0;
            // Check no other player is closer, cancelling shot
            for (int i = 0; i < game->numPlayers; i++) {
                tempPos = game->players[i]->pos;
                // Only check for players on the same level
                if (i != player && i != target && tempPos.y == 1) {
                    if ((targetPos.x > playerPos.x && targetPos.x > tempPos.x)
                            || (targetPos.x < playerPos.x
                            && targetPos.x < tempPos.x)) {
                        noShot = 1;
                    }
                }
            }
        }
    } else {
        noShot = 1;
    }
    // Do we have a shot?
    if (noShot) {
        fprintf(stderr, "%c has no target\n", pSymbol);
    } else {
        game->players[target]->hits++;
        fprintf(stderr, "%c targets %c who has %d hits\n",
                pSymbol, tSymbol, game->players[target]->hits);
    }
    game->players[player]->orders[0] = SHOOT_L;
}

/*
 * Updates short shot for player shooting at target.
 *
 * @param *game     Current game state.
 * @param player    shooter id
 * @param target    target id
 */
void update_shot_s(Game *game, int player, int target) {
    char pSymbol = game->players[player]->symbol, tSymbol;
    Position playerPos, targetPos;
    if (target != -1) {
        tSymbol = game->players[target]->symbol;
        playerPos = game->players[player]->pos;
        targetPos = game->players[target]->pos;

        // Handle short
        if (playerPos.x == targetPos.x && playerPos.y == targetPos.y) {
            if (game->players[target]->loot > 0) {
                // Target hit and drops loot
                game->players[target]->loot--;
                game->train[targetPos.y * game->numCarriages + targetPos.x]++;
                fprintf(stderr, "%c makes %c drop loot\n", pSymbol, tSymbol);
            } else {
                // Target hit by no loot to drop!
                fprintf(stderr,
                        "%c tries to make %c drop loot they don't have\n",
                        pSymbol, tSymbol);
            }
        } else {
            fprintf(stderr, "%c has no target\n", pSymbol);
        }
    } else {
        fprintf(stderr, "%c has no target\n", pSymbol);
    }

    // Update orders
    game->players[player]->orders[0] = SHOOT_S;
}

/*
 * Update function for looting.
 *
 * @param *game     the player's view of the game state.
 * @param player    the player's id
 */
void update_loot(Game *game, int player) {
    char pSymbol = game->players[player]->symbol;
    Position currentPos = game->players[player]->pos;
    int index = currentPos.y * game->numCarriages + currentPos.x;

    if (game->train[index] > 0) {
        // Found loot
        game->train[index]--;
        game->players[player]->loot++;
        fprintf(stderr, "%c picks up loot (they now have %d)\n",
                pSymbol, game->players[player]->loot);
    } else {
        // No loot
        fprintf(stderr,
                "%c tries to pick up loot but there isn't any\n", pSymbol);
    }

    // Update orders tracking
    game->players[player]->orders[0] = LOOT;
}

/*
 * Updates the move based on information parsed by update_state function.
 *
 * @param *game     the player's view of the game state.
 * @param player    the player's id to be updated.
 * @param direction the direction the player is moving in.
 */
void update_move(Game *game, int player, char direction) {
    Position currentPos = game->players[player]->pos;
    char pSymbol = game->players[player]->symbol;

    // Update position if applicable
    if (direction == MOVE_V && currentPos.y == 0) {
        // Move up
        game->players[player]->pos.y = 1;
    } else if (direction == MOVE_V && currentPos.y == 1) {
        // Move down
        game->players[player]->pos.y = 0;
    } else if (direction == DIR_LEFT && currentPos.x > 0) {
        // Player moves if not at left most carriage
        game->players[player]->pos.x--;
    } else if (direction == DIR_RIGHT &&
            currentPos.x < game->numCarriages - 1) {
        // Player moves if not at right most carriage
        game->players[player]->pos.x++;
    }

    // Update last orders
    if (direction == MOVE_V) {
        game->players[player]->orders[0] = MOVE_V;
    } else {
        game->players[player]->orders[0] = MOVE_H;
        game->players[player]->orders[1] = direction;
    }

    fprintf(stderr, "%c moved to %d/%d\n", pSymbol,
            game->players[player]->pos.x, game->players[player]->pos.y);
}

/*
 * Updates the state of the game for the players, based on
 * declared action by hub in execute phase.
 *
 * @param *game     the player's view of the game state.
 * @param message   the message sent by hub informing on executed action.
 */
void update_state(Game *game, char message[]) {
    int length = strlen(message), player;
    char param;
    // Get params
    if (strstr(message, TELL_VMOVE) != NULL
            || strstr(message, TELL_LOOT) != NULL
            || strstr(message, TELL_DRY) != NULL) {
        // single param provided by instruction, ie just the player id
        player = message[length - 1] - 'A';
    } else {
        // We have player id and target/direction
        player = message[length - 2] - 'A';
        param = message[length - 1];
    }
    // Ensure player exists
    if (player > game->numPlayers - 1 || player < 0) {
        handle_exit(COMMS_ERROR);
    }
    // Determine update path, ensure player exists!
    if (strstr(message, TELL_HMOVE) != NULL) {
        if (param != DIR_LEFT && param != DIR_RIGHT) {
            handle_exit(COMMS_ERROR);
        } else {
            update_move(game, player, param);
        }
    } else if (strstr(message, TELL_VMOVE) != NULL) {
        update_move(game, player, MOVE_V);
    } else if (strstr(message, TELL_LONG) != NULL) {
        if (param == NO_TARGET) {
            update_shot_l(game, player, -1);
        } else {
            update_shot_l(game, player, param - 'A');
        }
    } else if (strstr(message, TELL_SHORT) != NULL) {
        if (param == NO_TARGET) {
            update_shot_s(game, player, -1);
        } else {
            update_shot_s(game, player, param - 'A');
        }
    } else if (strstr(message, TELL_LOOT) != NULL) {
        update_loot(game, player);
    } else if (strstr(message, TELL_DRY) != NULL) {
        update_dry(game, player);
    }
}

/*
 * Directs message from hub to an action performed by player.
 * Assumes the format of the message fits protocol.
 *
 * @param message       the message from the hub
 * @param *game         the player's view of the game state
 * @param id            the id of this player
 */
void action_message(Game *game, char message[], int id) {
    int length = strlen(message);

    if (strcmp(message, GAME_OVER) == 0) {
        handle_exit(EXIT_SUCCESS);
    } else if (strcmp(message, NEW_ROUND) == 0) {
        game->execute = false;
        game->round++;
    } else if (strcmp(message, GET_ACTION) == 0) {
        choose_move(game, id);
    } else if (strstr(message, ORDERED) != NULL) {
        char player = message[length - 2];
        int order = message[length - 1];
        if ((player - 'A') >= game->numPlayers
                || strchr(VALID_MOVES, order) == NULL) {
            handle_exit(COMMS_ERROR);
        }
        fprintf(stderr, "%c ordered %c\n", message[length - 2],
                message[length - 1]);
    } else if (strcmp(message, EXECUTE) == 0) {
        game->execute = true;
    } else if (strcmp(message, GET_DIR) == 0
            || strcmp(message, GET_S_TARGET) == 0
            || strcmp(message, GET_L_TARGET) == 0) {
        describe_action(game, id, message);
    } else if (strstr(message, TELL_HMOVE) != NULL
            || strstr(message, TELL_VMOVE) != NULL
            || strstr(message, TELL_LONG) != NULL
            || strstr(message, TELL_SHORT) != NULL
            || strstr(message, TELL_LOOT) != NULL
            || strstr(message, TELL_DRY) != NULL) {
        update_state(game, message);
    } else {
        handle_exit(COMMS_ERROR);
    }
}

/*
 * Main player game loop to receive and handle messages.
 *
 * @param *game     game struct with player view of game state
 * @param id        the id of this player
 */
void player_game_loop(Game *game, int id) {
    char message[MSG_MAX_LEN];

    while(1) {
        // Listen for messages from hub
        if (fgets(message, MSG_MAX_LEN, stdin) == NULL) {
            handle_exit(COMMS_ERROR);
        }
        // Clean up message, we don't need newline
        if (message[strlen(message) - 1] == '\n') {
            message[strlen(message) - 1] = '\0';
        }

        // Check hub sent a valid message.
        if (!hub_message_valid(message)) {
            handle_exit(COMMS_ERROR);
        } else {
            action_message(game, message, id);
        }
    }
}

/*
 * Checks if another player is in the same carriage as player.
 *
 * @param *game     current game state
 * @param id        player we are checking against
 * @return true if a player is found in same carriage, else false
 */
bool player_here(Game *game, int id) {
    Position playerPos = game->players[id]->pos;
    Position checkPos;

    for (int i = 0; i < game->numPlayers; i++) {
        checkPos = game->players[i]->pos;
        if (i != id && checkPos.y == playerPos.y
                && checkPos.x == playerPos.x) {
            return true;
        }
    }

    // No players found
    return false;
}

/*
 * Checks if there is a long target. Doesn't select target.
 *
 * @param *game     current game state
 * @param id        player we are checking against
 * @return true if long target found, else false
 */
bool has_long_target(Game *game, int id) {
    Position playerPos = game->players[id]->pos;
    Position checkPos;

    // Check for long targets
    for (int i = 0; i < game->numPlayers; i++) {
        checkPos = game->players[i]->pos;
        // Check lower level criteria
        if (i != id && (checkPos.x == playerPos.x - 1
                || checkPos.x == playerPos.x + 1) && checkPos.x >= 0
                && checkPos.x < game->numCarriages
                && checkPos.y == playerPos.y && playerPos.y == 0) {
            return true;
        } else if (i != id && (checkPos.x < playerPos.x
                || checkPos.x > playerPos.x) && checkPos.y == playerPos.y
                && playerPos.y == 1) {
            return true;
        }
    }

    // No target
    return false;
}

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
void startup_check(int argc, char **argv) {
    int numPlayers, id, numCarriages;
    unsigned int seed;
    char *temp;

    // Check we have enough args
    if (argc < 5 || argc > 5) {
        handle_exit(WRONG_ARGS);
    }

    // Check number of players
    if (!arg_is_number(argv[1]) ||
            (numPlayers = strtol(argv[1], &temp, 10)) < MIN_PLAYERS ||
            numPlayers > MAX_PLAYERS) {
        handle_exit(INVALID_PLAYERS);
    }

    // Check player ID
    if (!arg_is_number(argv[2]) || (id = strtol(argv[2], &temp, 10)) < 0 ||
            id > (numPlayers - 1)) {
        handle_exit(INVALID_ID);
    }

    // Check Carriage/width
    if (!arg_is_number(argv[3]) ||
            (numCarriages = strtol(argv[3], &temp, 10)) < MIN_CARRIAGES) {
        handle_exit(INVALID_WIDTH);
    }

    // Check seed
    if (!arg_is_number(argv[4]) || (seed = strtoul(argv[4], &temp, 10)) < 0) {
        handle_exit(INVALID_SEED);
    }
}

/*
 * Common main function called by all players to start.
 *
 * @param argc      count of arguments provided at exec
 * @param argv      list/vector of argument values, strings.
 */
void player_main(int argc, char **argv) {
    int myID, numPlayers, numCarriages;
    unsigned int seed;

    startup_check(argc, argv);
    // Send handshake/ready signal
    printf("!");
    fflush(stdout);

    // Create game
    char *temp;
    myID = strtol(argv[2], &temp, 10);
    numPlayers = strtol(argv[1], &temp, 10);
    numCarriages = strtol(argv[3], &temp, 10);
    seed = strtoul(argv[4], &temp, 10);
    Game *game = make_game(numPlayers, numCarriages, seed);

    // run game
    player_game_loop(game, myID);
}
