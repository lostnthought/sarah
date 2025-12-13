#include "game.h"
#include "math.h"
#include "move_generation.h"
#include "types.h"
#include "utils.h"
#include "magic.h"
#include "zobrist.h"
#include <string.h>
#include "sort.h"
#include "eval.h"
#include "search.h"


int quiesce(Game * game, int  alpha, int  beta, SearchData * search_data, int depth, int ply){
    
    if (search_data->stop) return 0;
    search_data->qnodes += 1;
    search_data->pv_length[ply] = 0;
    bool pv_node = alpha != beta - 1;
    int old_alpha = alpha;

    Move best_move;
    bool has_best_move = true;

    bool is_in_check = false;

    if (in_check(game, game->side_to_move)) is_in_check = true;
    int eval = 0;
    int  ktv = 0;
    EvalEntry * eval_entry = NULL;
    eval_entry = search_for_eval(game, game->key);

    if (eval_entry){
        eval = eval_entry->eval;
    } else {
        
        eval = evaluate(game, game->side_to_move, search_data, &ktv);
        hash_eval(game, game->key, eval);
    }

    if (ply >= 70){
        return eval;
    }

    int best_score = eval;
    if (!is_in_check){
        if (best_score >= beta){
            return best_score;
        } 
        if (best_score > alpha){
            alpha = best_score;
        }
        
    }
    TTEntry * tt_entry = search_for_tt_entry(game, game->key);

    search_data->tt_probes += 1;
    // search_data->pv_length[ply] = 0;

    if (tt_entry){
        search_data->tt_hits += 1;


        if (tt_entry->has_best_move && !search_data->disable_writes){

            unpack_move(tt_entry->move32, &best_move);
            // best_move = tt_entry->move;
            has_best_move = true;

        }
        if (!pv_node ){
        // if (!pv_node ){
    
            int16_t adjusted_score = adjust_mate_score_from_tt(tt_entry->score, tt_entry->ply, ply);
            // int16_t adjusted_score = tt_entry->score;
            if (tt_entry->type == EXACT) return adjusted_score;
            if (tt_entry->type == UPPER && adjusted_score <= alpha) return adjusted_score;
            if (tt_entry->type == LOWER && adjusted_score >= beta) return adjusted_score;
        }
        // if (tt_entry->type == PV_EXACT) pv_node = true;
            

    }

    // delta pruning
    // if we win a huge amount and still are under alpha, we can return upper bound.
    // int  max_delta = piece_values_mg[QUEEN];
    // if (promotion_ranks[game->side_to_move] & game->pieces[game->side_to_move][PAWN]){
    //     max_delta += piece_values_mg[QUEEN];
    // }
    // if (!is_in_check && best_score < alpha - max_delta ){
    //     search_data->delta_prunes += 1;
    //     return alpha;
    // }
    // if (alpha < best_score){
    //     alpha = best_score;
    // }
        

    Move * new_best = NULL;
    TTType hash_flag = UPPER;
    Move move_list[200];
    int move_count = 0;
    generate_non_quiet_moves(game, game->side_to_move, move_list, &move_count);
    // generate_moves(game, game->side_to_move, move_list, &move_count);
    int maximum = sort_qmoves(game, move_list, move_count, &best_move, search_data, ply);
    int legal_moves = 0;
    for (int i = 0; i < move_count; i++){
        Move * move = &move_list[i];

        // see pruning
        // if (see(game, move) < -50 && !move->is_checking && !is_in_check) {

        //     search_data->q_see_prunes+=1;
        //     continue;
        // }
        if(!make_move(game, &move_list[i])){
            undo_move(game, &move_list[i]);


        } else {
            legal_moves += 1;
            int score = -quiesce(game, -beta, -alpha, search_data, depth - 1, ply + 1);
            undo_move(game, &move_list[i]);
            if (score > best_score){
                best_score = score;
            }
            if (score >= beta){
                create_new_tt_entry(game, game->key, score, LOWER, depth, ply, &move_list[i]);
                return score;
            }
            if (score > alpha){
                hash_flag = EXACT;
                new_best = &move_list[i];
                store_pv(ply, new_best, search_data);
                alpha = score;
            }

        }
    }
    create_new_tt_entry(game, game->key, best_score, hash_flag, depth, ply, new_best);
    if (is_in_check && legal_moves == 0){
        
        // int lm = 0;
        // Move new_move_list[200];
        // int new_move_count = 0;
        // generate_moves(game, game->side_to_move, new_move_list, &new_move_count);
        // for (int i = 0; i < new_move_count; i++){
        //     if (make_move(game, &new_move_list[i])) {
        //         lm += 1;
        //     }
        //     undo_move(game, &new_move_list[i]);
        // }
        // if (lm == 0){
        //     create_new_tt_entry(game, game->key, -MATE_SCORE + ply, EXACT, depth, ply, NULL);
        //     return -MATE_SCORE + ply;
        // }
    }

    return best_score;
    
}


