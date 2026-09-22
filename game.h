#ifndef GAME_H
#define GAME_H

// #include "move_generation.h"
// #include "move_generation.h"
#include "game_old.h"
#include "magic.h"
#include "types.h"
#include "math.h"
#include "utils.h"
#include "zobrist.h"
#include "magic.h"
// #include "eval_constants.h"
#include <stddef.h>
// #include "tuner.h"


static inline bool material_is_lone_king(Game * game, Side side){
    if (game->pieces[side][KNIGHT] == 0 && game->pieces[side][BISHOP] == 0 && game->pieces[side][ROOK] == 0 && game->pieces[side][QUEEN] == 0) return true;
    return false;
}
// must be called before making a move
static inline bool is_double_push(Game * game, Move m){
    uint8_t from = move_from(m);
    if (move_type(m) != NORMAL || game->piece_at[from] != PAWN) return false;
    uint8_t to = move_to(m);
    if (abs(from - to) == 16) return true;
    return false;
}

static inline uint64_t moves_at(Side side, uint8_t sq, PieceType p, uint64_t occ){
    
    switch (p){
        case PAWN:
            return pawn_captures[side][sq];
            break;
        case KNIGHT:
            return knight_moves[sq];
            break;
        case BISHOP:
            return fetch_bishop_moves(sq, occ);
            break;
        case ROOK:
            return fetch_rook_moves(sq, occ);
            break;
        case QUEEN:
            return fetch_queen_moves(sq, occ);
            break;
        case KING:
            return king_moves[sq];
            break;
        default:
            return 0;
    }
}


static inline bool move_causes_check(Game * game, Move m){
    PieceType p = move_piece(game, m);

    MoveType mt = move_type(m);
    uint8_t from = move_from(m), to = move_to(m);
    if (bits[to] & game->st->ci.check_sq[p] && mt != PROMOTION) {
        return true;
    }

    if ((game->st->ci.blockers_for_king[!game->side_to_move] & bits[from])
        && !is_aligned(from, to, game->st->k_sq[!game->side_to_move])) {
        return true;
    }
    switch (mt){
        case NORMAL: break;
        case ENPASSANT:
            {
                uint64_t occ = game->board_pieces[BOTH];
                occ ^= from;
                occ |= to;
                occ ^= pawn_moves[!game->side_to_move][to];
                return (fetch_rook_moves(game->st->k_sq[!game->side_to_move], occ) & (game->pieces[game->side_to_move][ROOK] | game->pieces[game->side_to_move][QUEEN])) |
                (fetch_bishop_moves(game->st->k_sq[!game->side_to_move], occ) & (game->pieces[game->side_to_move][BISHOP] | game->pieces[game->side_to_move][QUEEN]));
            }
        case CASTLE:
            {
                CastleSide cs = (king_end_locations[game->side_to_move][KINGSIDE] == to ? KINGSIDE : QUEENSIDE);
                uint8_t rfrom = rook_castle_locations[game->side_to_move][cs][START], rto = rook_castle_locations[game->side_to_move][cs][END];
                return (rook_moves[rto] & game->pieces[!game->side_to_move][KING]) && (fetch_rook_moves(rto, game->board_pieces[BOTH] ^ rfrom ^ from | to | rto) & game->pieces[!game->side_to_move][KING]);
            }
            break;
        case PROMOTION:
            return moves_at(game->side_to_move, to, move_promotion_type(m), game->board_pieces[BOTH] ^ from) & game->pieces[!game->side_to_move][KING];
        default: break;
    }



    return false;
}

static inline uint64_t pawn_attacks(Side side, uint64_t bb){
    
    uint64_t ra = !side ? (bb << 7 & ~file_masks[7]) : bb >> 9 & ~file_masks[7];
    uint64_t la = !side ? (bb << 9 & ~file_masks[0]) : bb >> 7 & ~file_masks[0];
    return ra | la;
}

static inline uint64_t pawn_pushes(Side side, uint64_t bb, uint64_t occ){
    
    uint64_t pushes = !side ? (bb << 8 & ~occ) : bb >> 8 & ~occ;
    uint64_t double_pushes = !side ? (pushes << 8 & double_push_masks[side] &~occ) : (pushes >> 8 & double_push_masks[side] & ~occ);
    return pushes | double_pushes;
}

