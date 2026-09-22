#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"
#include "eval.h"
#include "utils.h"


static inline void decay_history_table(){
    
    for (int i = 0; i < COLOR_MAX; i++){
        for (int p = 0; p < PIECE_TYPES; p++){

            for (int f = 0; f < 64; f++){
                for (int t = 0; t < 64; t++){
                    // history[i][p][f][t] >>= 2;
                }
            }
        }
    }
}
static inline void decay_conthist(){
    
    for (int i = 0; i < COLOR_MAX * 64 * PIECE_TYPES; i++){
        for (int d = 0; d < COLOR_MAX * 64 * PIECE_TYPES; d++){
                // conthist[i][d] >>= 2;
        }
    }
}

static inline void decay_cap_hist(){
    
    for (int i = 0; i < PIECE_TYPES; i++){
        for (int b = 0; b < BOARD_MAX; b++){
            for (int p = 0; p < PIECE_TYPES; p++){

                // capture_history[i][b][p] >>= 2;
            }
        }
    }
}

// static inline void clear_conthist(){
    
//     memset(conthist, 0, sizeof(int16_t) *(COLOR_MAX * PIECE_TYPES * BOARD_MAX) * COLOR_MAX * PIECE_TYPES * BOARD_MAX);
//     // for (int i = 0; i < COLOR_MAX; i++){
//     //     for (int p = 0; p < PIECE_TYPES; p++){
//     //         for (int f = 0; f < 64; f++){
//     //             conthist[i * 64 * 6 + p * 64 + f][] = 0;
//     //         }
//     //     }
//     // }
// }
// static inline void clear_history(){
    
//     for (int i = 0; i < COLOR_MAX; i++){
//         for (int p = 0; p < PIECE_TYPES; p++){
//             for (int f = 0; f < 64; f++){
//                 for (int t = 0; t < 64; t++){
//                     history[i][p][f][t] = 0;
//                 }
//             }
//         }
//     }
// }

// static inline void clear_capture_history(){
    
//     memset(capture_history, 0, sizeof(int16_t) * PIECE_TYPES * BOARD_MAX * PIECE_TYPES);
// }

static inline void update_corrhist(Game * game, ThreadData * td, SearchStack * stack, Side side, int depth, int diff){
    // const int CORRHIST_GRAIN = 256;
    // const int CORRHIST_WEIGHT = 256;

    // const int CORRHIST_MAX = 256 * 32;
    int weight = MIN(16, depth + sp.corr_depth_base);
    int d = diff * sp.corrhist_grain * weight;
    int cweight = sp.corrhist_weight - weight;
    

    int16_t * p = &td->corrhist_p[side][game->st->piece_key[PAWN] & CORRHIST_MASK];
    *p = MAX(MIN(((*p * cweight + d) / sp.corrhist_weight), sp.corrhist_max), -sp.corrhist_max);
    
    int16_t * np_w_p = &td->corrhist_nonpawns_w[side][game->st->nonpawn_key[WHITE] & CORRHIST_MASK];
    *np_w_p = MAX(MIN(((*np_w_p * cweight + d) / sp.corrhist_weight), sp.corrhist_max), -sp.corrhist_max);

    int16_t * np_b_p = &td->corrhist_nonpawns_b[side][game->st->nonpawn_key[BLACK] & CORRHIST_MASK];
    *np_b_p = MAX(MIN(((*np_b_p * cweight + d) / sp.corrhist_weight), sp.corrhist_max), -sp.corrhist_max);

    int16_t * m_p = &td->corrhist_material[side][game->st->material_key & CORRHIST_MASK];
    *m_p = MAX(MIN(((*m_p * cweight + d) / sp.corrhist_weight), sp.corrhist_max), -sp.corrhist_max);
    
     uint64_t kbn = game->st->piece_key[KING] ^ game->st->piece_key[BISHOP] ^ game->st->piece_key[KNIGHT];

    int16_t * kbn_p = &td->corrhist_kbn[side][kbn & CORRHIST_MASK];
    *kbn_p = MAX(MIN(((*kbn_p * cweight + d) / sp.corrhist_weight), sp.corrhist_max), -sp.corrhist_max);

    uint64_t kqr = game->st->piece_key[KING] ^ game->st->piece_key[QUEEN] ^ game->st->piece_key[ROOK];

    int16_t * kqr_p = &td->corrhist_kqr[side][kqr & CORRHIST_MASK];
    *kqr_p = MAX(MIN(((*kqr_p * cweight + d) / sp.corrhist_weight), sp.corrhist_max), -sp.corrhist_max);


    Move m = (stack-1)->current_move;
    if (m){
        int16_t * cr = &(*(stack-2)->cr)[game->piece_at[move_to(m)]][move_to(m)];
        *cr = MAX(MIN(((*cr * cweight + d) / sp.corrhist_weight), sp.corrhist_max), -sp.corrhist_max);
    }

    
    

    
}

static inline void update_cap_hist(ThreadData * td, PieceType p, PieceType cp, int pos, int bonus){
    
    bonus = MIN(CAP_HIST_MAX, MAX(bonus, -CAP_HIST_MAX));
    
    int16_t ba = abs(bonus);

    int16_t hc = td->cap_hist[p][pos][cp];
    int16_t bh = bonus - hc * ba / CAP_HIST_MAX;
    td->cap_hist[p][pos][cp] += bh;
}