int search(Game * game, int  alpha, int  beta, int depth, SearchData * search_data, int ply, bool is_extended){
    

    if (search_data->stop) return 0;
    
    search_data->node_count += 1;
    int old_alpha = alpha;
    if (search_data->node_count % 100000 == 0){
        decay_history_table(game);
    }
    Move best_move;
    bool has_best_move = false;
    bool found_entry = false;
    bool is_in_check = in_check(game, game->side_to_move);
    bool queen_is_attacked = false;

    bool pv_node = depth > 0 && alpha != beta - 1;
    bool cut_node = false;
    TTType found_flag = 0;
    bool use_tt = true;
    int tt_depth = depth;
    TTEntry * tt_entry = NULL;

    if (three_fold_repetition(game, game->key)){
        return 0;
    }
    // if we are in check, we extend our depth and add another layer of recursion between us and qsearch.
    if (is_in_check){
        search_data->check_extensions += 1;
        depth += 1;
    }


    if (ply <= 3){
        clock_t end_time = clock();
        double elapsed_time = (double)(end_time - search_data->start_time) / CLOCKS_PER_SEC; 
        if (elapsed_time >= search_data->max_time) {
            // if (search_data->max_depth < 5 && !search_data->has_extended){
            //     search_data->max_time += 0.5;
            //     search_data->has_extended = true;
            // } else {
            search_data->stop = true;
            return 0;
                
            // }
        }
    }
    int  ktv = 0;

    if (depth <= 0){

        int eval = 0;
        eval = quiesce(game, alpha, beta, search_data, depth, ply);
            // eval= evaluate(game, game->side_to_move, search_data, &ktv);
        return eval;
    }

    search_data->pv_length[ply] = 0;
    if (use_tt){
        
        search_data->tt_probes += 1;
        tt_entry = search_for_tt_entry(game, game->key);

    
        if (tt_entry){
            search_data->tt_hits += 1;


            if (tt_entry->has_best_move && !search_data->disable_writes){

                unpack_move(tt_entry->move32, &best_move);
                // best_move = tt_entry->move;
                has_best_move = true;

            }
            found_entry = true;
            if (tt_entry->depth >= depth && !pv_node){
        
                int16_t adjusted_score = adjust_mate_score_from_tt(tt_entry->score, tt_entry->ply, ply);
                // int16_t adjusted_score = tt_entry->score;
                if (tt_entry->type == EXACT) return adjusted_score;
                if (tt_entry->type == UPPER && adjusted_score <= alpha) return adjusted_score;
                if (tt_entry->type == LOWER && adjusted_score >= beta) return adjusted_score;
            }
            // if (tt_entry->type == PV_EXACT) pv_node = true;
                

        }
    }
        
    int stand= 0;
    int sktv = 0;
    EvalEntry * eval_entry = NULL;
    // if (depth <= 2){
        
        eval_entry = search_for_eval(game, game->key);
        if (eval_entry){
            stand = eval_entry->eval;
        } else {
        
            stand = evaluate(game, game->side_to_move, search_data, &sktv);
         hash_eval(game, game->key, stand);
     }
    // } else {
    //     stand = game->psqt_evaluation_mg[game->side_to_move] - game->psqt_evaluation_mg[!game->side_to_move];
    // }
    if (ply >= 70){
        // return evaluate(game, game->side_to_move, search_data, &ktv);
        return stand;
    }

    bool improving = false;
    if (!is_in_check){
        if (game->history_count - 2 >= 0){
            
            int last_eval= 0;
            int lktv = 0;
            EvalEntry * last_eval_entry = NULL;

            last_eval_entry = search_for_eval(game, game->key_history[game->history_count - 2]);
            if (eval_entry){
                last_eval = eval_entry->eval;
                if (stand > last_eval){
                    improving = true;
                }
            }
        }
    }

    


    // reverse futility pruning
    // the reverse of futility pruning. if our eval - margin * depth still clears beta, we can early out on the same premise as NMP.
    int rfp_depth = improving ? 5 : 4;
    if (!pv_node && depth > 1 && depth <= rfp_depth && !is_in_check && !is_mate(beta)){
        int eval = stand - depth * 58 + 70;
        if (eval >= beta) {
            search_data->rfp += 1;
            return eval;
        }
    }

    // razoring
    // similar to futility pruning but we drop into qsearch and compare both against alpha
    if (!pv_node && depth <= 2 && !is_in_check && stand + 282 * depth <= alpha){
        int score = quiesce(game, alpha, alpha + 1, search_data, depth, ply);
        if (score <= alpha){
            search_data->razoring += 1;
            return score;
        }
    }
    

    int  score = (int )-MATE_SCORE * 2;
    bool allow_nmp = true;

    // null move pruning
    // if we don't make a move and we still clear beta, we can return lower bound, since that should never be the case (except for in endgame, which is why it's gated by phase)
    if (depth >= 2 && !is_in_check && game->phase > 8 && !pv_node && !is_mate(beta) && !is_mate(alpha) && stand >= beta) {
        int r = 0;
        if (depth > 10){
            r = 4;
        } else if (depth > 6){
            r = 3;
        } else {
            r = 2;
        }
        r += MAX((stand - beta) / 200, 0);
        r += improving;
        // credit: yukari

        if (game->en_passant_index != -1){
            if (pawn_captures[!game->side_to_move][game->en_passant_index] & game->pieces[game->side_to_move][PAWN]) {

                game->key ^= get_en_passant_random(game->en_passant_index);
            }

        }
        game->side_to_move = !game->side_to_move;
        game->key ^= get_turn_random();
        int old_en_passant_index = game->en_passant_index;
        game->en_passant_index = -1;

        int  null_score = -MATE_SCORE * 2;
        null_score = -search(game, -beta, -beta + 1, depth-1-r , search_data, ply + 1, is_extended);
        
        game->side_to_move = !game->side_to_move;
        game->key ^= get_turn_random();
        game->en_passant_index = old_en_passant_index;

        if (game->en_passant_index != -1){
            if (pawn_captures[!game->side_to_move][game->en_passant_index] & game->pieces[game->side_to_move][PAWN]) {

                game->key ^= get_en_passant_random(game->en_passant_index);
            }

        }
        if (null_score >= beta){
            search_data->null_prunes += 1;
            return null_score;
        }
    }

    // establish a stand for futility pruning. to make sure we don't overprune, we check the eval + margin against alpha before we look at moves as well.
    int val = 0;
    bool futility_prune = false;
    if (depth <= 5 && !is_mate(alpha) && !is_in_check){
        int sc = stand + depth * 90;
        val += sc + 80;
        if (sc <= alpha){
            score = sc;
            futility_prune = true;
        }
    }
    
    
    Move move_list[200];
    int move_count = 0;
    generate_moves(game, game->side_to_move, move_list, &move_count);
    if (has_best_move){
        
        sort_moves(game, move_list, move_count, &best_move, search_data, ply);
    } else {
        
        sort_moves(game, move_list, move_count, NULL, search_data, ply);
    }

    // we pass this to the child to get a move back to possibly put into refutation table
    Move refuter;


    TTType hash_type = UPPER;
    Move * new_best = NULL;
    int legal_moves = 0;
    for (int i = 0; i < move_count; i++){
        Move * move = &move_list[i];

        bool dangerous = is_in_check || move->type == PROMOTION || move->type == CAPTURE || move->type == EN_PASSANT || move->type == CASTLE || move->is_checking; 


        // delta pruning for captures
        // basically futility pruning but we can apply this before we ever make the move or evaluate. just based on the type of piece it is capturing
        if (futility_prune && (move->type == CAPTURE || (move->type == PROMOTION && move->promotion_capture)) && !move->is_checking){
            int v = piece_values_mg[move->capture_piece];
            if (move->type == PROMOTION){
                v += piece_values_mg[move->promotion_type] - piece_values_mg[PAWN];
            }
            if (v + val < alpha){
                search_data->futility_prunes += 1;
                continue;
            }
        }

        // see pruning
        // discards moves if they lose statically, margin scales with depth
        if (depth >= 1 && depth <= 3 && legal_moves > 2 && score > -MATE_SCORE + 500 ){
            const int see_prune_quiet_margin = 130;
            const int see_prune_nonquiet_margin = 125;
            if (move_is_quiet(game, move)){
                if (see(game, move) < -depth * see_prune_quiet_margin){
                    search_data->see_prunes += 1;
                    continue;
                }
            } else {
                
                if (see(game, move) < -depth * see_prune_nonquiet_margin){
                    search_data->see_prunes += 1;
                    continue;
                }
            }
        }


        // late move pruning, margin scales with depth
        // formula from yukari
        int lmp = (int)(10 + pow(2 * depth, 2))>>(!improving);
        if (depth <= 3 && !is_mate(score) && legal_moves > lmp && !dangerous){
            search_data->late_move_prunes += 1;
            continue;
        }


        if (!make_move(game, &move_list[i])){
            undo_move(game, &move_list[i]);
        } else {
    

            legal_moves += 1;
            bool move_causes_check = in_check(game, game->side_to_move);
            bool move_attacks_queen = false;

            // futility pruning
            if (depth <= 3 && !dangerous && futility_prune ){
                int fktv = 0;
                // evaluate from the side that made the move. if our eval + margin * depth is not enough to get above alpha, discard.
                int v = evaluate(game, !game->side_to_move, search_data, &fktv) + 110 * depth + 40;
                if (v < alpha){
                    search_data->futility_prunes += 1;
                    undo_move(game, &move_list[i]);
                    continue;
                }
                
            }


            int  f_ktv = 0;
            // if (search_data->flags.check_hash){
             // if (!check_hash(game)){
             //     printf("ZOBRIST MISALIGNMENT DETECTED:\n");
             //     char fen[MAX_FEN];
             //     print_move_full(&move_list[i]);
             //     output_game_to_fen(game, fen);
             //     print_game_board(game);
             //     printf("FEN: %s\n", fen);
             //     printf("OUR KEY: %lx\n", game->key);
             //     printf("SCRATCH KEY: %lx\n", create_zobrist_from_scratch(game));
             //     exit(0);
             // }
             // if (create_pawn_hash_from_scratch(game) != game->pawn_key){
             //     printf("ZOBRIST MISALIGNMENT DETECTED:\n");
             //     char fen[MAX_FEN];
             //     print_move_full(&move_list[i]);
             //     output_game_to_fen(game, fen);
             //     print_game_board(game);
             //     printf("FEN: %s\n", fen);
             //     printf("OUR KEY: %lx\n", game->key);
             //     printf("SCRATCH KEY: %lx\n", create_zobrist_from_scratch(game));
             //     exit(0);
                 
             // }
                
            // }

            int extension = 0;
            bool extend = false;
            int reduction = 0;
            if (depth >= 3 && legal_moves >= 5 && (move->type != CAPTURE && !(move->type == PROMOTION && move->promotion_capture))){
                // reduction = (int)((depth * (legal_moves - 1)) * 0.58 + 1.1);

                if (legal_moves >= 10 && depth >= 5){
                     reduction = depth / 2;
                } else if (legal_moves >= 7){
                    reduction = 2;
                } else {
                    reduction = 1;
                }

                // dial back for pv nodes (expected)
                reduction -= pv_node;
            }
            int  current_score = 0;


            // Null window
            if (reduction || (pv_node && legal_moves > 4)){
                search_data->lmrs_tried += 1;
                
                current_score = -search(game, -alpha-1, -alpha , depth + extension - reduction-1, search_data, ply + 1, extend || is_extended);

                // PVS / LMR research
                if (current_score >= alpha){
                    search_data->lmrs_researched += 1;
                     current_score = -search(game, -beta, -alpha, depth + extension - 1, search_data, ply + 1, extend || is_extended) ;
                    
                }

            } else {
                current_score = -search(game, -beta, -alpha, depth + extension - 1, search_data, ply + 1, extend || is_extended);
            }
            refuter = game->last_last_move;
                
            undo_move(game, &move_list[i]);

            if (search_data->stop) return 0;

            
            if (current_score > score){
                score = current_score;
            }
            if (current_score >= beta){
                // new_best = &move_list[i];
                if (legal_moves == 1) search_data->ordering_success += 1;
                if (!search_data->disable_writes){
        
                    move_list[i].score = 10000000;
                    create_new_tt_entry(game, game->key, current_score, LOWER, depth, ply, &move_list[i]);
                    if (move_is_quiet(game, &move_list[i])){
                        search_data->killer_moves[ply][1] = search_data->killer_moves[ply][0];
                        search_data->killer_moves[ply][0] = move_list[i];
                    
                    }
                    // game->history_table[move->side][move->start_index][move->end_index] += (32 * depth) - game->history_table[move->side][move->start_index][move->end_index] / 64;
                    game->history_table[move->side][move->start_index][move->end_index] += depth * depth;
                }
                store_countermove(game, &game->last_move, move);


                if (i + 1 < move_count){
                    store_refutation(game, &move_list[i+1], &refuter);
                } else {
                    
                    store_refutation(game, &move_list[i], &refuter);
                }
                
                return current_score;
            }
            if (score > alpha){
                new_best = &move_list[i];
                if (legal_moves == 1) search_data->ordering_success += 1;
                new_best->score = 1000000;

                hash_type = EXACT;
                // if (score > old_alpha){
                    store_pv(ply, new_best, search_data);
                    // hash_type = PV_EXACT;
                    // pv_node = true;
                // }
                alpha = score;
            }

        }
    }

    if (!search_data->disable_writes){
        
        create_new_tt_entry(game, game->key, score, hash_type, depth, ply, new_best);
    }
            

    if (legal_moves == 0 && is_in_check){
        // if no legal quiet moves, we should check the rest to determine if we are in checkmate
        // if (game->pieces[BLACK][QUEEN] & (1ULL << 52)){
        //     printf("WE GET HERE IN THE TREE\n");
        //     print_game_board(game);
        // }

        create_new_tt_entry(game, game->key, -MATE_SCORE + ply, EXACT, depth, ply, NULL);
        return -MATE_SCORE + ply;
    
    } else if (legal_moves == 0){
        return 1;
    } else if (three_fold_repetition(game, game->key)){
        return 0;
    }
    return score;
    
}


