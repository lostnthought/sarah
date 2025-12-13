#ifndef GAME_H
#define GAME_H

// #include "move_generation.h"
// #include "move_generation.h"
#include "magic.h"
#include "types.h"
#include "math.h"
#include "zobrist.h"
#include "magic.h"
// #include "tuner.h"
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
    uint64_t bishop_rays = fetch_bishop_moves(game, index, game->board_pieces[BOTH]);
    
    if (bishop_rays & game->pieces[side][BISHOP]) {
        return true;
    }
    
    uint64_t rook_rays = fetch_rook_moves(game, index, game->board_pieces[BOTH]);
    
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

/* @brief handles polyglot hash key / tt updates, bitboard, and "piece_at[][]" changes. also stores history, and replaces last_move which is used for refutations near the end of the function. */

// void undo_move(Game * game, Move * move);

static inline void undo_move(Game * game, Move * move){

    Side side = move->side;
    uint64_t key = game->key;
    int start = move->start_index;
    int end = move->end_index;
    PieceType piece = move->piece;
    PieceType capture_piece = move->capture_piece;
    CastleSide castle_side = move->castle_side;
    PieceType promotion_type = move->promotion_type;
    int ep_sq = game->en_passant_index;
    uint64_t * piece_board = &game->pieces[side][piece];
    uint64_t * side_to_move_pieces = &game->board_pieces[side];
    uint64_t * other_side_pieces = &game->board_pieces[!side];
    uint64_t our_pieces = *side_to_move_pieces;
    uint64_t their_pieces = *other_side_pieces;

    

    if (move->double_push){

        // if we made an en passant square, we need to check for the special polyglot square as well

        if (game->pieces[!side][PAWN] & pawn_captures[side][ep_sq]){
            key ^= get_en_passant_random(ep_sq);
        }

        
    }
    switch(move->type){
        case MOVE:
            {

                uint64_t move_board = (1ULL << start | 1ULL << end);
                // move piece on its board
                *piece_board ^= move_board;

                // move piece on its color's board
                *side_to_move_pieces ^= move_board;


                key ^= get_piece_random(piece, side, start);
                key ^= get_piece_random(piece, side, end);
                game->piece_at[start] = piece;

                if (piece == PAWN){
                    game->pawn_key ^= pawn_random[side][start];
                    game->pawn_key ^= pawn_random[side][end];
                }

                game->psqt_evaluation_mg[side] += PSQT_MG[side][piece][start];
                game->psqt_evaluation_eg[side] += PSQT_EG[side][piece][start];
                game->psqt_evaluation_mg[side] -= PSQT_MG[side][piece][end];
                game->psqt_evaluation_eg[side] -= PSQT_EG[side][piece][end];

                

                break;

            }
        case CAPTURE:
            {

                uint64_t cap = 1ULL << end;
                uint64_t move_board = (1ULL << start | cap);
                // move piece on its board
                *piece_board ^= move_board;

                // toggle captured piece on its board
                game->pieces[!side][capture_piece] |= cap; 

                // move piece on its color's board
                *side_to_move_pieces ^= move_board;

                // toggle captured piece on its color's board
                *other_side_pieces |= cap;


                key ^= get_piece_random(piece, side, start);
                key ^= get_piece_random(piece, side, end);
                key ^= get_piece_random(capture_piece, (Side)!side, end);

                
                game->psqt_evaluation_mg[side] += PSQT_MG[side][piece][start];
                game->psqt_evaluation_eg[side] += PSQT_EG[side][piece][start];
                game->psqt_evaluation_mg[side] -= PSQT_MG[side][piece][end];
                game->psqt_evaluation_eg[side] -= PSQT_EG[side][piece][end];

                game->psqt_evaluation_mg[!side] += PSQT_MG[!side][capture_piece][end];
                game->psqt_evaluation_eg[!side] += PSQT_EG[!side][capture_piece][end];

                game->phase += phase_values[capture_piece];
                
                game->piece_at[start] = piece;
                game->piece_at[end] = capture_piece;
                if (piece == PAWN){
                    game->pawn_key ^= pawn_random[side][start];
                    game->pawn_key ^= pawn_random[side][end];
                }
                if (capture_piece == PAWN){
                    game->pawn_key ^= pawn_random[!side][end];
                    
                }
                // update_incremental_material_move(game, side, piece, start, end, true, end, capture_piece, true, our_pieces, their_pieces);
            }
            break;
        case EN_PASSANT:
            {
                int capture_square = __builtin_ctzll(pawn_moves[!side][end]);
                uint64_t move_board = (1ULL << start | 1ULL << end);
                uint64_t cap = 1ULL << capture_square;
                
                // move piece on its board
                *piece_board ^= move_board;

                // this utilizes our precomputed board masks for movement. the en passant square for each side will always have one bit forward, which is the exact bit where our pawn that we are capturing is
                // we avoid having to check if black / white and just use this mask
                game->pieces[!side][capture_piece] |= cap;

                // move piece on its color's board
                *side_to_move_pieces ^= move_board;

                // toggle captured piece on its color's board using the above trick to find it's location
                *other_side_pieces |= cap;
                



                key ^= get_piece_random(piece, side, start);
                key ^= get_piece_random(piece, side, end);
                key ^= get_piece_random(capture_piece, (Side)!side, capture_square);
                game->psqt_evaluation_mg[side] += PSQT_MG[side][piece][start];
                game->psqt_evaluation_eg[side] += PSQT_EG[side][piece][start];
                game->psqt_evaluation_mg[side] -= PSQT_MG[side][piece][end];
                game->psqt_evaluation_eg[side] -= PSQT_EG[side][piece][end];

                game->psqt_evaluation_mg[!side] += PSQT_MG[!side][capture_piece][capture_square];
                game->psqt_evaluation_eg[!side] += PSQT_EG[!side][capture_piece][capture_square];

                game->phase += phase_values[capture_piece];
                game->piece_at[start] = piece;
                game->piece_at[capture_square] = capture_piece;

                game->pawn_key ^= pawn_random[side][start];
                game->pawn_key ^= pawn_random[side][end];
                game->pawn_key ^= pawn_random[!side][capture_square];

                    
            }
            break;
        case CASTLE:
            {
                uint64_t move_board = (1ULL << start | 1ULL << end);
                uint64_t cb = 
                    1ULL << rook_castle_locations[side][castle_side][START] | 1ULL << rook_castle_locations[side][castle_side][END];
                    
                // moves king to its final location
                *piece_board ^= move_board;

                game->pieces[side][ROOK] ^= cb;

                *side_to_move_pieces ^= cb | move_board;
                

                key ^= get_piece_random(piece, side, start);
                key ^= get_piece_random(piece, side, end);
                key ^= get_piece_random(ROOK, side, rook_castle_locations[side][castle_side][START]);
                key ^= get_piece_random(ROOK, side, rook_castle_locations[side][castle_side][END]);

                game->psqt_evaluation_mg[side] += PSQT_MG[side][piece][start];
                game->psqt_evaluation_eg[side] += PSQT_EG[side][piece][start];
                game->psqt_evaluation_mg[side] -= PSQT_MG[side][piece][end];
                game->psqt_evaluation_eg[side] -= PSQT_EG[side][piece][end];

                game->psqt_evaluation_mg[side] += PSQT_MG[side][ROOK][rook_castle_locations[side][castle_side][START]];
                game->psqt_evaluation_eg[side] += PSQT_EG[side][ROOK][rook_castle_locations[side][castle_side][START]];
                game->psqt_evaluation_mg[side] -= PSQT_MG[side][ROOK][rook_castle_locations[side][castle_side][END]];
                game->psqt_evaluation_eg[side] -= PSQT_EG[side][ROOK][rook_castle_locations[side][castle_side][END]];

                game->piece_at[start] = piece;
                game->piece_at[rook_castle_locations[side][castle_side][START]] = ROOK;
            }
            break;
        case PROMOTION:
            {

                uint64_t s = 1ULL << start;
                uint64_t e = 1ULL << end;
                
                *piece_board |= s;

                game->pieces[side][promotion_type] ^= e;

                *side_to_move_pieces ^= (s | e);


                key ^= get_piece_random(piece, side, start);
                key ^= get_piece_random(promotion_type, side, end);
                // if a promotion capture, no need to change the end square since we are only replacing
                if (move->promotion_capture){
                   
                    game->pieces[!side][capture_piece] |= e;
                    
                    *other_side_pieces |= e;
                

                    key ^= get_piece_random(capture_piece, (Side)!side, end);


                    game->psqt_evaluation_mg[!side] += PSQT_MG[!side][capture_piece][end];
                    game->psqt_evaluation_eg[!side] += PSQT_EG[!side][capture_piece][end];

                    game->phase += phase_values[capture_piece];
                    game->piece_at[end] = capture_piece;
                    if (capture_piece == PAWN){
                        game->pawn_key ^= pawn_random[side][end];
                    
                    }
                } else {
                    
                }

                game->psqt_evaluation_mg[side] += PSQT_MG[side][piece][start];
                game->psqt_evaluation_eg[side] += PSQT_EG[side][piece][start];
                game->psqt_evaluation_mg[side] -= PSQT_MG[side][promotion_type][end];
                game->psqt_evaluation_eg[side] -= PSQT_EG[side][promotion_type][end];

                game->phase -= phase_values[move->promotion_type];
                game->piece_at[start] = piece;

                game->pawn_key ^= pawn_random[side][start];

            }
            break;
    }


    if (piece == KING){

        int file = start % 8;
        if (file <= 4){
            
            game->king_location_castle_side[side][QUEENSIDE] = true;
            game->king_location_castle_side[side][KINGSIDE] = false;
            game->king_location_castle_side[side][BOTHSIDE] = false;
        } else if (file >= 6){
            game->king_location_castle_side[side][KINGSIDE] = true;
            game->king_location_castle_side[side][QUEENSIDE] = false;
            game->king_location_castle_side[side][BOTHSIDE] = false;
        } else {
            game->king_location_castle_side[side][BOTHSIDE] = true;
            game->king_location_castle_side[side][QUEENSIDE] = false;
            game->king_location_castle_side[side][KINGSIDE] = false;
        }
    }

    if (move->last_en_passant_square != -1){
        
        if (pawn_captures[!side][move->last_en_passant_square] & game->pieces[side][PAWN]) {
        
            key ^= get_en_passant_random(move->last_en_passant_square);
        }
    }
    
    game->en_passant_index = move->last_en_passant_square;

    if (game->castle_flags[side][QUEENSIDE] != move->castle_flags[side][QUEENSIDE]){
        key ^= get_castling_random(side, QUEENSIDE);
    }
    if (game->castle_flags[side][KINGSIDE] != move->castle_flags[side][KINGSIDE]){
        key ^= get_castling_random(side, KINGSIDE);
    }
    if (game->castle_flags[!side][QUEENSIDE] != move->castle_flags[!side][QUEENSIDE]){
        key ^= get_castling_random((Side)!side, QUEENSIDE);
    }
    if (game->castle_flags[!side][KINGSIDE] != move->castle_flags[!side][KINGSIDE]){
        key ^= get_castling_random((Side)!side, KINGSIDE);
    }


    game->castle_flags[side][QUEENSIDE] = move->castle_flags[side][QUEENSIDE];    
    game->castle_flags[side][KINGSIDE] = move->castle_flags[side][KINGSIDE];    
    game->castle_flags[!side][QUEENSIDE] = move->castle_flags[!side][QUEENSIDE];    
    game->castle_flags[!side][KINGSIDE] = move->castle_flags[!side][KINGSIDE];    
    
    key ^= get_turn_random();

    game->board_pieces[BOTH] = game->board_pieces[WHITE] | game->board_pieces[BLACK];
    game->side_to_move = (Side)!game->side_to_move;
    game->key_history[game->history_count] = 0;
    game->history_count -= 1;
    Move last_last = game->last_move;
    game->last_move = game->last_last_move;


    // this move is the move we just played and are undoing.
    // this data is needed for refutation.
    game->last_last_move = last_last;
    game->key = key;
    
}