// we take in a side to determine which enemy sliders for this function, since we don't have an efficient way to infer both color pieces without having extra instructions
static inline uint64_t slider_blockers(Game * game, Side side, uint64_t sliders, uint8_t sq, uint64_t * pinners){

    uint64_t blockers = 0;
    *pinners = 0;

    
    uint64_t p_sliders =
        rook_moves[sq] & (game->pieces[side][QUEEN] | game->pieces[side][ROOK]) |
        bishop_moves[sq] & (game->pieces[side][QUEEN] | game->pieces[side][BISHOP]) & sliders;
    
    uint64_t occ = game->board_pieces[BOTH] ^ p_sliders;
    while (p_sliders){
        int pos = __builtin_ctzll(p_sliders);
        p_sliders = p_sliders & (p_sliders - 1);
        uint64_t b = between_sq[sq][pos] & occ;
        if (b && !more_than_one(b)){
            blockers |= b;
            if (b & game->board_pieces[!side]){
                *pinners |= bits[pos];
            }
        }
        
    }

    return blockers;
}

static inline void compute_check_info(Game * game, StateInfo * st){
    
    assert(st == game->st);
    st->ci.blockers_for_king[WHITE] = slider_blockers(game, BLACK, game->board_pieces[BLACK], st->k_sq[WHITE], &st->pinners[BLACK]);
    st->ci.blockers_for_king[BLACK] = slider_blockers(game, WHITE, game->board_pieces[WHITE], st->k_sq[BLACK], &st->pinners[WHITE]);
    
    uint8_t ksq = st->k_sq[!game->side_to_move];
    st->ci.check_sq[PAWN] = pawn_captures[!game->side_to_move][ksq];
    st->ci.check_sq[KNIGHT] = knight_moves[ksq];
    st->ci.check_sq[BISHOP] = fetch_bishop_moves(ksq, game->board_pieces[BOTH]);
    st->ci.check_sq[ROOK] = fetch_rook_moves(ksq, game->board_pieces[BOTH]);
    st->ci.check_sq[QUEEN] = st->ci.check_sq[BISHOP] | st->ci.check_sq[ROOK];
    
    
}


/*
    @brief used to mainly detect checking moves. ordered in roughly the order I would assume to be most efficient 
    @param side the ATTACKING side
    @param index the square we are checking attacks to
*/

static inline bool is_square_attacked(Game * game, Side side, int index){
    
    if (pawn_captures[!side][index] & game->pieces[side][PAWN]) {
        return true;
    }
    if (knight_moves[index] & game->pieces[side][KNIGHT]) {
        return true;
    }
    uint64_t bishop_rays = fetch_bishop_moves(index, game->board_pieces[BOTH]);
    
    if (bishop_rays & game->pieces[side][BISHOP]) {
        return true;
    }
    
    uint64_t rook_rays = fetch_rook_moves(index, game->board_pieces[BOTH]);
    
    if (rook_rays & game->pieces[side][ROOK]) {
        return true;
    }
    
    if ((bishop_rays | rook_rays) & game->pieces[side][QUEEN]) {
        return true; 
    }


    if (king_moves[index] & game->pieces[side][KING]) {
        return true;
    }
    

    return false;
    
}

/* @brief determines if a side is in check. used during make / undo move to check for legality and int main search to determine reductions */

static inline bool in_check(Game * game, Side side){
    
    return is_square_attacked(game, (Side)!side, __builtin_ctzll(game->pieces[side][KING]));

}


static inline void mv_pc(Game * game, Side side, PieceType p, uint8_t from, uint8_t to){
    game->piece_at[from] = PIECE_NONE;
    game->piece_at[to] = p;
    game->piece_index[to] = game->piece_index[from];
    game->piece_list[side][p][game->piece_index[to]] = to;
    
}

