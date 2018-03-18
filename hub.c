#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include "hub.h"
#include "comms.h"

/*
 * ===========================================================================
 * CSSE2310 Assignment 3
 * 2310express hub - controls all game operations.
 * ===========================================================================
 */

// Global store of players associated with hub for exit handling
Player **globalPlayers;
// Count of players for use in globalPlayers iteration
int playerCount;

/* ===========================================================================
 * Hub handler functions
 * ===========================================================================
 */
/*
 * Handler for signals. Only handles sig int for this program.
 *
 * @param sig   the signal being handled.
 */
void handle_sig(int sig) {
    if (sig == SIGINT) {
        handle_exit(GOT_SIGINT);
    }
}

/*
 * Checks if player exited cleanly.
 *
 * @param player    the id of the player.
 * @return true if player exited, false otherwise.
 */
bool player_exit(int player) {
    int status;
    waitpid(globalPlayers[player]->pid, &status, WNOHANG);

    if (WIFEXITED(status)) {
        // Print exit status
        if (WEXITSTATUS(status) > 0) {
            fprintf(stderr, "Player %c ended with status %d\n",
                    globalPlayers[player]->symbol, WEXITSTATUS(status));
            return true;
        } else if (WEXITSTATUS(status) == 0) {
            return true;
        }
    }
    return false;
}

/*
 * Attempts to exit players cleanly, otherwise sends SIGKILL.
 *
 * @param exitStatus    the exit status being handled.
 */
void exit_clean_up(int exitStatus) {
    for (int i = 0; i < playerCount; i++) {
        if (globalPlayers[i]->input != NULL) {
            fprintf(globalPlayers[i]->input, "%s\n", GAME_OVER);
        }
        // did player exit?
        sleep(2);
        if (!player_exit(i)) {
            kill(globalPlayers[i]->pid, SIGKILL);
            fprintf(stderr, "Player %c shutdown after receiving signal %d\n",
                    globalPlayers[i]->symbol, SIGKILL);
        }
    }
}

/*
 * Handles exit statuses.
 *
 * @params exitStatus   the exit code associated with exit handler call.
 */
void handle_exit(int exitStatus) {

    switch(exitStatus) {
        case WRONG_ARGS:
            // Wrong number of arguments
            fprintf(stderr,
                    "Usage: hub seed width player player [player ...]\n");
            break;

        case INVALID_ARG:
            // Invalid arguments
            fprintf(stderr, "Bad argument\n");
            break;

        case PROCESS_FAIL:
            // Process start failure
            fprintf(stderr, "Bad start\n");
            break;

        case PLAYER_CLOSED:
            // Player close before game over.
            fprintf(stderr, "Client disconnected\n");
            break;

        case PROTOCOL_ERROR:
            // Game protocol error by player/client
            fprintf(stderr, "Protocol error by client\n");
            break;

        case ILLEGAL_MOVE:
            // Player move illegal
            fprintf(stderr, "Illegal move by client\n");
            break;

        case GOT_SIGINT:
            // Caught and handled sigint
            fprintf(stderr, "SIGINT caught\n");
            break;
    }

    // Clean up players
    if (exitStatus >= PROCESS_FAIL || exitStatus == EXIT_SUCCESS) {
        exit_clean_up(exitStatus);
    }
    exit(exitStatus);
}

/*
 * ===========================================================================
 * Process setup functions.
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
        int id) {
    // Setup input to player pipe.
    if (close(input[WRITE]) == -1 || dup2(input[READ], READ) == -1 ||
            close(input[READ]) == -1) {
        handle_exit(PROCESS_FAIL);
    }

    // Setup output from player pipe, exit if failure at any point.
    if (close(output[READ]) == -1 || dup2(output[WRITE], WRITE) == -1 ||
            close(output[WRITE]) == -1) {
        handle_exit(PROCESS_FAIL);
    }

    // Close stderr, lowest fd will be stderr as stdin and stdout used
    if (fclose(stderr) == EOF || open("/dev/null", O_RDWR) == -1) {
        handle_exit(PROCESS_FAIL);
    }

    // Store parameters as strings for exec args, buffer +1 for terminator.
    char numPlayers[num_digits(game->numPlayers) + 1];
    sprintf(numPlayers, "%d", game->numPlayers);

    char thisID[num_digits(id) + 1];
    sprintf(thisID, "%d", id);

    char numCarriages[num_digits(game->numCarriages) + 1];
    sprintf(numCarriages, "%d", game->numCarriages);

    char seed[num_digits((int) game->seed) + 1];
    sprintf(seed, "%d", game->seed);

    // Exec player if pipe setup complete
    execlp(playerPath, playerPath, numPlayers, thisID, numCarriages,
            seed, NULL);

    // Exec failed if we got here
    handle_exit(PROCESS_FAIL);
}

/*
 * Sets up parent process after fork.
 *
 * @param input    input to player pipe
 * @param output   output from player pipe
 * @param *game    game structure and data
 * @param id       player ID per position in player array
 * @param pid      player process PID
 */