/* @brief handles polyglot hash key / tt updates, bitboard, and "piece_at[][]" changes. also stores history, and replaces last_move which is used for refutations near the end of the function. */

// bool make_move(Game * game, Move * move);


// returns if move was valid - if false, undo this move
static inline bool make_move(Game * game, Move * move){

    Side side = move->side;
    PieceType piece = move->piece;
    PieceType capture_piece = move->capture_piece;
    int start= move->start_index;
    int end= move->end_index;
    int en_passant_index = game->en_passant_index;
    uint64_t key = game->key;
    CastleSide castle_side = move->castle_side;
    int ep_sq = game->en_passant_index;
    PieceType promotion_type = move->promotion_type;
    uint64_t * piece_board = &game->pieces[side][piece];
    uint64_t * side_to_move_pieces = &game->board_pieces[side];
    uint64_t * other_side_pieces = &game->board_pieces[!side];
    // move->old_key = game->key;
    // double phase = (double)game->phase / MAX_PHASE;
    // double eg_phase = 1.0 - phase;
    uint64_t our_pieces = *side_to_move_pieces;
    uint64_t their_pieces = *other_side_pieces;


    if (en_passant_index != -1){
        if (pawn_captures[!side][en_passant_index] & game->pieces[side][PAWN]) {
        
            key ^= get_en_passant_random(en_passant_index);
        }
        
    }

    
    switch(move->type){
        case MOVE:
            {

                uint64_t move_board = (1ULL << start | 1ULL << end);
                // move piece on its board
                *piece_board ^= move_board;

                // move piece on its color's board
                *side_to_move_pieces ^= move_board;

                key ^= get_piece_random(piece, side, start);
                key ^= get_piece_random(piece, side, end);

                game->piece_at[end] = piece;

                if (piece == PAWN){
                    game->pawn_key ^= pawn_random[side][start];
                    game->pawn_key ^= pawn_random[side][end];
                }
                
                game->psqt_evaluation_mg[side] -= PSQT_MG[side][piece][start];
                game->psqt_evaluation_eg[side] -= PSQT_EG[side][piece][start];
                game->psqt_evaluation_mg[side] += PSQT_MG[side][piece][end];
                game->psqt_evaluation_eg[side] += PSQT_EG[side][piece][end];

                
                // update_incremental_material_move(game, side, piece, start, end, false, 0, 0, false, our_pieces, their_pieces);


                
                break;
            }
        case CAPTURE:
            {

                uint64_t cap = 1ULL << end;
                uint64_t move_board = (1ULL << start | cap);
                // move piece on its board
                *piece_board ^= move_board;

                // toggle captured piece on its board
                game->pieces[!side][capture_piece] ^= cap; 

                // move piece on its color's board
                *side_to_move_pieces ^= move_board;

                // toggle captured piece on its color's board
                *other_side_pieces ^= cap;


                key ^= get_piece_random(piece, side, start);
                key ^= get_piece_random(piece, side, end);
                key ^= get_piece_random(capture_piece, (Side)!side, end);

                // game->material_evaluation_mg[!side] -= piece_values_mg[move->capture_piece];
                // game->material_evaluation_eg[!side] -= piece_values_eg[move->capture_piece];

                game->phase -= phase_values[capture_piece];

                game->piece_at[end] = piece;

                if (piece == PAWN){
                    game->pawn_key ^= pawn_random[side][start];
                    game->pawn_key ^= pawn_random[side][end];
                }
                if (capture_piece == PAWN){
                    // game->pawn_key ^= pawn_random[side][start];
                    game->pawn_key ^= pawn_random[!side][end];
                    
                }
                game->psqt_evaluation_mg[side] -= PSQT_MG[side][piece][start];
                game->psqt_evaluation_eg[side] -= PSQT_EG[side][piece][start];
                game->psqt_evaluation_mg[side] += PSQT_MG[side][piece][end];
                game->psqt_evaluation_eg[side] += PSQT_EG[side][piece][end];

                game->psqt_evaluation_mg[!side] -= PSQT_MG[!side][capture_piece][end];
                game->psqt_evaluation_eg[!side] -= PSQT_EG[!side][capture_piece][end];

                // update_incremental_material_move(game, side, piece, start, end, true, end, capture_piece, false, our_pieces, their_pieces);
            }
            break;
        case EN_PASSANT:
            {
                int capture_square = __builtin_ctzll(pawn_moves[!side][end]);
                
                uint64_t move_board = (1ULL << start | 1ULL << end);
                uint64_t cap = 1ULL << capture_square;
                // move piece on its board
                *piece_board ^= move_board;

                // this utilizes our precomputed board masks for movement. the en passant square for each side will always have one bit forward, which is the exact bit where our pawn that we are capturing is
                // we avoid having to check if black / white and just use this mask
                game->pieces[!side][capture_piece] ^= cap;

                *side_to_move_pieces ^= move_board;

                *other_side_pieces ^= cap;


                key ^= get_piece_random(piece, side, start);
                key ^= get_piece_random(piece, side, end);
                key ^= get_piece_random(capture_piece, (Side)!side, capture_square);


                game->phase -= phase_values[capture_piece];

                game->piece_at[end] = piece;

                if (piece == PAWN){
                    game->pawn_key ^= pawn_random[side][start];
                    game->pawn_key ^= pawn_random[side][end];
                }
                    // game->pawn_key ^= pawn_random[side][start];
                game->pawn_key ^= pawn_random[!side][capture_square];

                game->psqt_evaluation_mg[side] -= PSQT_MG[side][piece][start];
                game->psqt_evaluation_eg[side] -= PSQT_EG[side][piece][start];
                game->psqt_evaluation_mg[side] += PSQT_MG[side][piece][end];
                game->psqt_evaluation_eg[side] += PSQT_EG[side][piece][end];

                game->psqt_evaluation_mg[!side] -= PSQT_MG[!side][capture_piece][capture_square];
                game->psqt_evaluation_eg[!side] -= PSQT_EG[!side][capture_piece][capture_square];

            }
            break;
        case CASTLE:
            {
                // moves king to its final location
                // *piece_board = toggle_bit(*piece_board, move.start_index);
                // *piece_board = set_bit(*piece_board, move.end_index);
                uint64_t move_board = (1ULL << start | 1ULL << end);
                uint64_t cb = 
                    1ULL << rook_castle_locations[side][castle_side][START] | 1ULL << rook_castle_locations[side][castle_side][END];

                *piece_board ^= move_board;

                game->pieces[side][ROOK] ^= cb;


                *side_to_move_pieces ^= cb | move_board;

                key ^= get_piece_random(piece, side, start);
                key ^= get_piece_random(piece, side, end);
                key ^= get_piece_random(ROOK, side, rook_castle_locations[side][castle_side][START]);
                key ^= get_piece_random(ROOK, side, rook_castle_locations[side][castle_side][END]);

                game->psqt_evaluation_mg[side] -= PSQT_MG[side][piece][start];
                game->psqt_evaluation_eg[side] -= PSQT_EG[side][piece][start];
                game->psqt_evaluation_mg[side] += PSQT_MG[side][piece][end];
                game->psqt_evaluation_eg[side] += PSQT_EG[side][piece][end];


                game->psqt_evaluation_mg[side] -= PSQT_MG[side][ROOK][rook_castle_locations[side][castle_side][START]];
                game->psqt_evaluation_eg[side] -= PSQT_EG[side][ROOK][rook_castle_locations[side][castle_side][START]];
                game->psqt_evaluation_mg[side] += PSQT_MG[side][ROOK][rook_castle_locations[side][castle_side][END]];
                game->psqt_evaluation_eg[side] += PSQT_EG[side][ROOK][rook_castle_locations[side][castle_side][END]];

                game->piece_at[end] = piece;
                game->piece_at[rook_castle_locations[side][castle_side][END]] = ROOK;

                // game->pawn_key ^= king_location_random[side][castle_side];


            }
            break;
        case PROMOTION:
            {

                uint64_t s = 1ULL << start;
                uint64_t e = 1ULL << end;
                
                *piece_board ^= s;

                game->pieces[side][promotion_type] |= e;

                *side_to_move_pieces ^= (s | e);


                key ^= get_piece_random(piece, side, start);
                key ^= get_piece_random(promotion_type, side, end);
                // if a promotion capture, no need to change the end square since we are only replacing
                if (move->promotion_capture){
                    
                    game->pieces[!side][capture_piece] ^= e;                    
                    *other_side_pieces ^= e;
                
                    key ^= get_piece_random(capture_piece, (Side)!side, end);
                    game->psqt_evaluation_mg[!side] -= PSQT_MG[!side][capture_piece][end];
                    game->psqt_evaluation_eg[!side] -= PSQT_EG[!side][capture_piece][end];

                    game->phase -= phase_values[capture_piece];
                    if (capture_piece == PAWN){
                        game->pawn_key ^= pawn_random[!side][end];
                    
                    }
                } else {
                    
                }

                game->psqt_evaluation_mg[side] -= PSQT_MG[side][piece][start];
                game->psqt_evaluation_eg[side] -= PSQT_EG[side][piece][start];
                game->psqt_evaluation_mg[side] += PSQT_MG[side][promotion_type][end];
                game->psqt_evaluation_eg[side] += PSQT_EG[side][promotion_type][end];


                game->phase += phase_values[promotion_type];
                game->piece_at[end] = promotion_type;

                game->pawn_key ^= pawn_random[side][start];
            }
            break;
    }


    move->last_en_passant_square = en_passant_index;
    move->castle_flags[side][QUEENSIDE] = game->castle_flags[side][QUEENSIDE];    
    move->castle_flags[side][KINGSIDE] = game->castle_flags[side][KINGSIDE];    
    move->castle_flags[!side][QUEENSIDE] = game->castle_flags[!side][QUEENSIDE];    
    move->castle_flags[!side][KINGSIDE] = game->castle_flags[!side][KINGSIDE];    
    

    
    if (move->double_push){
        game->en_passant_index = -push_direction[side] * 8 + end;
        // if we create an en passant square, we need to check for the special polyglot square as well

        if (game->pieces[!side][PAWN] & pawn_captures[side][game->en_passant_index]){
            key ^= get_en_passant_random(game->en_passant_index);
        }

        
    } else {
        game->en_passant_index = -1;
    }
    


    if (piece == KING){

        int file = end % 8;
        if (file <= 4){
            
            game->king_location_castle_side[side][QUEENSIDE] = true;
            game->king_location_castle_side[side][KINGSIDE] = false;
            game->king_location_castle_side[side][BOTHSIDE] = false;
        } else if (file >= 6){
            game->king_location_castle_side[side][KINGSIDE] = true;
            game->king_location_castle_side[side][QUEENSIDE] = false;
            game->king_location_castle_side[side][BOTHSIDE] = false;
        } else {
            game->king_location_castle_side[side][BOTHSIDE] = true;
            game->king_location_castle_side[side][QUEENSIDE] = false;
            game->king_location_castle_side[side][KINGSIDE] = false;
        }
    }

    
    
    if (move->end_index == rook_starting_locations[!side][QUEENSIDE]){

        // if we had a flag here, we need to zobrist hash it changing
        if (game->castle_flags[!side][QUEENSIDE] == true){
            key ^= get_castling_random((Side)!side, QUEENSIDE);
        }

        
        game->castle_flags[!side][QUEENSIDE] = false;
    }
    if (move->end_index == rook_starting_locations[!side][KINGSIDE]){
        
        if (game->castle_flags[!side][KINGSIDE] == true){
            key ^= get_castling_random((Side)!side, KINGSIDE);
        }
        
        game->castle_flags[!side][KINGSIDE] = false;
    }
    
    if (move->loses_rights){
        switch(move->rights_toggle){
            case QUEENSIDE:
                if (game->castle_flags[side][QUEENSIDE]){
                    key ^= get_castling_random(side, QUEENSIDE);
                }
                game->castle_flags[side][QUEENSIDE] = false;
                break;
            case KINGSIDE:
                if (game->castle_flags[side][KINGSIDE]){
                    key ^= get_castling_random(side, KINGSIDE);
                }
                game->castle_flags[side][KINGSIDE] = false;
                break;
            case BOTHSIDE:
                if (game->castle_flags[side][QUEENSIDE]){
                    key ^= get_castling_random(side, QUEENSIDE);
                }
                if (game->castle_flags[side][KINGSIDE]){
                    key ^= get_castling_random(side, KINGSIDE);
                }
                game->castle_flags[side][QUEENSIDE] = false;
                game->castle_flags[side][KINGSIDE] = false;
                break;
        
            default:
                break;
        }
    }

    // zobrist
    // if (game->side_to_move == WHITE){
    key ^= get_turn_random();
    // }
    game->board_pieces[BOTH] = game->board_pieces[WHITE] | game->board_pieces[BLACK];
    game->side_to_move = (Side)!game->side_to_move;

    game->key_history[game->history_count] = key;
    game->key = key;
    game->last_last_move = game->last_move;
    game->last_move = *move;
    game->history_count += 1;
    
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



// bool piece_is_attacking_square(Game * game, PieceType piece, Side side, int start_square, int end_square, uint64_t blockers);

static inline void swap_move(Move *a, Move *b){
    Move t = *a; *a = *b; *b = t;
}
static inline void selection_extract_best(Move move_list[], int move_count, int start){
    int best = start;
    int best_score = move_list[start].score;
    for (int i = start + 1; i < move_count; ++i){
        if (move_list[i].score > best_score){
            best_score = move_list[i].score;
            best = i;
        }
    }
    if (best != start) swap_move(&move_list[start], &move_list[best]);
}

/* @brief efficient sort for swapping moves. using a "movepicker" would probably be faster though */

static void partial_selection_sort(Move move_list[], int move_count, int K){
    if (move_count <= 1) return;
    int limit = K < move_count ? K : move_count;
    for (int i = 0; i < limit; ++i){
        selection_extract_best(move_list, move_count, i);
    }
}


/* @brief a quiet move is a noncapture, nonpromoting, and nonchecking move. unlike other engines, we also include "good quiets" sometimes in qsearch. see movegen */
static inline bool move_is_quiet(Game * game, Move * move){

  if (move->type == CAPTURE) {

    return false;

  } else if (move->type == PROMOTION){

    return false;
    
  } else if (move->is_checking){
      
    return false;
  } else if (move->type == EN_PASSANT){
    return false;
  } else {
      return true;
  }
  
}

/* @brief checks against our hash key history to avoid drawing when we are up in material. if we are down in eval, then our engine naturally plays to draw on purpose. */

static inline bool three_fold_repetition(Game * game, uint64_t key){
    int count = 0;
    for (int i = 0; i < game->history_count; i++){
        if (key == game->key_history[i]){
            count += 1;
            if (count >= 2){
                return true;
            }
        }
    }
    return false;
}


static inline bool move_is_equal(Move * a, Move * b){
  
  if (a->start_index == b->start_index && a->end_index == b->end_index && a->type == b->type){
    if (a->type == PROMOTION){
      if (a->promotion_type == b->promotion_type){
        return true;
      } else {
        return false;
      }
    } else {
      return true;
    }
  } else {
    return false;
  }
}




/* @brief stores pv into the triangular pv table. you can find this implementation from code monkey king / chess programming wiki */

static inline void store_pv(int ply, Move * move, SearchData * search_data) {
    if (ply >= 63) return;
    search_data->pv_table[ply][0] = *move;

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
    uint64_t bishop_rays = fetch_bishop_moves(game, pinned_to_sq, occ);
    uint64_t rook_rays = fetch_rook_moves(game, pinned_to_sq, occ);
    uint64_t our_pieces = game->board_pieces[side];

    uint64_t rook_blockers = rook_rays & our_pieces;
    uint64_t bishop_blockers = bishop_rays & our_pieces;

    uint64_t rooks_and_queens = game->pieces[!side][QUEEN] | game->pieces[!side][ROOK];
    uint64_t bishops_and_queens = game->pieces[!side][QUEEN] | game->pieces[!side][BISHOP];

    uint64_t pinned = 0;

    while (rook_blockers){
        int pos = pop_lsb(&rook_blockers);
        uint64_t ray = fetch_rook_moves(game, pinned_to_sq, occ ^ (1ULL << pos));
        if (ray & rooks_and_queens) pinned |= (1ULL << pos);
    }
    while (bishop_blockers){
        int pos = pop_lsb(&bishop_blockers);
        uint64_t ray = fetch_bishop_moves(game, pinned_to_sq, occ ^ (1ULL << pos));
        if (ray & bishops_and_queens) pinned |= (1ULL << pos);
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
static inline void set_piece(Game * game, uint64_t * board, PieceType type, int index){

    *board = set_bit(*board, index);    
    game->piece_at[index] = type;

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