static inline void rm_pc(Game * game, Side side, PieceType p, uint8_t s){
    uint8_t last_sq = game->piece_list[side][p][--game->piece_count[side][p]];
    ASSERT(last_sq != SQ_NONE);
    uint8_t index = game->piece_index[s];
    game->piece_index[last_sq] = index;
    game->piece_list[side][p][index] = last_sq;
    game->piece_list[side][p][game->piece_count[side][p]] = SQ_NONE;
    game->piece_at[s] = PIECE_NONE;

    
    
}
static inline void pt_pc(Game * game, Side side, PieceType p, uint8_t s){
    game->piece_index[s] = game->piece_count[side][p]++;
    game->piece_list[side][p][game->piece_index[s]] = s;
    game->piece_at[s] = p;
}

/* @brief handles polyglot hash key / tt updates, bitboard, and "piece_at[][]" changes. also stores history, and replaces last_move which is used for refutations near the end of the function. */

static inline void undo_move(Game * game, Move m, SearchStack * stack){

    Side enemy_side = (Side)game->side_to_move;
    Side side = (Side)!enemy_side;
    uint8_t from = move_from(m);
    uint8_t to = move_to(m);
    MoveType mt = move_type(m);
    assert(game->piece_at[from] == PIECE_NONE);
   PieceType p = game->piece_at[to];
    PieceType cp = stack->cap_piece;
    uint8_t id = stack->p_id;
    uint8_t cap_id = stack->cap_id;

    // assert(p == game->piece_at[to] || mt == PROMOTION);


    uint64_t * piece_board = &game->pieces[side][p];
    uint64_t * side_to_move_pieces = &game->board_pieces[side];
    uint64_t * other_side_pieces = &game->board_pieces[enemy_side];
    uint64_t our_pieces = *side_to_move_pieces;
    uint64_t their_pieces = *other_side_pieces;
    PieceType * piece_at = game->piece_at;


    bool cap = cp != PIECE_NONE;


    // assert((game->board_pieces[WHITE] & game->board_pieces[BLACK]) == 0);

    switch(mt){
        case NORMAL:
            {

                uint64_t move_board = (bits[from] | bits[to]);
                *piece_board ^= move_board;

                *side_to_move_pieces ^= move_board;
                // assert(game->piece_at[from] == PIECE_NONE);

                piece_at[from] = p;

                mv_pc(game, side, p, to, from);
                if (cap){
                    
                    game->pieces[enemy_side][cp] |= bits[to]; 
                    *other_side_pieces |= bits[to];
                    game->phase += phase_values[cp];
                    ASSERT(piece_at[to] == PIECE_NONE);
                    piece_at[to] = cp;

                    pt_pc(game, enemy_side, cp, to);
                }
                
                break;

            }
        case ENPASSANT:
            {
                // TODO optimize this by precomputing these bits and literally looking them up in an array, this is unnecessary
                uint8_t capture_square = to + push_direction[enemy_side] * 8;
                uint64_t move_board = (bits[from] | bits[to]);
                uint64_t cap_sq = bits[capture_square];
                
                *piece_board ^= move_board;

                game->pieces[!side][cp] |= cap_sq;

                *side_to_move_pieces ^= move_board;

                *other_side_pieces |= cap_sq;
                
                game->phase += phase_values[cp];
                piece_at[from] = p;
                piece_at[to] = PIECE_NONE;
                piece_at[capture_square] = cp;

                    
                mv_pc(game, side, p, to, from);
                pt_pc(game, enemy_side, cp, capture_square);
            }
            break;
        case CASTLE:
            {

                CastleSide cs = (king_end_locations[side][KINGSIDE] == to ? KINGSIDE : QUEENSIDE);
                uint64_t move_board = (bits[from] | bits[to]);
                uint8_t rfrom = rook_castle_locations[side][cs][START], rend = rook_castle_locations[side][cs][END];
                uint64_t cb = 
                    bits[rfrom] | bits[rend];
                    
                *piece_board ^= move_board;

                game->pieces[side][ROOK] ^= cb;

                *side_to_move_pieces ^= cb | move_board;
                


                mv_pc(game, side, p, to, from);
                mv_pc(game, side, ROOK, rend, rfrom);
            }
            break;
        case PROMOTION:
            {

                uint64_t s = bits[from];
                uint64_t e = bits[to];
                
                game->pieces[side][PAWN] |= s;

                game->pieces[side][p] ^= e;

                *side_to_move_pieces ^= (s | e);
                rm_pc(game, side, p, to);
                pt_pc(game, side, PAWN, from);

                if (cap){
                   
                    game->pieces[enemy_side][cp] |= e;
                    
                    *other_side_pieces |= e;
                

                    game->phase += phase_values[cp];
                    piece_at[to] = cp;

                    pt_pc(game, enemy_side, cp, to);
                    
                }


                game->phase -= phase_values[p];
                piece_at[from] = PAWN;


            }
            break;
    }


    stack->current_move = 0;
    stack->moved_piece = PIECE_NONE;
    stack->cap_piece = PIECE_NONE;
    game->board_pieces[BOTH] = game->board_pieces[WHITE] | game->board_pieces[BLACK];
    game->side_to_move = side;
    game->key_history[game->history_count] = 0;
    game->history_count -= 1;
    // game->fifty_move_clock = game->fifty_move_clock_was;

    game->st = game->st->pst;
    
}




