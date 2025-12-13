#ifndef SORT_H
#define SORT_H

#include "types.h"
#include "game.h"
#include "eval.h"

// int compare_moves(const void * va, const void * vb);

static inline int compare_moves(const void * va, const void * vb){
    const Move *a = (Move*)va;
    const Move *b = (Move*)vb;
    if (a->score > b->score) return -1;
    if (a->score < b->score) return 1;
    return 0;
}
static inline int score_move(Game * game, Move * move, SearchData * search_data, int ply, Move * tt_move, Move countermove, bool has_countermove){



    // print_move_full(move);
    int score = 0;
    if (tt_move){
        if (move_is_equal(move, tt_move)){
            score +=10000000;
            
        }
        
    }
    if (move_is_equal(move, &search_data->pv_table[ply][0])){
        score +=9000000;
    }
    
    if (move->type == CAPTURE || (move->type == PROMOTION && move->promotion_capture) || move->type == EN_PASSANT){
        // int see_val = see(game, move);
        // if (see_val > 0){
        int s = mvv_lva[move->piece][move->capture_piece];
        // if (see_val < -10000) see_val = -10000;
        // if (see_val > 10000) see_val = 10000;
        // printf("MVV_LVA: %d\n", s);
        // if (see_val < 0) see_val -= 200;
        // printf("SEE: %d\n", see_val);
        score +=s + 8000000;
        // return s * 2 + see_val + 8000000;
        // score += s * 2 + see_val + 8000000;
        // score += s * 2;
            
        // }
    }
    uint32_t refutation = get_refutation(game, &game->last_move);
    if (refutation){
        Move r;
        unpack_move(refutation, &r);
        if (move_is_equal(move, &r)){
          score += 7000000;
        }
    }
    if (has_countermove){
        if (move_is_equal(move, &countermove)){
            score +=6500000;
        }
    }
    if (move->type == PROMOTION){
        score +=6000000;
        // printf("PROMOTION: %d\n", score);
    }
    if (move->is_checking){
        score +=5000000;
    }
    if (search_data){
        if (move_is_equal(move, &search_data->killer_moves[ply][0]) || move_is_equal(move, &search_data->killer_moves[ply][1])){
            // score = 4000000;
            score +=4000000;
            // printf("KILLER: 200\n");
        } else {
            
        }

        int history = game->history_table[move->side][move->start_index][move->end_index];
        // printf("HISTORY: %d\n", history);
        // score += history;
        score +=history;
        
    }
    // printf("FINAL SCORE: %d\n", score);


    
    return score;

}

static inline void sort_moves(Game * game, Move move_list[200], int move_count, Move * best_move, SearchData * search_data, int ply){
    
    // print_moves(move_list, move_count);
    bool has_pv_move = false;
    uint32_t countermove = get_countermove(game, &game->last_move);
    Move c;
    bool has_countermove = false;
    bool has_refutation = false;
    if (countermove){
        has_countermove = true;
        unpack_move(countermove, &c);
    }
    
    
    for (int i = 0; i < move_count; i++){
        Move * move = &move_list[i];
        move->score = score_move(game, move, search_data, ply, best_move, c, has_countermove);
    }
    partial_selection_sort(move_list, move_count, 15);
    // qsort(move_list, move_count, sizeof(Move), compare_moves);
    // if (has_pv_move){
        
    // print_moves(move_list, move_count);
    // }

}

static inline int score_qmove(Game * game, Move * move, SearchData * search_data, int ply, Move countermove, bool has_countermove, Move * tt_move){



    // print_move_full(move);
    int score = 0;
    if (tt_move){
        if (move_is_equal(move, tt_move)){
            score +=10000000;
            
        }
        
    }
    if (move_is_equal(move, &search_data->pv_table[ply][0])){
        score += 9000000;
    }
    
    if (move->type == CAPTURE || (move->type == PROMOTION && move->promotion_capture) || move->type == EN_PASSANT){
        int s = mvv_lva[move->piece][move->capture_piece];
        // int see_val = see(game, move);
        // if (see_val < -10000) see_val = -10000;
        // if (see_val > 10000) see_val = 10000;
        // printf("MVV_LVA: %d\n", s);
        // if (see_val < 0) see_val -= 200;
        // printf("SEE: %d\n", see_val);
        // if (see_val <= -100){
        //     return -100;
        // }
        // score += s * 2 + see_val + 8000000;
        score += s  + 8000000;
        // score += s * 2 + 8000000;
            
        // }
    } 
    if (move->is_checking){
        score += 7000000;
    }
    uint32_t refutation = get_refutation(game, &game->last_move);
    if (refutation){
        Move r;
        unpack_move(refutation, &r);
        if (move_is_equal(move, &r)){
            score += 6000000;
        }
    }
    if (has_countermove){
        if (move_is_equal(move, &countermove)){
            score += 5500000;
        }
    }
    if (move->type == PROMOTION){
        score += 5000000;
    }
    if (search_data){
        if (move_is_equal(move, &search_data->killer_moves[ply][0]) || move_is_equal(move, &search_data->killer_moves[ply][1])){
            score += 4000000;
        }

        int history = game->history_table[move->side][move->start_index][move->end_index];
        // score += history;
        score += history;
        
    }
    // if (move->is_checking){
    //     if (see(game, move) < -150){
    //         score = -20000;
    //     }
    // }


    
    return score;

}

static inline int sort_qmoves(Game * game, Move move_list[200], int move_count, Move * best_move, SearchData * search_data, int ply){
    
    // print_moves(move_list, move_count);
    bool has_pv_move = false;
    uint32_t countermove = get_countermove(game, &game->last_move);
    Move c;
    bool has_countermove = false;
    bool has_refutation = false;
    if (countermove){
        has_countermove = true;
        unpack_move(countermove, &c);
    }
    
    int good_quiets = 0;
    int moves = 0;
    for (int i = 0; i < move_count; i++){
        Move * move = &move_list[i];
        move->score = score_qmove(game, move, search_data, ply, c, has_countermove, best_move);
        if (move->score > -10000) moves++;
    }
    int maximum = moves;
    partial_selection_sort(move_list, move_count, 15);
    // qsort(move_list, move_count, sizeof(Move), compare_moves);
    // if (has_pv_move){
        
    // print_moves(move_list, move_count);
    // }
    return maximum;

}


#endif
