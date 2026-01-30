#ifndef SORT_H
#define SORT_H

#include "utils.h"
#include "game.h"
#include "search.h"
#include "types.h"
#include "game.h"
#include "eval.h"
#include "zobrist.h"
#include <stdint.h>


static inline int16_t score_move(Game * game, SearchStack * stack, ThreadData * td, int ply, Move m ){

    int16_t score = 0;

    if (is_cap(game, m) || move_type(m) == PROMOTION){
        PieceType p = move_piece(game, m);
        PieceType cp = move_cp(game, m);
        if (move_type(m) == ENPASSANT) cp = PAWN;
        score += PVAL[cp] * 3 + td->cap_hist[p][move_to(m)][cp];
    } else {
        PieceType p = move_piece(game, m);
        uint8_t from = move_from(m);
        uint8_t to = move_to(m);
        Side side = game->side_to_move;

        score += td->history[side][from_to(m)];

        const int ply_minus[4] = { 1, 2, 4, 6 };
        double scale[4] = { sp.chist1_scale, sp.chist2_scale, sp.chist4_scale, sp.chist6_scale }; 
        for (int i = 0; i < 4; i++){
            score = MAX(MIN(score + (*(stack - ply_minus[i])->ch)[move_piece(game, m)][to] / scale[i], INT16_MAX), INT16_MIN);
        }
    }
    return score;

}


static inline void init_ordering(uint8_t ordering[256], uint8_t move_count){
    
    for (int i = 0; i < move_count; i++){
        ordering[i] = i;
    }
}



static inline void init_move_picker(MovePicker * mp){
    for (int i = 0; i < mp->move_count; i++){
        mp->scores[i] = SCORE_NONE;
        mp->ordering[i] = i;
    }
    mp->current_index = 0;
    
}

static inline bool move_is_eligible(Game * game, Move move, int index, MovePicker * mp, SearchStack * stack, int ply){

    switch (mp->stage){
        case PICK_TT_MOVE:
            return move == mp->tt_move;
            break;
        case PICK_GOOD_CAP:
            if (is_cap(game, move)){
                if (see(game, move, sp.mp_goodcap_margin)) {
                    return true;
                }
            }
            break;
        case PICK_PROMO:
            return is_promo(move);
            break;
        case PICK_KILLER:
            if (!is_cap(game, move)){
                    
                return move == stack[ply].killers[0] || move == stack[ply].killers[1];
            }
            break;
        case PICK_QUIET:
            return !is_cap(game, move);
            break;
        case PICK_BAD_CAP:
            if (is_cap(game, move)){
                return true;
            }
            break;
        default: break;
    }
    return false;
}

// returns -1 if none eligible
static inline int16_t pick_next_move(Game * game, MovePicker * mp, SearchStack * stack, ThreadData * td, int ply){

    int c = mp->current_index;
    int best = c;
    int16_t best_score = INT16_MIN;

    int eligible_count = 0;
    for (int i = c; i < mp->move_count; i++) {
        int m = mp->ordering[i];
        if (move_is_eligible(game, mp->moves[m], m, mp, stack, ply)){
            eligible_count += 1;
            if (mp->scores[m] == SCORE_NONE){
                mp->scores[m] = score_move(game, stack, td, ply, mp->moves[m]);
            }

            if (mp->scores[m] > best_score) {
                best_score = mp->scores[m];
                best = i;
            }
        
        }
            
    }


    int16_t index = -1;
    if (eligible_count){
        int tmp = mp->ordering[c];
        mp->ordering[c] = mp->ordering[best];
        mp->ordering[best] = tmp;

        index = mp->ordering[c];
        mp->current_index++;
    }
    return index;

}




#endif