int search_root(Game * game, int depth, int  alpha, int  beta, SearchData * search_data, Move move_list[200], int move_count){

    if (search_data->stop) return 0;
    bool pv_node = true;

    search_data->pv_length[0] = 0;
    TTEntry * entry = NULL;

    int score = -MATE_SCORE * 2;
    Move * best_move = NULL;

    search_data->ply = 0;
    
    int legal_moves = 0;
    bool is_in_check = in_check(game, game->side_to_move);
    Move refuter;
    for (int i = 0; i < move_count; i++){
        
        Move * move = &move_list[i];
        if (make_move(game, move)){
            legal_moves += 1;
            bool move_causes_check = in_check(game, game->side_to_move);
            bool dangerous = is_in_check || move->type == PROMOTION || move->type == CAPTURE || move->type == CASTLE || move->is_checking;
        
            int extension = 0;
            bool extend = false;
            int reduction = 0;
            int current_score = 0;

            current_score = -search(game, -beta, -alpha, depth + extension - 1, search_data, 1, extend);
            move_list[i].score = current_score;

            if (current_score > score){
                score = current_score;
                best_move = &move_list[i];
                store_pv(0, best_move, search_data);
            }

        }
        undo_move(game, &move_list[i]);
        search_data->ply -= 1;

        if (search_data->stop) return 0;
        clock_t end_time = clock();
        double elapsed_time = (double)(end_time - search_data->start_time) / CLOCKS_PER_SEC; 
        if (elapsed_time >= search_data->max_time) {
        //     if (search_data->max_depth < 5 && !search_data->has_extended){
        //         search_data->max_time += 0.5;
        //         search_data->has_extended = true;
        //     } else {
            search_data->stop = true;
            return 0;
                
            // }
        }
    }
    if (best_move){
        // store_pv(0, best_move, search_data);
        search_data->current_best = *best_move;
    } else if (legal_moves == 0 && in_check(game, game->side_to_move)){
        search_data->flags.mate = true;
    } else if (legal_moves == 0){
        search_data->flags.three_fold_repetition = true;
    } else if (three_fold_repetition(game, game->key)){
        search_data->flags.three_fold_repetition = true;
    }
    return score;
    
}