void setup_parent_fork(int input[], int output[], Game *game, int id,
        pid_t pid) {
    // Close pipe ends for parent side.
    if (close(input[READ]) == -1 || close(output[WRITE]) == -1) {
        handle_exit(PROCESS_FAIL);
    }

    // Store player information
    if ((game->players[id]->input = fdopen(input[WRITE], "w")) == NULL ||
            (game->players[id]->output = fdopen(output[READ], "r")) == NULL) {
        handle_exit(PROCESS_FAIL);
    }
    game->players[id]->pid = pid;

    // Associate player in global player holder
    globalPlayers[id] = game->players[id];
    playerCount++;
}

/*
 * Setup pipes for players and execs player process.
 *
 * @param *game         struct representing the Game.
 * @param id            id of player in array of players.
 * @param *playerPaths  array of player paths for exec
 */
void setup_process(Game *game, int id, char *playerPaths[]) {
    int inputToPlayer[2], outputFromPlayer[2];
    pid_t childPID;

    // Create pipes
    if (pipe(inputToPlayer) == -1 || pipe(outputFromPlayer) == -1) {
        handle_exit(PROCESS_FAIL);
    }

    if ((childPID = fork()) == -1) {
        handle_exit(PROCESS_FAIL);
    }

    if (childPID == 0) {
        // Child Process
        setup_child_fork(inputToPlayer, outputFromPlayer, playerPaths[id],
                game, id);
    } else {
        // Parent process
        setup_parent_fork(inputToPlayer, outputFromPlayer, game, id,
                childPID);
    }
}

/*
 * Checks that all players are ready.
 * Players ready if they send the '!' signal.
 *
 * @param *game     the game data struct
 * @return  true if all players have sent '!' ready signal, else false.
 */
bool players_ready(Game *game) {
    int handshake;
    for (int i = 0; i < game->numPlayers; i++) {
        handshake = fgetc(game->players[i]->output);
        if (handshake != '!') {
            return false;
        }
        // Clear rest of pipe.
        // handshake = fgetc(game->players[i]->output);
    }
    return true;
}

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
void handle_dryout(Game *game, int id) {
    // Dry out player
    game->players[id]->hits = 0;

    // Update orders
    game->players[id]->orders[0] = DRY;

    // Update players
    char args[MAX_PARAMS] = {'\0'};
    args[0] = game->players[id]->symbol;
    message_all(game, TELL_DRY, args);
}

/*
 * Handles looting instructions. Sends execution phase message when done.
 *
 * @param *game     current game state.
 * @param id        player id we are handling.
 */
void handle_loot(Game *game, int id) {
    // Current player position
    Position pos = game->players[id]->pos;
    int width = game->numCarriages;

    if (game->train[pos.y * width + pos.x] > 0) {
        // found loot
        game->train[pos.y * width + pos.x]--;
        game->players[id]->loot++;
    }

    // Update orders
    game->players[id]->orders[0] = LOOT;

    // Message to players
    char args[MAX_PARAMS] = {'\0'};
    args[0] = game->players[id]->symbol;
    message_all(game, TELL_LOOT, args);
}

/*
 * Handles shooting instructions. Sends execution phase instructions once
 * finalised.
 *
 * @param *game     the current game state.
 * @param id        player id we are handling
 */
void handle_shot(Game *game, int id) {
    // Player order
    char order = game->players[id]->newOrders[0];
    char param = game->players[id]->newOrders[1];
    char args[MAX_PARAMS] = {'\0'};
    args[0] = game->players[id]->symbol;
    args[1] = param;

    if (order == SHOOT_S && param != NO_TARGET) {
        // Target loses loot and train gains loot at position
        if (game->players[param - 'A']->loot > 0) {
            game->players[param - 'A']->loot--;
            Position targetPos = game->players[param - 'A']->pos;
            game->train[targetPos.y * game->numCarriages + targetPos.x]++;
        }
        message_all(game, TELL_SHORT, args);
    } else if (order == SHOOT_L && param != NO_TARGET) {
        game->players[param - 'A']->hits++;
        message_all(game, TELL_LONG, args);
    } else if (order == SHOOT_S && param == NO_TARGET) {
        message_all(game, TELL_SHORT, args);
    } else if (order == SHOOT_L && param == NO_TARGET) {
        message_all(game, TELL_LONG, args);
    }
}

/*
 * Handles movement instructions. Sends execution phase instructions once
 * finalised.
 *
 * @param *game     the current game state.
 * @param id        player id we are handling.
 */