/* @brief handles polyglot hash key / tt updates, bitboard, and "piece_at[][]" changes. also stores history, and replaces last_move which is used for refutations near the end of the function. */



// returns if move was valid - if false, undo this move
static inline bool make_move(Game * game, Move m, StateInfo * st, SearchStack * stack){

    
    memcpy(st, game->st, sizeof(StateInfo));
    st->pst = game->st;
    game->st = st;
    uint64_t k = st->key;

    
    Side side = game->side_to_move;
    Side enemy_side = (Side)!side;
    uint8_t from = move_from(m);
    uint8_t to = move_to(m);
    MoveType mt = move_type(m);
    PieceType p = game->piece_at[from];
    // assert(p < PIECE_NONE);
    // assert(p == game->piece_info[game->sq_to_uid[from]].p);
    // assert(bits[from] & game->pieces[side][p]);
    PieceType cp = game->piece_at[to];
    int en_passant_index = st->en_passant_index;
    uint64_t blockers = game->board_pieces[BOTH];
    uint64_t * piece_board = &game->pieces[side][p];
    uint64_t * side_to_move_pieces = &game->board_pieces[side];
    uint64_t * other_side_pieces = &game->board_pieces[!side];
    uint64_t our_pieces = *side_to_move_pieces;
    uint64_t their_pieces = *other_side_pieces;
    PieceType * piece_at = game->piece_at;
    bool dp = false;
    if (p == PAWN){
        dp = is_double_push(game, m);
    }

    bool cap = cp != PIECE_NONE || mt == ENPASSANT;



    if (en_passant_index != -1){
        if (pawn_captures[enemy_side][en_passant_index] & game->pieces[side][PAWN]) {
        
            k ^= get_en_passant_random(en_passant_index);
        }
        
    }

    
    switch(mt){
        case NORMAL:
            {


                uint64_t move_board = (bits[from] | bits[to]);
                *piece_board ^= move_board;

                *side_to_move_pieces ^= move_board;

                piece_at[to] = p;
                piece_at[from] = PIECE_NONE;
                
                st->psqt_score[side] -= PSQT[side][p][from];
                st->psqt_score[side] += PSQT[side][p][to];

                if (cap){
                    
                    // assert(cp < PIECE_NONE);
                    game->pieces[enemy_side][cp] ^= bits[to]; 
                    *other_side_pieces ^= bits[to];
                    game->phase -= phase_values[cp];
                    st->psqt_score[enemy_side] -= PSQT[enemy_side][cp][to];
                    st->material_score[enemy_side] -= eval_params[ep_idx.piece_values[cp]];
                    blockers ^= bits[from];

                    k ^= get_piece_random(cp, enemy_side, to);
                    st->piece_key[cp] ^= piece_random[enemy_side][to];

                    st->material_key ^= material_randoms[enemy_side][cp][game->piece_count[enemy_side][cp]];
                    rm_pc(game, enemy_side, cp, to);
                    st->material_key ^= material_randoms[enemy_side][cp][game->piece_count[enemy_side][cp]];

                    if (cp != PAWN){
                        st->nonpawn_key[enemy_side] ^= piece_random[enemy_side][to];
                    }
                } else {
                    
                    blockers ^= move_board;
                }

                k ^= get_piece_random(p, side, from);
                k ^= get_piece_random(p, side, to);
                st->piece_key[p] ^= piece_random[side][from];
                st->piece_key[p] ^= piece_random[side][to];
                if (p == KING){
                    st->k_sq[side] = to;
                }
                if (p != PAWN){
                    st->nonpawn_key[side] ^= piece_random[side][from];
                    st->nonpawn_key[side] ^= piece_random[side][to];
                }
    
                
                mv_pc(game, side, p, from, to);
                break;
            }
        case ENPASSANT:
            {
                uint8_t capture_square = to + push_direction[enemy_side] * 8;
                cp = PAWN;
                // assert(game->piece_at[capture_square] == PAWN);
                
                uint64_t move_board = (bits[from] | bits[to]);
                uint64_t cap_board = bits[capture_square];
                *piece_board ^= move_board;

                game->pieces[enemy_side][cp] ^= cap_board;

                *side_to_move_pieces ^= move_board;

                *other_side_pieces ^= cap_board;
                game->phase -= phase_values[cp];

                piece_at[to] = p;
                piece_at[from] = PIECE_NONE;
                piece_at[capture_square] = PIECE_NONE;


                st->psqt_score[side] -= PSQT[side][p][from];
                st->psqt_score[side] += PSQT[side][p][to];

                st->psqt_score[enemy_side] -= PSQT[enemy_side][cp][capture_square];
                st->material_score[enemy_side] -= eval_params[ep_idx.piece_values[cp]];


                blockers ^= (move_board | cap_board);

                k ^= get_piece_random(PAWN, enemy_side, capture_square);
                st->piece_key[PAWN] ^= piece_random[enemy_side][capture_square];

                st->material_key ^= material_randoms[enemy_side][cp][game->piece_count[enemy_side][cp]];
                rm_pc(game, enemy_side, cp, capture_square);
                st->material_key ^= material_randoms[enemy_side][cp][game->piece_count[enemy_side][cp]];


                k ^= get_piece_random(p, side, from);
                k ^= get_piece_random(p, side, to);
                st->piece_key[p] ^= piece_random[side][from];
                st->piece_key[p] ^= piece_random[side][to];
    
                mv_pc(game, side, p, from, to);
            }
            break;
        case CASTLE:
            {
                CastleSide cs = (king_end_locations[side][KINGSIDE] == to ? KINGSIDE : QUEENSIDE);
                uint64_t move_board = (bits[from] | bits[to]);
                uint8_t rfrom = rook_castle_locations[side][cs][START], rend = rook_castle_locations[side][cs][END];
                uint64_t cb = 
                    bits[rfrom] | bits[rend];

                *piece_board ^= move_board;

                game->pieces[side][ROOK] ^= cb;


                *side_to_move_pieces ^= cb | move_board;

                st->psqt_score[side] -= PSQT[side][p][from];
                st->psqt_score[side] += PSQT[side][p][to];
                st->psqt_score[side] -= PSQT[side][ROOK][rfrom];
                st->psqt_score[side] += PSQT[side][ROOK][rend];

                piece_at[to] = p;
                piece_at[from] = PIECE_NONE;
                piece_at[rend] = ROOK;
                piece_at[rfrom] = PIECE_NONE;

                blockers ^= cb | move_board;

                // king
                k ^= get_piece_random(p, side, from);
                k ^= get_piece_random(p, side, to);
                st->piece_key[p] ^= piece_random[side][from];
                st->piece_key[p] ^= piece_random[side][to];
                // assert(st->k_sq[side] == from);
                st->k_sq[side] = to;
                st->nonpawn_key[side] ^= piece_random[side][from];
                st->nonpawn_key[side] ^= piece_random[side][to];

                // rook
                k ^= get_piece_random(ROOK, side, rfrom);
                k ^= get_piece_random(ROOK, side, rend);
                st->piece_key[ROOK] ^= piece_random[side][rfrom];
                st->piece_key[ROOK] ^= piece_random[side][rend];
                st->nonpawn_key[side] ^= piece_random[side][rfrom];
                st->nonpawn_key[side] ^= piece_random[side][rend];
    
                mv_pc(game, side, p, from, to);
                mv_pc(game, side, ROOK, rfrom, rend);
                

            }
            break;
        case PROMOTION:
            {

                // assert(p == PAWN);
                uint64_t s = bits[from];
                uint64_t e = bits[to];
                PieceType promo = move_promotion_type(m);
                // assert(promo < PIECE_NONE);
                
                *piece_board ^= s;

                game->pieces[side][promo] |= e;

                *side_to_move_pieces ^= (s | e);
                blockers ^= (s | e);

                if (cap){
                    
                    // assert(cp < PIECE_NONE);
                    // assert(e & game->pieces[!side][cp]);
                    // assert(cp == game->piece_info[game->sq_to_uid[to]].p);
                    game->pieces[enemy_side][cp] ^= e;                    
                    *other_side_pieces ^= e;
                    blockers ^= e;
                
                    st->psqt_score[enemy_side] -= PSQT[enemy_side][cp][to];
                    st->material_score[enemy_side] -= eval_params[ep_idx.piece_values[cp]];

                    game->phase -= phase_values[cp];
                    k ^= get_piece_random(cp, enemy_side, to);
                    st->piece_key[cp] ^= piece_random[enemy_side][to];

                    st->material_key ^= material_randoms[enemy_side][cp][game->piece_count[enemy_side][cp]];
                    rm_pc(game, enemy_side, cp, to);
                    st->material_key ^= material_randoms[enemy_side][cp][game->piece_count[enemy_side][cp]];

                    // a promotion capture cannot capture an enemy pawn as that would be illegal, therefore we do not check for nonpawn update

                    st->nonpawn_key[enemy_side] ^= piece_random[enemy_side][to];

                    
                }

                st->psqt_score[side] -= PSQT[side][p][from];
                st->psqt_score[side] += PSQT[side][promo][to];
                st->material_score[side] -= eval_params[ep_idx.piece_values[p]];
                st->material_score[side] += eval_params[ep_idx.piece_values[promo]];


                game->phase += phase_values[promo];
                piece_at[to] = promo;
                piece_at[from] = PIECE_NONE;


                k ^= get_piece_random(p, side, from);
                k ^= get_piece_random(promo, side, to);
                st->piece_key[p] ^= piece_random[side][from];
                st->piece_key[promo] ^= piece_random[side][to];

                st->nonpawn_key[side] ^= piece_random[side][to];

                st->material_key ^= material_randoms[side][p][game->piece_count[side][p]];
                rm_pc(game, side, p, from);
                st->material_key ^= material_randoms[side][p][game->piece_count[side][p]];

                st->material_key ^= material_randoms[side][promo][game->piece_count[side][promo]];
                pt_pc(game, side, promo, to);
                st->material_key ^= material_randoms[side][promo][game->piece_count[side][promo]];
                // ASSERT(game->piece_count[side][promo] == __builtin_popcountll(game->pieces[side][promo]));
    

            }
            break;
    }


    if (dp){
        st->en_passant_index = -push_direction[side] * 8 + to;
        // if we create an en passant square, we need to check for the special polyglot square as well

        if (game->pieces[enemy_side][PAWN] & pawn_captures[side][st->en_passant_index]){
            k ^= get_en_passant_random(st->en_passant_index);
        }

        
    } else {
        st->en_passant_index = -1;
    }

    uint8_t cr = castling_rights_masks[from] | castling_rights_masks[to];

    uint8_t ct = cr & st->castle_flags;
    while (ct){
        uint8_t c = __builtin_ctz(ct);
        ct = ct & (ct - 1);
        k ^= Random64[CASTLING_OFFSET + c];
        
    }
    st->castle_flags &= ~cr;
    
    stack->current_move = m;
    stack->moved_piece = p;
    stack->cap_piece = cp;
    k ^= get_turn_random();
    st->key = k;
    game->board_pieces[BOTH] = blockers;
    game->side_to_move = enemy_side;

    game->key_history[game->history_count] = k;
    game->history_count += 1;
    

    game->fifty_move_clock_was = game->fifty_move_clock;
    if (p == PAWN || cap){
        st->rule50 = 0;        
    } else {
        st->rule50 = st->pst->rule50 + 1;        
    }
    // assert((game->board_pieces[WHITE] & game->board_pieces[BLACK]) == 0);
    // if(!check_hash(game)){
    //     printf("HASH WRONG: %lx\n", game->st->key);
    //     printf("CORRECT: %lx\n", create_zobrist_from_scratch(game));
    // }
    // ASSERT(check_hash(game));
    // ASSERT(game->st->material_key == create_material_hash_from_scratch(game));
    // for (int i = 0; i < PIECE_TYPES;i++){
    //     ASSERT(game->st->piece_key[i] == create_piece_hash_from_scratch(game, (PieceType)i));
    // }



    // precompute checking info
    
    compute_check_info(game, st);
    
    return !in_check(game, side);
}