Move iterative_search(Game * game, SearchFlags * flags){

    int MAX_DEPTH = 30;
    Move current_best;

    SearchData search_data;
    search_data.stop = false;
    search_data.start_time = clock();
    
    if (flags){
        search_data.max_time = flags->max_time;
        search_data.flags = *flags;
        // MAX_DEPTH = flags->max_depth;
    }
    search_data.ply = 0;
    search_data.node_count = 0;
    // memset(search_data.history, 0, sizeof(search_data.history));
    memset(search_data.killer_moves, 0, sizeof(search_data.killer_moves));
    memset(search_data.pv_length, 0, sizeof(search_data.pv_length));
    memset(search_data.pv_table, 0, sizeof(search_data.pv_table));
    search_data.flags.mate = false;
    search_data.flags.three_fold_repetition = false;
    search_data.current_best_score = 0;
    search_data.use_opening_book = true;
    search_data.has_extended = false;
    

    int best_depth = 1;

    double elapsed_time = 0; 

    int  best_move_score = 0;

    int delta = 50;
    int  alpha = -INT_MAX;
    int  beta = INT_MAX;
    // int a = 0;
    // int b = 0;
    Move move_list[200];
    int move_count = 0;
    generate_moves(game, game->side_to_move, move_list, &move_count);

    TTEntry * tt_entry = search_for_tt_entry(game, game->key);
    if (tt_entry){
        if (tt_entry->is_opening_book){
            Move m; 
            unpack_move(tt_entry->move32, &m);

            print_move_full(&m);
            fflush(stdout);
            bool promo = m.type == PROMOTION;
            Move * resolved_move = find_move(move_list, move_count, m.start_index, m.end_index, promo, m.promotion_type);

            if (resolved_move){
                // fflush(stdout);
                printf("bestmove ");
                print_move_algebraic(resolved_move);
                printf("\n");
                fflush(stdout);
               return *resolved_move; 
            }
        }
    }
    
    for (int i = 1; i < MAX_DEPTH; i++){


        search_data.node_count = 0;
        search_data.lmrs_researched = 0;
        search_data.lmrs_tried = 0;
        search_data.aspiration_fail = 0;
        search_data.null_prunes = 0;
        search_data.ordering_success = 0;
        search_data.tt_hits= 0;
        search_data.tt_probes= 0;
        search_data.qnodes= 0;
        search_data.futility_prunes= 0;
        search_data.max_depth = i;
        search_data.see_prunes = 0;
        search_data.q_see_prunes = 0;
        search_data.rfp = 0;
        search_data.razoring = 0;
        search_data.check_extensions = 0;
        search_data.delta_prunes = 0;
        search_data.late_move_prunes  = 0;
        search_data.pawn_hash_hits = 0;
        search_data.pawn_hash_probes = 0;
        for (int i = 0; i < 64; i++) {
            search_data.pv_length[i] = 0;

        }
        int  current_score = 0;
        qsort(move_list, move_count, sizeof(Move), compare_moves);
        // if (i <= 5){
            
        //     search_data.disable_writes = false;
        //     current_score = negamax_root(game, i, alpha, beta, &search_data, move_list, move_count);
        //     // delta = 35;
        //     // a = current_score - delta;
        //     // b = current_score + delta;
        // } else {
        //     delta = 45;
        //     int  new_score = negamax_root(game, i, current_score - delta, current_score + delta, &search_data, move_list, move_count);
        //     if (search_data.stop) break;
        //     if (new_score <= current_score - delta || new_score >= current_score + delta){
                search_data.disable_writes = false;
                current_score = search_root(game, i, alpha, beta, &search_data, move_list, move_count);
                if (search_data.stop) break;
         // } else {
         //         current_score = new_score;
         //    }
        // }
        // if (search_data.stop) break;
        //     int new_score = 0;


        
        // if we ran out of time, exit and discard that move
        clock_t end_time = clock();
        double elapsed_time = (double)(end_time - search_data.start_time) / CLOCKS_PER_SEC; 
        if (elapsed_time >= search_data.max_time || search_data.stop) break;
        best_depth = i;
        best_move_score = current_score;

        // debug info!
        printf("Depth %d nodes %d qnodes %d tt hits %d tt probes %d asp fails %d lmr tried %d lmr research %d nmp %d ordering success %d futility prunes %d lmp %d rfp %d razor %d check ext %d delta %d see prunes %d q see prunes %d pawn hash probes %d hits %d nps %f\n",
        search_data.max_depth, search_data.node_count, search_data.qnodes, search_data.tt_hits, search_data.tt_probes, search_data.aspiration_fail, search_data.lmrs_tried, search_data.lmrs_researched, search_data.null_prunes, search_data.ordering_success, search_data.futility_prunes, search_data.late_move_prunes, search_data.rfp, search_data.razoring, search_data.check_extensions, search_data.delta_prunes, search_data.see_prunes, search_data.q_see_prunes, search_data.pawn_hash_probes, search_data.pawn_hash_hits, (search_data.node_count + search_data.qnodes) / elapsed_time);

        // uci printouts
        if (!search_data.flags.mate && !search_data.flags.three_fold_repetition){
            printf("info depth %d score cp %d nodes %d pv ", i, best_move_score, search_data.node_count);
            for (int i = 0; i < search_data.pv_length[0]; i++) {
                print_move_algebraic(&search_data.pv_table[0][i]);
                printf(" ");
            }
            printf("\n");
            current_best = search_data.current_best;
            search_data.current_best_score = current_score;
            fflush(stdout);
            
        }

    }

    if (!search_data.flags.mate && !search_data.flags.three_fold_repetition){
        printf("bestmove ");
        print_move_algebraic(&current_best);
        printf("\n");
        fflush(stdout);
        
    } else {
        printf("Mate or Draw by Repetition detected.");
        if (flags){
            *flags = search_data.flags;
            
        }
    }
    return current_best;
    
}