void handle_movement(Game *game, int id) {
    // Player orders
    char order = game->players[id]->newOrders[0];
    char param = game->players[id]->newOrders[1];
    // Current position of player
    Position pos = game->players[id]->pos;
    // Arg storage for message sending
    char args[MAX_PARAMS] = {'\0'};

    // Handle vertical
    if (order == MOVE_V && pos.y == 0) {
        game->players[id]->pos.y = 1;
    } else if (order == MOVE_V && pos.y == 1) {
        game->players[id]->pos.y = 0;
    }

    // Handle horizontal
    if (order == MOVE_H && param == DIR_LEFT) {
        game->players[id]->pos.x--;
    } else if (order == MOVE_H && param == DIR_RIGHT) {
        game->players[id]->pos.x++;
    }

    // Update past orders
    game->players[id]->orders[0] = order;
    game->players[id]->orders[1] = param;

    // Send message to all players
    args[0] = game->players[id]->symbol;
    if (order == MOVE_V) {
        message_all(game, TELL_VMOVE, args);
    } else if (order == MOVE_H) {
        args[1] = param;
        message_all(game, TELL_HMOVE, args);
    }
}

/*
 * Gathers/seeks further instructions.
 *
 * @param *game     the hub's game state.
 * @param id        the player we are requesting additional info from.
 */
void gather_instructions(Game *game, int id) {
    char instruction[MSG_MAX_LEN], order;
    order = game->players[id]->newOrders[0];

    switch (order) {
        case MOVE_H:
            send_message(game->players[id]->input, GET_DIR, NULL);
            break;
        case SHOOT_L:
            send_message(game->players[id]->input, GET_L_TARGET, NULL);
            break;
        case SHOOT_S:
            send_message(game->players[id]->input, GET_S_TARGET, NULL);
            break;
    }

    if (fgets(instruction, MSG_MAX_LEN, game->players[id]->output) == NULL) {
        handle_exit(PLAYER_CLOSED);
    } else {
        // Clean up message
        if (instruction[strlen(instruction) - 1] == '\n') {
            instruction[strlen(instruction) - 1] = '\0';
        }
    }
    if (!player_message_valid(instruction)) {
        handle_exit(PROTOCOL_ERROR);
    } else {
        game->players[id]->newOrders[1] = instruction[strlen(instruction) - 1];
    }
}

/*
 * Execution phase, hub examines orders and requests more information
 * if appropriate, before sending instructions to players + updating state.
 *
 * @param *game     the hub's view of game state.
 */
void execution_phase(Game *game) {
    char order;
    // Ensure all instructions are correct.
    for (int i = 0; i < game->numPlayers; i++) {
        order = game->players[i]->newOrders[0];
        if (strchr(VALID_MOVES, order) == NULL) {
            handle_exit(PROTOCOL_ERROR);
        }
    }

    // Execute for each player.
    for (int i = 0; i < game->numPlayers; i++) {
        order = game->players[i]->newOrders[0];
        if (order == MOVE_H || order == SHOOT_L || order == SHOOT_S) {
            // Need further instructions
            gather_instructions(game, i);
        }
        // Validate move is legal
        if (!move_is_legal(game, i)) {
            handle_exit(ILLEGAL_MOVE);
        }

        // Handle order and send message
        if (order == MOVE_H || order == MOVE_V) {
            handle_movement(game, i);
        } else if (order == SHOOT_L || order == SHOOT_S) {
            handle_shot(game, i);
        } else if (order == LOOT) {
            handle_loot(game, i);
        } else if (order == DRY) {
            handle_dryout(game, i);
        }

    }
}

/*
 * Requests an order from the player, non execution phase.
 * Expects some kind of playX order, then sends order to players.
 *
 * @param *game     the current game state according to the hub.
 */
void request_player_action(Game *game) {
    char message[MSG_MAX_LEN], params[MAX_PARAMS] = {'\0'};

    for (int i = 0; i < game->numPlayers; i++) {
        // If player needs to dry out, no need for instructions.
        if (game->players[i]->hits >= 3) {
            game->players[i]->newOrders[0] = DRY;
            continue;
        }
        // Tell player 'yourturn'
        send_message(game->players[i]->input, GET_ACTION, NULL);
        // Listen for response
        if (fgets(message, MSG_MAX_LEN, game->players[i]->output) == NULL) {
            handle_exit(PLAYER_CLOSED);
        } else {
            // Clean up message
            if (message[strlen(message) - 1] == '\n') {
                message[strlen(message) - 1] = '\0';
            }
        }

        // Process message
        if (!player_message_valid(message)) {
            handle_exit(PROTOCOL_ERROR);
        } else {
            // Update orders
            game->players[i]->newOrders[0] = message[strlen(message) - 1];
            params[0] = game->players[i]->symbol;
            params[1] = game->players[i]->newOrders[0];
            message_all(game, ORDERED, params);
        }
    }
}