/* @brief swaps rank due to board orientation and then returns the new position */

static inline int swap_position_side(int pos){
    
    int new_pos = pos;
    int r = abs(7 - (pos / 8));
    int f = pos % 8;
    new_pos = r * 8 + f;
    return new_pos;
        
}


int set_board_to_fen(Game * game, char fen[MAX_FEN]);




static inline void swap_move(Move *a, Move *b){
    Move t = *a; *a = *b; *b = t;
}



/* @brief checks against our hash key history to avoid drawing when we are up in material. if we are down in eval, then our engine naturally plays to draw on purpose. */

static inline bool three_fold_repetition(Game * game, uint64_t key){
    int count = 0;
    for (int i = 0; i < game->history_count; i++){
        if (key == game->key_history[i]){
            count += 1;
        }
    }
    if (count >= 3){
        return true;
    }
    return false;
}

static inline void dump_stack_moves(SearchStack * stack, int ply){
    for (int i = 0; i < ply; i++){
        print_move_full(stack[i].current_move);
    }
}
static inline void dump_state_keys(const StateInfo * st){
    
    const StateInfo* cur = st;
    int count = 0;

    for (const StateInfo* s = st; s; s = s->pst) {
        printf("%d: %lx\n", count, s->key);
        count++;
    }
}