static inline void update_history(Game * game, ThreadData * td, SearchStack * stack, int ply, Move m, int bonus){

    Side side = game->side_to_move;
    uint8_t to = move_to(m);
    
    bonus = MIN(HISTORY_MAX, MAX(bonus, -HISTORY_MAX));
    
    int16_t ba = abs(bonus);

    int16_t * hc = &td->history[side][from_to(m)];
    int16_t bh = bonus - *hc * ba / HISTORY_MAX;
    *hc += bh;

    const int ply_minus[4] = {1, 2, 4, 6};
    for (int i = 0; i < 4; i++){
        int16_t c =
            (*(stack - ply_minus[i])->ch)[move_piece(game, m)][to];
        int16_t b = bonus - c * ba / HISTORY_MAX;
        (*(stack - ply_minus[i])->ch)[move_piece(game, m)][to] += b;
    }
}



static inline uint64_t attacks_from(PieceType p, Side side, int pos, uint64_t occ){
    switch (p){
        case PAWN:
            return pawn_captures[side][pos];
            break;
        case KNIGHT:
            return knight_moves[pos];
            break;
        case BISHOP:
            return fetch_bishop_moves(pos, occ);
            break;
        case ROOK:
            return fetch_rook_moves(pos, occ);
            break;
        case QUEEN:
            return fetch_queen_moves(pos, occ);
            break;
        case KING:
            return king_moves[pos];
            break;
    }
}

static inline bool check_is_dangerous(Game * game, Move m, int futility, int beta){

    PieceType p = move_piece(game, m);
    Side side = game->side_to_move;
    uint8_t start = move_from(m);
    uint8_t pos = move_to(m);
    uint8_t ek_sq = game->st->k_sq[!side];
    uint64_t occ = game->board_pieces[BOTH] ^ bits[ek_sq] ^ bits[start];

    if (king_moves[ek_sq] & bits[pos] && p == QUEEN){
        return true;
    }

    uint64_t old_moves = attacks_from(p, side, start, occ);
    uint64_t new_moves = attacks_from(p, side, pos, occ);
    if (__builtin_popcountll(king_moves[ek_sq] & ~(new_moves | bits[pos] | game->board_pieces[BOTH])) <= 1){
        return true;
    }

    // uint64_t db = (game->board_pieces[!side] ^ bits[ek_sq]) & new_moves & ~old_moves;
    // while (db){
        
    //     int sq = __builtin_ctzll(db);
    //     db = db & (db - 1);

    //     if (futility + PSQT_EG[!side][game->piece_at[sq]][sq] >= beta){
    //         return true;
    //     }
    // }


    return false;
    
}

static inline int futility_margin(bool improving, int depth){
    return (175 - 50 * improving) * depth;
}

static inline int lmp_margin(bool improving, int depth){
    return (7 + depth * depth) * (1 + improving);
    // return (5 + depth * depth);
}



static inline void init_search_data(SearchData * search_data, SearchFlags * flags,  double start_time, double max_time, int max_depth){
    
    search_data->stop = false;
    search_data->start_time = start_time;
    search_data->max_time = max_time;
    search_data->enable_time = false;
    if (flags){

        search_data->nodes_enabled = flags->nodes;
        search_data->node_max = flags->nodes;
    } else {
        search_data->nodes_enabled = 0;
        search_data->node_max = INT_MAX;
    }
    
    search_data->ply = 0;
    search_data->node_count = 0;
    memset(search_data->pv_length, 0, sizeof(search_data->pv_length));
    memset(search_data->pv_table, 0, sizeof(search_data->pv_table));
    search_data->flags.mate = false;
    search_data->flags.three_fold_repetition = false;
    search_data->flags.draw = false;
    search_data->current_best_score = 0;
    search_data->use_opening_book = true;
    search_data->has_extended = false;
    search_data->node_count = 0;
    search_data->lmrs_researched = 0;
    search_data->lmrs_tried = 0;
    search_data->aspiration_fail = 0;
    search_data->null_prunes = 0;
    search_data->ordering_success = 0;
    search_data->tt_hits= 0;
    search_data->tt_probes= 0;
    search_data->qnodes= 0;
    search_data->futility_prunes= 0;
    search_data->max_depth = max_depth;
    search_data->see_prunes = 0;
    search_data->q_see_prunes = 0;
    search_data->qdelta_prunes = 0;
    search_data->rfp = 0;
    search_data->razoring = 0;
    search_data->check_extensions = 0;
    search_data->delta_prunes = 0;
    search_data->late_move_prunes  = 0;
    search_data->pawn_hash_hits = 0;
    search_data->pawn_hash_probes = 0;
    search_data->highest_mat_reached = 0;
    search_data->fast_evals = 0;
    search_data->lazy_cutoffs_s1 = 0;
    search_data->lazy_cutoffs_s2 = 0;
    search_data->lazy_cutoffs_s3 = 0;
    search_data->check_prunes = 0;
    search_data->chist_prunes = 0;
    search_data->extensions = 0;
    search_data->reductions = 0;
}

void init_search_params();
void init_threads();
/* @brief main iterative deepening search
@param search flags, see types.h */

Move iterative_search(Game * game, SearchFlags * flags);

/* @brief perft recursion
@return node count */

uint64_t perft(Game * game, SearchStack * stack, int depth);

/* @brief perft, benches at 17m nps with hash updates, more without */

void perft_root(Game * game, int depth);

void init_search_tables();

#endif
 