/*
 * Hub game loop to run game.
 *
 * @param *game     the game state and data
 */
void hub_game_loop(Game *game) {
    while(1) {
        if (game->round > 15) {
            // End of game!
            determine_winners(game);
            message_all(game, GAME_OVER, NULL);
            // winner function
            handle_exit(EXIT_SUCCESS);
        }

        // Indicate a new round
        game->round++;
        game->execute = false;
        message_all(game, NEW_ROUND, NULL);

        // Get player action
        request_player_action(game);

        // Execution phase
        game->execute = true;
        message_all(game, EXECUTE, NULL);
        execution_phase(game);

        // Print game summary
        print_game_state(game);
    }
}

/*
 * Reports the winners of the game.
 *
 * @param *game     game state according to hub.
 */
void determine_winners(Game *game) {
    int mostLoot = 0;
    char winners[game->numPlayers];
    int numWinners = 0;

    // Calculate highest loot
    for (int i = 0; i < game->numPlayers; i++) {
        if (game->players[i]->loot > mostLoot) {
            mostLoot = game->players[i]->loot;
        }
    }

    // Get winners
    for (int i = 0; i < game->numPlayers; i++) {
        if (game->players[i]->loot == mostLoot) {
            winners[numWinners++] = game->players[i]->symbol;
        }
    }

    // Report winners
    printf("Winner(s):");
    for (int i = 0; i < numWinners; i++) {
        printf("%c", winners[i]);
        if (i == numWinners - 1) {
            printf("\n");
        } else {
            printf(",");
        }
    }
}

/*
 * Prints the current game state teo terminal/stdout.
 *
 * @param *game     game data that state will be drawn from.
 */
void print_game_state(Game *game) {
    // Print player status
    for (int i = 0; i < game->numPlayers; i++) {
        printf("%c@(%d,%d): $=%d hits=%d\n", game->players[i]->symbol,
                game->players[i]->pos.x, game->players[i]->pos.y,
                game->players[i]->loot, game->players[i]->hits);
        fflush(stdout);
    }

    // Print train status/loot remaining.
    for (int i = 0; i < game->numCarriages; i++) {
        printf("Carriage %d: $=%d : $=%d\n", i, game->train[i],
                game->train[game->numCarriages + i]);
        fflush(stdout);
    }
}

/*
 * Initialises game, after checking arguments are correct.
 *
 * @param argc      count of arguments provided
 * @param argv      array of pointers to arguments provided
 * @return Game struct with game info and players.
 */
Game *init_args(int argc, char **argv) {
    int numPlayers, numCarriages;
    unsigned int seed;

    // Check we an acceptable number of arguments.
    if (argc < (3 + MIN_PLAYERS)) {
        handle_exit(WRONG_ARGS);
    }
    // Check that seed and carriage arguments are numbers.
    if (!arg_is_number(argv[1]) || !arg_is_number(argv[2])) {
        handle_exit(INVALID_ARG);
    }

    // Extract number arguments for game.
    char *temp;
    numPlayers = argc - 3;
    seed = strtoul(argv[1], &temp, 10);
    numCarriages = strtol(argv[2], &temp, 10);
    // Final check for arguments
    if (numPlayers < MIN_PLAYERS || numPlayers > MAX_PLAYERS ||
            numCarriages < MIN_CARRIAGES) {
        handle_exit(INVALID_ARG);
    }

    // Initialise game struct
    Game *game = make_game(numPlayers, numCarriages, seed);
    globalPlayers = (Player **) malloc(sizeof(Player *) * numPlayers);
    for (int i = 0; i < numPlayers; i++) {
        globalPlayers[i] = (Player *) malloc(sizeof(Player) * numPlayers);
    }

    return game;
}

int main(int argc, char **argv) {
    // Main signal handler
    struct sigaction sa;
    sa.sa_handler = &handle_sig;
    sa.sa_flags = SA_RESTART;
    // Handle SIGINT
    sigaction(SIGINT, &sa, NULL);

    // Ignore these signals
    struct sigaction saIgnore;
    saIgnore.sa_handler = SIG_IGN;
    saIgnore.sa_flags = SA_RESTART;
    sigaction(SIGPIPE, &saIgnore, NULL);

    Game *game = init_args(argc, argv);

    // Extract player paths
    char *playerPaths[game->numPlayers];
    for (int i = 3; i < argc; i++) {
        playerPaths[i - 3] = argv[i];
    }

    // Setup players
    for (int i = 0; i < game->numPlayers; i++) {
        setup_process(game, i, playerPaths);
    }
    if (!players_ready(game)) {
        handle_exit(PROCESS_FAIL);
    }
    // Play game
    hub_game_loop(game);

    return EXIT_SUCCESS;
}
