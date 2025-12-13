#include "search.h"
#include "types.h"

#include "game.h"
#include "math.h"
#include "move_generation.h"
#include "types.h"
#include "utils.h"
#include "magic.h"
#include "zobrist.h"
#include <string.h>


bool handle_uci_input(Game * game, SearchData * search_data){



    char input_line[2000000];
    fgets(input_line, sizeof(input_line), stdin);
    // if (strlen(input_line) <= 0) return true;

    UCICommand command = 0;
    char * l = strtok(input_line, " \n");
    if (l){
        if (strcmp(l, "isready") == 0){
            command = UCI_ISREADY;
            printf("readyok\n");
            fflush(stdout);
        } else if (strcmp(l, "ucinewgame") == 0){
            command = UCI_NEWGAME;
        } else if (strcmp(l, "position") == 0){
            command = UCI_POSITION;
        } else if (strcmp(l, "go") == 0){
            command = UCI_GO;
        
        } else if (strcmp(l, "stop") == 0){
            command = UCI_STOP;
        } else if (strcmp(l, "quit") == 0){
            command = UCI_QUIT;
        }
    } else {
        return true;
    }


    switch (command){

        case UCI_ISREADY:
            break;
        case UCI_NEWGAME:
            {
                init_new_game(game, WHITE);
                // reset_tt(game);
                for (int i = 0; i < 64; i++){
                    for (int c = 0; c < 64; c++){
                        game->history_table[WHITE][i][c] = 0;
                        game->history_table[BLACK][i][c] = 0;
                    }
                }
                break;
            }
        case UCI_POSITION:
            {
                l = strtok(NULL, " \n");
                if (l){
                   if (strcmp(l, "startpos") == 0){
                       init_new_game(game, WHITE);
                       l = strtok(NULL, " \n");
                       if (l){
                           if (strcmp(l, "moves") == 0){
                               l = strtok(NULL, " \n");
                               while (l){
                                   
                                    Move move;
                                    PieceType promotion_piece;
                                    int code = parse_move(game, l, &move, &promotion_piece);
                                    bool promotion = false;
                                    if (code == 2){
                                        promotion = true;
                                    }
                                   Move move_list[200];
                                   int move_count = 0;
                                   generate_moves(game, game->side_to_move, move_list, &move_count);
                                    Move * found_move = find_move(move_list, move_count, move.start_index, move.end_index, promotion, promotion_piece);
                                    if (!found_move){
                                        printf("NO MOVE FOUND FOR %s\n", l);
                                        fflush(stdout);
                                    } else {
                                        // print_move_full(found_move);
                                        if (!make_move(game, found_move)){
                                            printf("INVALID MOVE\n");
                                            fflush(stdout);
                                            undo_move(game, found_move);
                                        }
                            
                                    }
                                    l = strtok(NULL, " \n");
                               }
                           }
                       }
                   } else if (strcmp(l, "fen") == 0){
                       l = strtok(NULL, "\n");
                       if (l){
                           set_board_to_fen(game, l);
                           int moves_start = 0;
                           char moves[20000];
                           int char_count = 0;
                           // i hate this
                           for (int i = 0; i < strlen(l); i++){
                               // searching for "moves"
                               if (l[i] == 'm'){
                                   moves_start = i;
                               }
                               if (moves_start > 0 && i > moves_start + 5){
                                   
                                   moves[char_count++] = l[i];
                               }
                           }
                           moves[char_count++] = '\n';
                           moves[char_count++] = '\0';
                           if (moves_start > 0){
                               char * iter_moves = strtok(moves, " \n");
                               while(iter_moves){
                                   
                                    Move move;
                                    PieceType promotion_piece;
                                    int code = parse_move(game, iter_moves, &move, &promotion_piece);
                                    bool promotion = false;
                                    if (code == 2){
                                        promotion = true;
                                    }
                                   Move move_list[200];
                                   int move_count = 0;
                                   generate_moves(game, game->side_to_move, move_list, &move_count);
                                    Move * found_move = find_move(move_list, move_count, move.start_index, move.end_index, promotion, promotion_piece);
                                    if (!found_move){
                                        printf("NO MOVE FOUND FOR %s\n", l);
                                        fflush(stdout);
                                    } else {
                                        // print_move_full(found_move);
                                        if (!make_move(game, found_move)){
                                            printf("INVALID MOVE\n");
                                            fflush(stdout);
                                            undo_move(game, found_move);
                                        }
                            
                                    }
                                    iter_moves = strtok(NULL, " \n");
                               }
                           }

                       }
                   }
                }
            }
            // print_game_board(game);
            break;
        case UCI_GO:
            {
                SearchFlags flags;
                int max_time = 10;
                l = strtok(NULL, " \n");
                Side side = game->side_to_move;
                int w_time = 0;
                int b_time = 0;
                if (l){
                    if (strcmp(l, "wtime") == 0){
                        l = strtok(NULL, " ");
                        if (l){
                            w_time = strtol(l, NULL, 10);
                            l = strtok(NULL, " ");
                            if (l){
                                if (strcmp(l, "btime") == 0){
                                    l = strtok(NULL, " \n");
                                    if (l){
                                        b_time = strtol(l, NULL, 10);
                                    }
                                }
                            }
                        }
                    }
                }
                // fflush(stdout);
                // if (l){
                //     max_time = strtod(l, NULL);
                // }
                float time_left = (float)b_time / 1000;
                switch(side){
                    case BLACK:
                        if (b_time > 0){
                            time_left = (float)b_time / 1000;
                        }
                        break;
                    case WHITE:
                        if (w_time > 0){
                            time_left = (float)w_time / 1000;
                        }
                        break;
                    default:
                        break;
                }
                if (time_left > 5000){
                    flags.max_time = 75;
                } else if (time_left > 3000){
                    flags.max_time = 60;
                } else if (time_left > 2000){
                    flags.max_time = 50;
                } else if (time_left > 1000){
                    flags.max_time = 40;
                } else if (time_left > 600){
                    flags.max_time = 30;
                } else if (time_left > 300){
                    flags.max_time = 11;
                } else if (time_left > 200){
                    flags.max_time = 9;
                } else if (time_left > 120){
                    flags.max_time = 6;
                } else if (time_left > 80){
                    flags.max_time = 4;
                } else if (time_left > 50){
                    flags.max_time = 2.6;
                } else if (time_left > 40){
                    flags.max_time = 2;
                } else if (time_left > 30){
                    flags.max_time = 1.25;
                } else if (time_left > 20){
                    flags.max_time = 1;
                } else{
                    flags.max_time = 0.6;
                }
                // printf("MAX TIME: %f\n", flags.max_time);
                // flags.max_time = 4;
                flags.check_hash = false;
                flags.uci = true;
                iterative_search(game, &flags);
                if (flags.mate || flags.three_fold_repetition){
                    printf("game end detected.\n");
                    fflush(stdout);
                }
                
            }
            break;
        case UCI_STOP:
            if (search_data){
                search_data->stop = true;
            }

            break;
        case UCI_QUIT:
            running = 0;
            return false;

            break;



        
    }
    
    return true;

    
}

