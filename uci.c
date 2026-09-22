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


StateInfo state_stack[1024];
int st_idx = 0;
bool handle_uci_input(Game * game, SearchData * search_data){



    char input_line[2000000];
    char * a = fgets(input_line, sizeof(input_line), stdin);

    UCICommand command = 0;
    char * save;
    char * l = strtok_r(input_line, " \n", &save);
    if (l){
        if (strcmp(l, "isready") == 0){
            command = UCI_ISREADY;
            printf("readyok\n");
            fflush(stdout);
        } else if (strcmp(l, "ucinewgame") == 0){
            command = UCI_NEWGAME;
        } else if (strcmp(l, "setoption") == 0){
            command = UCI_SETOPTION;
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
                reset_tt(game);
                reset_pawn_hash(game);
                reset_countermove_and_refutation_tables(game);
                reset_corrhist(game);
                reset_eval_table(game);
                init_opening_book(game, "./komodo.bin");
                game->gen = 0;
                game->fifty_move_clock = 1;
                for (int i = 0; i < 2024; i++){
                    game->key_history[i] = 0;
                }
                game->history_count = 0;
                game->has_valid_last_move = false;
                reset_piece_keys(game);
                // clear_history();
                // clear_capture_history();
                // clear_conthist();
                init_threads();
                break;
            }
        case UCI_SETOPTION:
            {
                l = strtok_r(NULL, " ", &save);
                if (!l) break;
                if (strcmp(l, "name")) break;
                
                l = strtok_r(NULL, " ", &save);
                if (!l) break;

                #define CMP(name, line, s) \
                    if (strcmp(line, #name) == 0){ \
                        line = strtok_r(NULL, " ", &s);\
                        if (!line) break;\
                        if (strcmp(line, "value") == 0){\
                            line = strtok_r(NULL, " ", &s);\
                            if (line){ \
                                double d = strtod(line, NULL);\
                                (sp).name = d;\
                            }\
                        }\
                    }\

                #define EOCMP(name, line, s) \
                    if (strcmp(line, #name) == 0){ \
                        line = strtok_r(NULL, " ", &s);\
                        if (!line) break;\
                        if (strcmp(line, "value") == 0){\
                            line = strtok_r(NULL, " ", &s);\
                            if (line){ \
                                double d = strtod(line, NULL);\
                                (eo).name = d;\
                            }\
                        }\
                    }\

                EOCMP(debug_info, l, save);
                EOCMP(threads, l, save);
                CMP(eval_scale, l, save);
                CMP(aspiration_base, l, save);
                CMP(aspiration_mul, l, save);
                CMP(qdelta_margin, l, save);
                CMP(qsee_margin, l, save);
                CMP(check_prune_margin, l, save);
                CMP(rfp_mul, l, save);
                CMP(rfp_base, l, save);
                CMP(rfp_improving, l, save);
                CMP(rfp_depth, l, save);
                CMP(razor_depth, l, save);
                CMP(razor_base, l, save);
                CMP(razor_mul, l, save);
                CMP(razor_improving, l, save);
                CMP(nmp_mul, l, save);
                CMP(nmp_base, l, save);
                CMP(nmp_slope, l, save);
                CMP(probcut_depth, l, save);
                CMP(probcut_base, l, save);
                CMP(probcut_improving, l, save);
                CMP(iid_depth, l, save);
                CMP(lmr_depth, l, save);
                CMP(lmr_move_start, l, save);
                CMP(lmr_hd, l, save);
                CMP(lmr_cap_mul, l, save);
                CMP(lmr_cap_base, l, save);
                CMP(lmr_quiet_mul, l, save);
                CMP(lmr_quiet_base, l, save);
                CMP(lmp_depth, l, save);
                CMP(lmp_base, l, save);
                CMP(lmp_improving, l, save);
                CMP(lmp_depth_pow, l, save);
                CMP(futility_depth, l, save);
                CMP(futility_base, l, save);
                CMP(futility_mul, l, save);
                CMP(futility_hist_mul, l, save);
                CMP(futility_improving, l, save);
                CMP(chist_depth, l, save);
                CMP(chist1_margin, l, save);
                CMP(chist2_margin, l, save);
                CMP(mp_goodcap_margin, l, save);
                CMP(chist1_scale, l, save);
                CMP(chist2_scale, l, save);
                CMP(chist4_scale, l, save);
                CMP(chist6_scale, l, save);
                CMP(see_depth, l, save);
                CMP(see_quiet_margin, l, save);
                CMP(see_nonquiet_margin, l, save);
                CMP(se_depth, l, save);
                CMP(se_depth_margin, l, save);
                CMP(qhistory_base, l, save);
                CMP(qhistory_mul, l, save);
                CMP(qhpen_base, l, save);
                CMP(qhpen_mul, l, save);
                CMP(chistory_base, l, save);
                CMP(chistory_mul, l, save);
                CMP(chpen_base, l, save);
                CMP(chpen_mul, l, save);
                CMP(beta_bonus, l, save);
                CMP(corr_depth_base, l, save);
                CMP(corrhist_grain, l, save);
                CMP(corrhist_weight, l, save);
                CMP(corrhist_max, l, save);
                CMP(corr_pawn_weight, l, save);
                CMP(corr_np_weight, l, save);
                CMP(corr_mat_weight, l, save);
                CMP(corr_kbn_weight, l, save);
                CMP(corr_kqr_weight, l, save);
                CMP(corr_ch_weight, l, save);
                CMP(l1, l, save);
                CMP(l2, l, save);
                CMP(l3, l, save);
                
                break;
            }
        case UCI_POSITION:
            {
                game->st = &game->os;
                game->os.pst = NULL;
                st_idx = 0;
                l = strtok_r(NULL, " \n", &save);
                if (l){
                   if (strcmp(l, "startpos") == 0){
                       init_new_game(game, WHITE);
                       l = strtok_r(NULL, " \n", &save);
                       if (l){
                           if (strcmp(l, "moves") == 0){
                               l = strtok_r(NULL, " \n", &save);
                               while (l){
                                   
                                    Move move;
                                    PieceType promotion_piece;
                                    int code = parse_move(game, l, &move, &promotion_piece);
                                    bool promotion = false;
                                    if (code == 2){
                                        promotion = true;
                                    }
                                   Move move_list[256];
                                   uint8_t move_count = 0;
                                   generate_moves(game, game->side_to_move, move_list, &move_count);
                                    Move found_move = find_move(move_list, move_count, move_from(move), move_to(move), promotion, promotion_piece);
                                    if (!found_move){
                                        printf("NO MOVE FOUND FOR %s\n", l);
                                        print_moves(move_list, move_count);
                                        fflush(stdout);
                                    } else {
                                        SearchStack stack[1];
                                        Undo undo;

                                        if (!make_move(game, found_move, &state_stack[st_idx++], stack)){
                                            printf("INVALID MOVE\n");
                                            fflush(stdout);
                                            undo_move(game, found_move, stack);
                                        }
                                        game->os.pst = NULL;
                            
                                    }
                                    l = strtok_r(NULL, " \n", &save);
                               }
                           }
                       }
                   } else if (strcmp(l, "fen") == 0){
                       l = strtok_r(NULL, "\n", &save);
                       if (l){
                           set_board_to_fen(game, l);
                           int moves_start = 0;
                           char moves[20000];
                           int char_count = 0;
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
                               char * move_save;
                               char * iter_moves = strtok_r(moves, " \n", &move_save);
                               while(iter_moves){
                                   
                                    Move move;
                                    PieceType promotion_piece;
                                    int code = parse_move(game, iter_moves, &move, &promotion_piece);
                                    bool promotion = false;
                                    if (code == 2){
                                        promotion = true;
                                    }
                                   Move move_list[256];
                                   uint8_t move_count = 0;
                                   generate_moves(game, game->side_to_move, move_list, &move_count);
                                    Move found_move = find_move(move_list, move_count, move_from(move), move_to(move), promotion, promotion_piece);
                                    if (!found_move){
                                        printf("NO MOVE FOUND FOR %s\n", l);
                                        print_moves(move_list, move_count);
                                        fflush(stdout);
                                    } else {
                                        // print_move_full(found_move);
                                        Undo undo;
                                        SearchStack stack[1];
                                        // StateInfo st;
                                        if (!make_move(game, found_move, &state_stack[ st_idx++], stack )){
                                            printf("INVALID MOVE\n");
                                            fflush(stdout);
                                            undo_move(game, found_move, stack);
                                        }
                            
                                    }
                                    iter_moves = strtok_r(NULL, " \n", &move_save);
                               }
                           }

                       }
                   }
                }
                // memcpy(&game->os, game->st, sizeof(StateInfo));
                // game->os.pst = NULL;
                // game->st = &game->os;
                // print_game_board(game);
            }

            break;
        case UCI_GO:
            {
                SearchFlags flags;
                flags.draw = false;
                flags.three_fold_repetition = false;
                flags.mate = false;
                int max_time = 10;
                l = strtok_r(NULL, " \n", &save);
                Side side = game->side_to_move;
                int w_time = 0;
                int b_time = 0;
                int winc = 0;
                int binc = 0;
                int mtime = 0;
                flags.nodes = 0;
                while (l){
                    if (strcmp(l, "wtime") == 0){

                        l = strtok_r(NULL, " ", &save);
                        if (l){
                            w_time = strtol(l, NULL, 10);
                        }
                        
                    } else if (strcmp(l, "btime") == 0){
                        
                        l = strtok_r(NULL, " ", &save);
                        if (l){
                            b_time = strtol(l, NULL, 10);
                        }
                    } else if (strcmp(l, "winc") == 0){
                        l = strtok_r(NULL, " ", &save);
                        if (l){
                            winc = strtol(l, NULL, 10);
                        }
                        
                    } else if (strcmp(l, "binc") == 0){
                        
                        l = strtok_r(NULL, " ", &save);
                        if (l){
                            binc = strtol(l, NULL, 10);
                        }
                    } else if (strcmp(l, "nodes") == 0){
                        l = strtok_r(NULL, " ", &save);
                        if (l){

                            flags.nodes = strtol(l, NULL, 10);
                        }
                    } else if (strcmp(l, "movetime") == 0){
                        l = strtok_r(NULL, " ", &save);
                        if (l){
                            mtime = strtol(l, NULL, 10);
                        }
                        
                    }
                    l = strtok_r(NULL, " ", &save);
                }
                double time_left = 0.0;
                double inc = 0.0;
                switch(side){
                    case BLACK:
                        time_left = (double)b_time / 1000.0;
                        inc = (double)binc / 1000.0;
                        break;
                    case WHITE:
                        time_left = (double)w_time / 1000.0;
                        inc = (double)winc / 1000.0;
                        break;
                    default:
                        break;
                }
                if (time_left > 5000){
                    flags.max_time = 75;
                } else if (time_left > 3000){
                    flags.max_time = 60;
                } else if (time_left > 2000){
                    flags.max_time = 220;
                } else if (time_left > 1000){
                    flags.max_time = 120;
                } else if (time_left > 600){
                    flags.max_time = 90;
                } else if (time_left > 300){
                    flags.max_time = 34;
                } else if (time_left > 200){
                    flags.max_time = 25;
                } else if (time_left > 120){
                    flags.max_time = 10;
                } else if (time_left > 80){
                    flags.max_time = 6;
                } else if (time_left > 50){
                    flags.max_time = 5;
                } else if (time_left > 40){
                    flags.max_time = 4;
                } else if (time_left > 30){
                    flags.max_time = 3;
                } else if (time_left > 10){
                    flags.max_time = 2;
                } else if (time_left > 1){
                    flags.max_time = 0.8;
                } else {
                    flags.max_time = 0.3;
                }
                double moves_left = 40;
                double alloc = time_left / moves_left + inc;
                // flags.max_time =
                //     MIN(flags.max_time + inc, MAX(time_left - 0.05, 0.0));
                alloc *= 0.95;
                flags.max_time = MAX(MIN(alloc, time_left * 0.2), 0.0005);


                if (mtime){
                    flags.max_time = (double)mtime / 1000.0;
                }


                flags.check_hash = false;
                flags.uci = true;
                // game->st = &game->os;
                // game->os.pst = NULL;
                if (flags.nodes) flags.max_time = INFINITY;
                iterative_search(game, &flags);
                if (flags.mate){
                    printf("LOSS\n");
                    fflush(stdout);
                }
                if (flags.draw){
                    printf("DRAW\n");
                    fflush(stdout);
                }
                
            }
            break;
        case UCI_STOP:
            if (search_data){
                search_data->stop = true;
            }
            running = 0;
            return false;

            break;
        case UCI_QUIT:
            running = 0;
            return false;

            break;



        
    }
    
    return true;

    
}