static inline bool draw_by_insufficient_material(Game * game){
    if (game->pieces[WHITE][PAWN] || game->pieces[BLACK][PAWN] || game->pieces[WHITE][QUEEN] || game->pieces[WHITE][ROOK] || game->pieces[BLACK][QUEEN] || game->pieces[BLACK][ROOK]) return false;

    bool lk_w = material_is_lone_king(game, WHITE);
    bool lk_b = material_is_lone_king(game, BLACK);
    ASSERT(game->pieces[WHITE][PAWN] == 0 && game->pieces[BLACK][PAWN] == 0);
    if (lk_w && lk_b) return true;

    if (lk_w){
        bool b = game->pieces[BLACK][BISHOP], n = game->pieces[BLACK][KNIGHT];
        if (b && !more_than_one(game->pieces[BLACK][BISHOP]) && !n){
            return true;
        }
        if (n && !more_than_one(game->pieces[BLACK][KNIGHT]) && !b){
            return true;
        }
    }
    if (lk_b){
        bool b = game->pieces[WHITE][BISHOP], n = game->pieces[WHITE][KNIGHT];
        if (b && !more_than_one(game->pieces[WHITE][BISHOP]) && !n){
            return true;
        }
        if (n && !more_than_one(game->pieces[WHITE][KNIGHT]) && !b){
            return true;
        }
    }
    return false;
}