uint64_t perft(Game * game, int depth){
    SearchData data;

    if (depth == 0) {
        int ktv = 0;
        // evaluate(game, game->side_to_move, &data, &ktv);
        return 1ULL;
    }
    
    Move move_list[200];
    int move_count = 0;
    generate_moves(game, game->side_to_move, move_list, &move_count);
    uint64_t nodes = 0;


    bool print = false;
    for (int i = 0; i < move_count; i++){
        
        Move * move = &move_list[i];

        // if illegal move
        if (!make_move(game, move)){

            undo_move(game, move);

        } else {
            uint64_t move_nodes = perft(game, depth - 1);
            nodes += move_nodes;

            undo_move(game, move);
        }
    }
    return nodes;
}

void perft_root(Game * game, int depth){

    clock_t start_time = clock();
    Move move_list[200];
    int move_count = 0;
    generate_moves(game, game->side_to_move, move_list, &move_count);
    uint64_t nodes = 0;

    for (int i = 0; i < move_count; i++){
        
        Move * move = &move_list[i];

        // if illegal move
        if (!make_move(game, move)){

            undo_move(game, move);

        } else {

            uint64_t move_nodes = perft(game, depth - 1);
            print_move_algebraic(move);
            printf(" Nodes: %ld\n", move_nodes);
            nodes += move_nodes;

            undo_move(game, move);
        }
    }

    clock_t end_time = clock();
    
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC; 
    
    printf("Depth %d Nodes: %ld and NPS: %f\n", depth, nodes, nodes / elapsed_time);
}