static inline bool threefold(const StateInfo* st) {
    const StateInfo* cur = st;
    int count = 0;

    for (const StateInfo* s = st->pst; s; s = s->pst) {

        if (s->key == cur->key) {
            count++;
            if (count >= 2){
                return true;
                
            }
        }
    }

    return false;
}

static inline bool three_fold_repetition_root(Game * game, uint64_t key){

    int count = 0;
    for (int i = 0; i < game->history_count; i++){
        if (key == game->key_history[i]){
            count += 1;
        }
    }
    if (count >= 3){
        return true;
    }
    return false;
}


/* @brief stores pv into the triangular pv table. you can find this implementation from code monkey king / chess programming wiki */

static inline void store_pv(int ply, Move move, SearchData * search_data) {
    if (ply >= 63) return;
    search_data->pv_table[ply][0] = move;

    for (int i = 0; i < search_data->pv_length[ply + 1]; i++) {
        search_data->pv_table[ply][i + 1] = search_data->pv_table[ply + 1][i];
    }

    search_data->pv_length[ply] = search_data->pv_length[ply + 1] + 1;
}



/* 
    @brief determines if a piece is pinned
    @param pinned_to_sq the square we are checking for pins TO 
*/


static inline uint64_t piece_is_pinned(Game * game, Side side, int pinned_to_sq){
    uint64_t occ = game->board_pieces[BOTH];
    uint64_t bishop_rays = fetch_bishop_moves(pinned_to_sq, occ);
    uint64_t rook_rays = fetch_rook_moves(pinned_to_sq, occ);
    uint64_t our_pieces = game->board_pieces[side];

    uint64_t rook_blockers = rook_rays & our_pieces;
    uint64_t bishop_blockers = bishop_rays & our_pieces;

    uint64_t rooks_and_queens = game->pieces[!side][QUEEN] | game->pieces[!side][ROOK];
    uint64_t bishops_and_queens = game->pieces[!side][QUEEN] | game->pieces[!side][BISHOP];

    uint64_t pinned = 0;

    while (rook_blockers){
        int pos = pop_lsb(&rook_blockers);
        if (game->piece_at[pos] == PAWN) continue;
        uint64_t ray = fetch_rook_moves(pinned_to_sq, occ ^ (bits[pos]));
        if (ray & rooks_and_queens) pinned |= (bits[pos]);
    }
    while (bishop_blockers){
        int pos = pop_lsb(&bishop_blockers);
        if (game->piece_at[pos] == PAWN) continue;
        uint64_t ray = fetch_bishop_moves(pinned_to_sq, occ ^ (bits[pos]));
        if (ray & bishops_and_queens) pinned |= (bits[pos]);
    }

    return pinned;
}


static inline void compute_pin_masks(Game * game, Side side, MovementMasks * masks){


    int ksq = 0;
    int qsq = 0;
    uint64_t our_pieces = game->board_pieces[side];
    uint64_t enemy_pieces = game->board_pieces[!side];
    uint64_t queens = game->pieces[side][QUEEN];
    uint64_t pins = 0;
    while (queens){
        int pos = pop_lsb(&queens);
        masks->our_pinned_pieces |= piece_is_pinned(game, side, pos);
    }
    ksq = bit_scan_forward(&game->pieces[side][KING]);
    masks->our_pinned_pieces |= piece_is_pinned(game, side, ksq);
    
    int e_ksq = 0;
    int e_qsq = 0;
    uint64_t e_queens = game->pieces[!side][QUEEN];
    uint64_t e_pins = 0;
    while (e_queens){
        int pos = pop_lsb(&e_queens);
        masks->enemy_pinned_pieces |= piece_is_pinned(game, (Side)!side, pos);
    }
    e_ksq = bit_scan_forward(&game->pieces[!side][KING]);
    masks->enemy_pinned_pieces |= piece_is_pinned(game, (Side)!side, e_ksq);


    
}

/* @brief only used during board setup, simply puts a piece on its board and updates piece_at */
static inline void set_piece(Game * game, uint64_t * board, Side side, PieceType type, int index){
    game->pieces[side][type] |= 1ULL << index;
    game->piece_at[index] = type;
    game->piece_index[index] = game->piece_count[side][type];
    game->piece_list[side][type][game->piece_count[side][type]] = index;
    game->piece_count[side][type] += 1;

}


// basically just makes a state for the root node so that we don't dereference garbage
static inline void init_st(Game * game, StateInfo * st){

    game->st = st;
    memcpy(st, &game->os, sizeof(StateInfo));
    compute_check_info(game, st);
}


/* @brief main input loop, commands in types.h, leads to uci input if "uci" is invoked */

void handle_input(Game * game);

/* @brief sets up a blank new game from startpos */

int init_new_game(Game * game, Side color);


/* @brief sets up a blank new game from fen */

void init_game_from_fen(Game * game, Side color, char fen[MAX_FEN]);

/* @brief prints game to fen, invoked using "display" command */

void output_game_to_fen(Game * game, char fen[MAX_FEN]);

#endif
