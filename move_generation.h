#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

#include "types.h"
#include "math.h"
#include "game.h"
#include "magic.h"

/* @brief generates an attack mask from scratch for one side's pieces */

// uint64_t generate_attack_mask(Game * game, Side side);

static inline uint64_t generate_attack_mask(Game * game, Side side){
    
    uint64_t attacked_squares = 0;

    uint64_t pawns = game->pieces[side][PAWN];
    while (pawns){
        int index = pop_lsb(&pawns);
        attacked_squares |= pawn_captures[side][index];
    }
    uint64_t knights = game->pieces[side][KNIGHT];
    while (knights){
        int index = pop_lsb(&knights);
        attacked_squares |= knight_moves[index];
    }

    uint64_t bishops = game->pieces[side][BISHOP];
    while (bishops){
        int index = pop_lsb(&bishops);
        attacked_squares |= fetch_bishop_moves(game, index, game->board_pieces[BOTH]);
    }
    uint64_t rooks = game->pieces[side][ROOK];
    while (rooks){
        int index = pop_lsb(&rooks);
        attacked_squares |= fetch_rook_moves(game, index, game->board_pieces[BOTH]);
    }
    uint64_t queens = game->pieces[side][QUEEN];
    while (queens){
        int index = pop_lsb(&queens);
        attacked_squares |= fetch_queen_moves(game, index, game->board_pieces[BOTH]);
    }
    
    int kpos = __builtin_ctzll(game->pieces[side][KING]);
    attacked_squares |= king_moves[kpos];
    
    return attacked_squares;
    
}
/* @brief calculated once a node to determine good quiets. may be used later to further improve move ordering in heavy tactical sequences */

static inline void generate_movement_masks(Game * game, Side side, MovementMasks * masks){
    
    
    masks->our_attack_mask = generate_attack_mask(game, side);
    masks->enemy_attack_mask = generate_attack_mask(game, (Side)!side);

    masks->our_attacked_pieces = game->board_pieces[!side] & masks->our_attack_mask;
    masks->enemy_attacked_pieces = game->board_pieces[side] & masks->enemy_attack_mask;

    // dont include pawns
    masks->our_hanging_pieces = (game->board_pieces[side] & ~(masks->our_attack_mask & game->board_pieces[side]) & ~game->pieces[side][PAWN]);
    masks->enemy_hanging_pieces = (game->board_pieces[!side] & ~(masks->enemy_attack_mask & game->board_pieces[!side]) & ~game->pieces[!side][PAWN]);
    
}
/* @brief creates an attacker mask just for pieces that are looking at a certain square. takes in a special piece array due to see usage, we need a working set of piece boards while we loop through and pop them. */

static inline uint64_t attacker_mask_for_square(Game * game, Side side, uint64_t pieces[PIECE_TYPES], int index, uint64_t occupancy){

    uint64_t attackers = 0;
    uint64_t bishop_rays = fetch_bishop_moves(game, index, occupancy);
    
    attackers |= bishop_rays & pieces[BISHOP];
    uint64_t rook_rays = fetch_rook_moves(game, index, occupancy);
    
    attackers |= rook_rays & pieces[ROOK];
    
    attackers |= (bishop_rays | rook_rays) & pieces[QUEEN];

    attackers |= (knight_moves[index] & pieces[KNIGHT]);
    attackers |= pawn_captures[!side][index] & pieces[PAWN];
    attackers |= king_moves[index] & pieces[KING];
    
    return attackers;

}

static inline int lva_from_attacker_mask(uint64_t pieces[PIECE_TYPES], uint64_t mask, PieceType * p){
    if (!mask || !p) return -1;
    
    for (int i = 0; i < PIECE_TYPES; i++){
        uint64_t overlap = mask & pieces[i];
        if (overlap){
            *p = (PieceType)i;
            return __builtin_ctzll(overlap);
        }
    }
    return -1;
}

// static inline int lva_from_attacker_mask(Game * game, uint64_t mask, PieceType * p){
//     if (!mask || !p) return -1;
//     *p = (PieceType)KING;
//     int bp = 0;
//     while (mask){
//         int pos = pop_lsb(&mask);
//         int pt = game->piece_at[pos];
//         if (*p > pt){
//             *p = (PieceType)pt;
//             bp = pos;
//         }
//         if (pt == PAWN){
//             return pos;
//         }
//     }
//     return bp;
// }

static inline int mva_from_attacker_mask(Game * game, uint64_t pieces[PIECE_TYPES], uint64_t mask, Side side, PieceType * p){
    for (int i = PIECE_TYPES - 1; i >= 0; i++){
        uint64_t overlap = mask & pieces[i];
        if (overlap){
            *p = (PieceType)i;
            return bit_scan_forward(&overlap);
        }
    }
    return -1;
}

static inline void push_move(Move move_list[200], Move move, int * move_index){

    move_list[(*move_index)++] = move;
    
}
static inline void find_and_push_captures(Game * game, Move move_list[200], int * move_count, Side side, PieceType piece_type, uint64_t captures, int start_index){
    
    while (captures){
        
        int end_index = pop_lsb(&captures);
        push_move(move_list, (Move){
            .start_index = start_index,
            .end_index = end_index,
            .type = CAPTURE,
            .side = side,
            .piece = piece_type,
            .capture_piece = game->piece_at[end_index],
            .double_push = false,
            .loses_rights = false,
        }, move_count);
    }
    
}

/* @brief an experimental idea to let in certain good quiet moves into qsearch and push them higher in main search move ordering.
it currently has moves that attack enemy hanging pieces or defends our attacking pieces, or attacks a piece we already attack.
@param next_attack_mask the attack mask from the end position of the move */

static inline bool evaluate_good_quiet(MovementMasks * masks, uint64_t next_attack_mask){

    // if (next_attack_mask & masks->our_attacked_pieces){
    //     return true;
    // }

    // if (next_attack_mask & masks->our_hanging_pieces){
    //     return true;
    // }
    // if (next_attack_mask & masks->enemy_hanging_pieces){
    //     return true;
    // }
    // if (see(game, move) > 0){
    //     return true;
    // }
    return false;
}

static inline void generate_pawn_moves(Game * game, Move move_list[200], int * move_count, Side side, uint64_t potential_captures, uint64_t own_pieces, MovementMasks * masks, bool only_non_quiet){

    // uint64_t * pawn_moves = pawn_moves[side];
    // uint64_t * pawn_captures = game->pawn_captures[side];

    uint64_t pawns = game->pieces[side][PAWN];
    uint64_t blockers = game->board_pieces[BOTH];
    uint64_t enemy_king = game->pieces[!side][KING];

    
    
    while (pawns){

        int index = pop_lsb(&pawns);
        uint64_t moves = pawn_moves[side][index];
        bool cannot_double_push = false;
        uint64_t blocker = moves & game->board_pieces[BOTH];
        if (blocker) cannot_double_push = true;
        moves ^= blocker;
            
        uint64_t en_passant = 0;
        
        if (game->en_passant_index != -1) en_passant = 1ULL << game->en_passant_index;
        uint64_t captures = pawn_captures[side][index] & potential_captures;
        en_passant &= pawn_captures[side][index];
        uint64_t promotions = (moves | captures) & promotion_ranks[side];
        moves &= ~promotions;
        // this is a working copy to remove our captures from afterwards, since we need our captures during this loop to determine promotion captures
        uint64_t promotion_copy = promotions;
        
        while (promotions){
            int move_index = pop_lsb(&promotions);
            bool promotion_capture = 1ULL << move_index & captures;
            bool checking = (fetch_queen_moves(game, move_index, blockers) & enemy_king);
            // if (!checking && only_non_quiet) continue;

            PieceType promotion_capture_piece = (PieceType)0;

            bool found_promo_piece = false;
            if (promotion_capture){
                promotion_capture_piece = game->piece_at[move_index];
            }

            push_move(move_list, (Move){
                .start_index = index,
                .end_index = move_index,
                .type = PROMOTION,
                .side = side,
                .piece = PAWN,
                .capture_piece = promotion_capture_piece,
                .promotion_type = KNIGHT,
                .promotion_capture = promotion_capture,
                .double_push = false,
                .loses_rights = false,
                .is_checking = checking,
                .good_quiet = false
            }, move_count);
            push_move(move_list, (Move){
                .start_index = index,
                .end_index = move_index,
                .type = PROMOTION,
                .side = side,
                .piece = PAWN,
                .capture_piece = promotion_capture_piece,
                .promotion_type = BISHOP,
                .promotion_capture = promotion_capture,
                .double_push = false,
                .loses_rights = false,
                .is_checking = checking,
                .good_quiet = false
            }, move_count);
            push_move(move_list, (Move){
                .start_index = index,
                .end_index = move_index,
                .type = PROMOTION,
                .side = side,
                .piece = PAWN,
                .capture_piece = promotion_capture_piece,
                .promotion_type = ROOK,
                .promotion_capture = promotion_capture,
                .double_push = false,
                .loses_rights = false,
                .is_checking = checking,
                .good_quiet = false
            }, move_count);
            push_move(move_list, (Move){
                .start_index = index,
                .end_index = move_index,
                .type = PROMOTION,
                .side = side,
                .piece = PAWN,
                .capture_piece = promotion_capture_piece,
                .promotion_type = QUEEN,
                .promotion_capture = promotion_capture,
                .double_push = false,
                .loses_rights = false,
                .is_checking = checking,
                .good_quiet = false
            }, move_count);
        }
        captures &= ~promotion_copy;
        // find_and_push_captures(game, move_list, move_count, side, PAWN, captures, index);
        while (captures){
        
            int end_index = pop_lsb(&captures);
            bool checking = pawn_captures[side][end_index] & enemy_king;
            push_move(move_list, (Move){
                .start_index = index,
                .end_index = end_index,
                .type = CAPTURE,
                .side = side,
                .piece = PAWN,
                .capture_piece = game->piece_at[end_index],
                .double_push = false,
                .loses_rights = false,
                .is_checking = checking,
                .good_quiet = false
            }, move_count);
        }

        if (en_passant && !only_non_quiet){
            bool checking = (pawn_captures[side][game->en_passant_index] & enemy_king);
            if (!checking && only_non_quiet) continue;
            push_move(move_list, (Move){
                .start_index = index,
                .end_index = game->en_passant_index,
                .type = EN_PASSANT,
                .side = side,
                .piece = PAWN,
                .double_push = false,
                .loses_rights = false,
                .is_checking = checking,
                .good_quiet = false
            }, move_count);
        }
        // if (only_non_quiet) continue;
        while (moves){
            int move_index = pop_lsb(&moves);
            uint64_t next_attack_mask = pawn_captures[side][move_index];
            bool checking = (next_attack_mask & enemy_king);
            bool good_quiet = evaluate_good_quiet(masks, next_attack_mask);
            
            if (!checking && !good_quiet && only_non_quiet) continue;
        
            if (only_non_quiet) continue;
            bool double_push = false;
        
            if (move_index == double_pushed_pawn_squares[side][index]){
                double_push = true;
                if (cannot_double_push) continue;
            }
        
            push_move(move_list, (Move){
                .start_index = index,
                .end_index = move_index,
                .type = MOVE,
                .side = side,
                .piece = PAWN,
                .double_push = double_push,
                .loses_rights = false,
                .is_checking = checking,
                .good_quiet = good_quiet
            }, move_count);
        }
    }
    
}

static inline void generate_bishop_moves(Game * game, Move move_list[200], int * move_count, Side side, uint64_t potential_captures, uint64_t own_pieces, uint64_t blockers, MovementMasks * masks, bool only_non_quiet){

    uint64_t bishops = game->pieces[side][BISHOP];
    // int bishop_count = bit_count(bishops);
    uint64_t enemy_king = game->pieces[!side][KING];
    while (bishops){
        int index = pop_lsb(&bishops);
        uint64_t moves = fetch_bishop_moves(game, index, blockers);
        // print_board(moves, BLACK_BISHOP);
        uint64_t captures = moves & potential_captures;

        // find_and_push_captures(game, move_list, move_count, side, BISHOP, captures, index);
        while (captures){
        
            int end_index = pop_lsb(&captures);
            bool checking = fetch_bishop_moves(game, end_index, game->board_pieces[BOTH]) & enemy_king;
            push_move(move_list, (Move){
                .start_index = index,
                .end_index = end_index,
                .type = CAPTURE,
                .side = side,
                .piece = BISHOP,
                .capture_piece = game->piece_at[end_index],
                .double_push = false,
                .loses_rights = false,
                .is_checking = checking
            }, move_count);
        }

        moves &= ~game->board_pieces[BOTH];
        while (moves){
            int move_index = pop_lsb(&moves);
            // if (only_non_quiet && !(fetch_bishop_moves(game, move_index, blockers) & enemy_king)) continue;
            uint64_t next_attack_mask = fetch_bishop_moves(game, move_index, game->board_pieces[BOTH]);
            bool checking = next_attack_mask & enemy_king;
            bool good_quiet = evaluate_good_quiet(masks, next_attack_mask);
            if (only_non_quiet && !checking && !good_quiet) continue;
            push_move(move_list, (Move){
                .start_index = index,
                .end_index = move_index,
                .type = MOVE,
                .side = side,
                .piece = BISHOP,
                .double_push = false,
                .loses_rights = false,
                .is_checking = checking
            }, move_count);
        }
            
    }
}
static inline void generate_rook_moves(Game * game, Move move_list[200], int * move_count, Side side, uint64_t potential_captures, uint64_t own_pieces, uint64_t blockers, MovementMasks * masks, bool only_non_quiet){

    CastleSide rights_toggle = NONESIDE;
    bool loses_rights = false;
    uint64_t rooks = game->pieces[side][ROOK];
    uint64_t enemy_king = game->pieces[!side][KING];
    while (rooks){
        int index = pop_lsb(&rooks);
        uint64_t moves = fetch_rook_moves(game, index, blockers);
        uint64_t captures = moves & potential_captures;
        if (!only_non_quiet){
            moves &= ~game->board_pieces[BOTH];


            while (moves){
                int move_index = pop_lsb(&moves);
                // CastleSide rights_toggle = QUEENSIDE;
                // bool loses_rights = false;
                // if (only_non_quiet && !(fetch_rook_moves(game, move_index, blockers) & enemy_king)) continue;
                uint64_t next_attack_mask = fetch_rook_moves(game, move_index, game->board_pieces[BOTH]);
                bool checking = next_attack_mask & enemy_king;
                bool good_quiet = evaluate_good_quiet(masks, next_attack_mask);
                if (only_non_quiet && !checking && !good_quiet) continue;

                if (game->castle_flags[side][KINGSIDE] && (index == 63 || index == 7)){
                    rights_toggle = KINGSIDE;
                    loses_rights = true;
                } else if (game->castle_flags[side][QUEENSIDE] && (index == 56 || index == 0)){
                    rights_toggle = QUEENSIDE;
                    loses_rights = true;
                }
                push_move(move_list, (Move){
                    .start_index = index,
                    .end_index = move_index,
                    .type = MOVE,
                    .side = side,
                    .piece = ROOK,
                    .double_push = false,
                    .loses_rights = loses_rights,
                    .rights_toggle = rights_toggle,
                    .is_checking = checking
                }, move_count);
            }
            
        }


        while (captures){
            int end_index = pop_lsb(&captures);
            if (game->castle_flags[side][KINGSIDE] && (index == 63 || index == 7)){
                rights_toggle = KINGSIDE;
                loses_rights = true;

            } else if (game->castle_flags[side][QUEENSIDE] && (index == 56 || index == 0)){
                rights_toggle = QUEENSIDE;
                loses_rights = true;
            }
            bool checking = fetch_rook_moves(game, end_index, game->board_pieces[BOTH]) & enemy_king;
            push_move(move_list, (Move){
                .start_index = index,
                .end_index = end_index,
                .type = CAPTURE,
                .side = side,
                .piece = ROOK,
                .capture_piece = game->piece_at[end_index],
                .double_push = false,
                .loses_rights = loses_rights,
                .rights_toggle = rights_toggle,
                .is_checking = checking
            }, move_count);
        }

        
    }
}

static inline void generate_queen_moves(Game * game, Move move_list[200], int * move_count, Side side, uint64_t potential_captures, uint64_t own_pieces, uint64_t blockers, MovementMasks * masks, bool only_non_quiet){

    uint64_t queens = game->pieces[side][QUEEN];
    uint64_t enemy_king = game->pieces[!side][KING];
    while (queens){
        int index = pop_lsb(&queens);
        uint64_t moves = fetch_queen_moves(game, index, blockers);
        uint64_t captures = moves & potential_captures;
        // find_and_push_captures(game, move_list, move_count, side, QUEEN, captures, index);
        while (captures){
        
            int end_index = pop_lsb(&captures);
            bool checking = fetch_queen_moves(game, end_index, game->board_pieces[BOTH]) & enemy_king;
            push_move(move_list, (Move){
                .start_index = index,
                .end_index = end_index,
                .type = CAPTURE,
                .side = side,
                .piece = QUEEN,
                .capture_piece = game->piece_at[end_index],
                .double_push = false,
                .loses_rights = false,
                .is_checking = checking,
                .good_quiet = false
            }, move_count);
        }
        // if (only_non_quiet) continue;
        moves &= ~game->board_pieces[BOTH];


        while (moves){
            int move_index = pop_lsb(&moves);
            // if (only_nonquiet && !(fetch_queen_moves(game, move_index, blockers) & enemy_king)) continue;
            uint64_t next_attack_mask = fetch_queen_moves(game, move_index, game->board_pieces[BOTH]);
            bool checking = next_attack_mask & enemy_king;
            bool good_quiet = evaluate_good_quiet(masks, next_attack_mask);
            if (only_non_quiet && !checking && !good_quiet) continue;
            push_move(move_list, (Move){
                .start_index = index,
                .end_index = move_index,
                .type = MOVE,
                .side = side,
                .piece = QUEEN,
                .double_push = false,
                .loses_rights = false,
                .is_checking = checking,
                .good_quiet = good_quiet
            }, move_count);
        }
            
    }
}

static inline void generate_king_moves(Game * game, Move move_list[200], int * move_count, Side side, uint64_t potential_captures, uint64_t own_pieces, MovementMasks * masks, bool only_non_quiet){


    uint64_t kings = game->pieces[side][KING];
    bool loses_rights = false;
    CastleSide rights_toggle = NONESIDE;
    if (game->castle_flags[side][KINGSIDE] && game->castle_flags[side][QUEENSIDE]){
        loses_rights = true;
        rights_toggle = BOTHSIDE;
    } else if (game->castle_flags[side][KINGSIDE]){
        loses_rights = true;
        rights_toggle = KINGSIDE;
    } else if (game->castle_flags[side][QUEENSIDE]){
        loses_rights = true;
        rights_toggle = QUEENSIDE;
    }

        int index = __builtin_ctzll(kings);

        uint64_t moves = king_moves[index];
        moves &= ~own_pieces;
        uint64_t captures = moves & potential_captures;
        moves &= ~captures;

        while (moves){
    
            int move_index = pop_lsb(&moves);
        
            bool good_quiet = evaluate_good_quiet(masks,  king_moves[move_index]);
            if (only_non_quiet && !good_quiet) continue;
            push_move(move_list, (Move){
                .start_index = index,
                .end_index = move_index,
                .type = MOVE,
                .side = side,
                .piece = KING,
                .double_push = false,
                .loses_rights = loses_rights,
                .rights_toggle = rights_toggle,
                .is_checking = false, 
                .good_quiet = good_quiet
            }, move_count);
            
        }
        while (captures){
            int end_index = pop_lsb(&captures);
            
            push_move(move_list, (Move){
                .start_index = index,
                .end_index = end_index,
                .type = CAPTURE,
                .side = side,
                .piece = KING,
                .capture_piece = game->piece_at[end_index],
                .double_push = false,
                .loses_rights = loses_rights,
                .rights_toggle = rights_toggle,
                .is_checking = false, 
                .good_quiet = false
            }, move_count);
        }
    
}

static inline void generate_knight_moves(Game * game, Move move_list[200], int * move_count, Side side, uint64_t potential_captures, uint64_t own_pieces, MovementMasks * masks, bool only_non_quiet){
    
    uint64_t knights = game->pieces[side][KNIGHT];
    uint64_t enemy_king = game->pieces[!side][KING];
    while (knights){

        int index = pop_lsb(&knights);
        
        uint64_t moves = knight_moves[index];
        moves &= ~own_pieces;
        uint64_t captures = moves & potential_captures;

        moves &= ~captures;
        // find_and_push_captures(game, move_list, move_count, side, KNIGHT, captures, index);
        while (captures){
        
            int end_index = pop_lsb(&captures);
            bool checking = knight_moves[end_index]& enemy_king;
            push_move(move_list, (Move){
                .start_index = index,
                .end_index = end_index,
                .type = CAPTURE,
                .side = side,
                .piece = KNIGHT,
                .capture_piece = game->piece_at[end_index],
                .double_push = false,
                .loses_rights = false,
                .is_checking = checking,
                .good_quiet = false
            }, move_count);
        }
        // if (only_non_quiet) continue;
        while (moves){
    
            int move_index = pop_lsb(&moves);
            // if (only_non_quiet && !(game->knight_moves[move_index] & enemy_king)) continue;
            bool checking = knight_moves[move_index] & enemy_king;
            bool good_quiet = evaluate_good_quiet(masks, knight_moves[move_index]);
            if (only_non_quiet && !checking && !good_quiet) continue;
            push_move(move_list, (Move){
                .start_index = index,
                .end_index = move_index,
                .type = MOVE,
                .side = side,
                .piece = KNIGHT,
                .double_push = false,
                .loses_rights = false,
                .is_checking = checking,
                .good_quiet = good_quiet
            }, move_count);
        }
            

    }
    
}

/* @brief found on chess programming wiki */

static inline int manhattanDistance(int sq1, int sq2) {
   int file1, file2, rank1, rank2;
   int rankDistance, fileDistance;
   file1 = sq1  & 7;
   file2 = sq2  & 7;
   rank1 = sq1 >> 3;
   rank2 = sq2 >> 3;
   rankDistance = abs (rank2 - rank1);
   fileDistance = abs (file2 - file1);
   return rankDistance + fileDistance;
}



// these functions just generate const bitboard masks for moves.

void generate_pawn_boards(Game * game);
void generate_knight_boards(Game * game);
void generate_king_boards(Game * game);




void init_piece_boards(Game * game);
void update_blocker_masks(Game * game);
// void generate_non_quiet_moves(Game * game, Side side, Move move_list[200], int * move_count);
// void generate_moves(Game * game, Side side, Move move_list[200], int * move_count);

static inline void generate_non_quiet_moves(Game * game, Side side, Move move_list[200], int * move_count){
    uint64_t blockers = game->board_pieces[BOTH];

    uint64_t potential_captures = 0;
    uint64_t own_pieces = 0;
    uint64_t attack_mask = 0;
    Side other_side = (Side)!side;
    
    potential_captures = game->board_pieces[other_side];
    own_pieces = game->board_pieces[side];
    MovementMasks masks;
    // generate_movement_masks(game, side, &masks);
    
    generate_pawn_moves(game, move_list, move_count, side, potential_captures, own_pieces, &masks, true);
    generate_knight_moves(game, move_list, move_count, side, potential_captures, own_pieces, &masks, true);
    generate_bishop_moves(game, move_list, move_count, side, potential_captures, own_pieces, blockers, &masks, true);
    generate_rook_moves(game, move_list, move_count, side, potential_captures, own_pieces, blockers, &masks, true);
    generate_queen_moves(game, move_list, move_count, side, potential_captures, own_pieces, blockers, &masks, true);
    generate_king_moves(game, move_list, move_count, side, potential_captures, own_pieces, &masks, true);
}

static inline void generate_moves(Game * game, Side side, Move move_list[200], int * move_count){

    uint64_t blockers = game->board_pieces[BOTH];

    uint64_t potential_captures = 0;
    uint64_t own_pieces = 0;
    uint64_t our_attack_mask = 0;
    uint64_t enemy_attack_mask = 0;
    Side other_side = (Side)!side;


    potential_captures = game->board_pieces[other_side];
    own_pieces = game->board_pieces[side];

    // castling and attack mask generation
    MovementMasks masks;
    
    // generate_movement_masks(game, side, &masks);

    // attack_mask = generate_attack_mask(game, other_side);

    if (game->castle_flags[side][QUEENSIDE]){
        
        if (!(game->board_pieces[BOTH] & castle_occupation_masks[side][QUEENSIDE])){

            if (
            !is_square_attacked(game, (Side)!side, castle_attack_squares[side][QUEENSIDE][0])
            &&
            !is_square_attacked(game, (Side)!side, castle_attack_squares[side][QUEENSIDE][1])
            &&
            !is_square_attacked(game, (Side)!side, castle_attack_squares[side][QUEENSIDE][2])){
        
                push_move(move_list, (Move){
                    .start_index = king_castle_locations[side][QUEENSIDE][START],
                    .end_index = king_castle_locations[side][QUEENSIDE][END],
                    .type = CASTLE,
                    .side = side,
                    .piece = KING,
                    .castle_side = QUEENSIDE,
                    .double_push = false,
                    .loses_rights = true,
                    .rights_toggle = BOTHSIDE,
                    .is_checking = false,
                    .good_quiet = false
                }, move_count);
            }
        }
    }

    if (game->castle_flags[side][KINGSIDE]){
        
        if (!(game->board_pieces[BOTH] & castle_occupation_masks[side][KINGSIDE])){

            if (
            !is_square_attacked(game, (Side)!side, castle_attack_squares[side][KINGSIDE][0])
            &&
            !is_square_attacked(game, (Side)!side, castle_attack_squares[side][KINGSIDE][1])
            &&
            !is_square_attacked(game, (Side)!side, castle_attack_squares[side][KINGSIDE][2])){
        
                push_move(move_list, (Move){
                    .start_index = king_castle_locations[side][KINGSIDE][START],
                    .end_index = king_castle_locations[side][KINGSIDE][END],
                    .type = CASTLE,
                    .side = side,
                    .piece = KING,
                    .castle_side = KINGSIDE,
                    .loses_rights = true,
                    .rights_toggle = BOTHSIDE,
                    .is_checking = false,
                    .good_quiet = false
                }, move_count);
            }
        }
    } 
    
    
    generate_pawn_moves(game, move_list, move_count, side, potential_captures, own_pieces, &masks, false);
    generate_knight_moves(game, move_list, move_count, side, potential_captures, own_pieces, &masks, false);
    generate_bishop_moves(game, move_list, move_count, side, potential_captures, own_pieces, blockers, &masks, false);
    generate_rook_moves(game, move_list, move_count, side, potential_captures, own_pieces, blockers, &masks, false);
    generate_queen_moves(game, move_list, move_count, side, potential_captures, own_pieces, blockers, &masks, false);
    generate_king_moves(game, move_list, move_count, side, potential_captures, own_pieces, &masks, false);

    

}

static inline void update_incremental_material_move(Game * game, Side side, PieceType p, int start, int end, bool capture, int capture_square, PieceType cp, bool undo, uint64_t our_pieces_before, uint64_t their_pieces_before){

    uint64_t start_mask = 0;
    uint64_t mas = 0;
    uint64_t end_mask;
    uint64_t mae = 0;
    switch(p){
        case PAWN:

            start_mask = pawn_captures[side][start];
            end_mask = pawn_captures[side][end];
            mas = start_mask & game->board_pieces[!side];
            mae = end_mask & game->board_pieces[!side];

            break;
        case KNIGHT:
            
            start_mask = knight_moves[start];
            end_mask = knight_moves[end];
            mas = start_mask & game->board_pieces[!side];
            mae = end_mask & game->board_pieces[!side];

            break;
        case BISHOP:

            start_mask = fetch_bishop_moves(game, start, our_pieces_before|their_pieces_before);
            end_mask = fetch_bishop_moves(game, end, game->board_pieces[BOTH] );
            mas = start_mask & game->board_pieces[!side];
            mae = end_mask & game->board_pieces[!side];

            
            break;
        case ROOK:
            start_mask = fetch_rook_moves(game, start, our_pieces_before|their_pieces_before);
            end_mask = fetch_rook_moves(game, end, game->board_pieces[BOTH] );
            mas = start_mask & game->board_pieces[!side];
            mae = end_mask & game->board_pieces[!side];
            break;
        case QUEEN:
            start_mask = fetch_queen_moves(game, start, our_pieces_before|their_pieces_before);
            end_mask = fetch_queen_moves(game, end, game->board_pieces[BOTH] );
            mas = start_mask & game->board_pieces[!side];
            mae = end_mask & game->board_pieces[!side];
            break;
        case KING:
            start_mask = king_moves[start];
            end_mask = king_moves[end];
            mas = start_mask & game->board_pieces[!side];
            mae = end_mask & game->board_pieces[!side];
            break;
    }



    
    int sc = bit_count(start_mask & ~our_pieces_before );
    int ec = 0;
    ec =bit_count(end_mask & ~(game->board_pieces[side]));
    if (undo){
        game->mobility_score_mg[side] += sc * MOBILITY_BONUS_MG[p];
        game->mobility_score_eg[side] += sc * MOBILITY_BONUS_EG[p];
        game->mobility_score_mg[side] -= ec * MOBILITY_BONUS_MG[p];
        game->mobility_score_eg[side] -= ec * MOBILITY_BONUS_EG[p];

        if (mas){
            PieceType a;
            mva_from_attacker_mask(game, game->pieces[!side], mas, side, &a);
            game->valuable_attacker_score[side] += ATTACKING_HIGHER_VALUE_BONUS[p][a];
        }
        if (mae){
            PieceType a;
            mva_from_attacker_mask(game, game->pieces[!side], mae, side, &a);
            game->valuable_attacker_score[side] -= ATTACKING_HIGHER_VALUE_BONUS[p][a];
        }
        
    } else {
        game->mobility_score_mg[side] -= sc * MOBILITY_BONUS_MG[p];
        game->mobility_score_eg[side] -= sc * MOBILITY_BONUS_EG[p];
        game->mobility_score_mg[side] += ec * MOBILITY_BONUS_MG[p];
        game->mobility_score_eg[side] += ec * MOBILITY_BONUS_EG[p];
    
        if (mas){
            PieceType a;
            mva_from_attacker_mask(game, game->pieces[!side], mas, side, &a);
            game->valuable_attacker_score[side] -= ATTACKING_HIGHER_VALUE_BONUS[p][a];
        }
        if (mae){
            PieceType a;
            mva_from_attacker_mask(game, game->pieces[!side], mae, side, &a);
            game->valuable_attacker_score[side] += ATTACKING_HIGHER_VALUE_BONUS[p][a];
        }
        
    }



    if (!capture) return;
    
    uint64_t cmask = 0;
    uint64_t cma = 0;
    switch (cp){
        case PAWN:

            cmask = pawn_captures[!side][capture_square];
            cma = cmask & game->board_pieces[side];

            break;
        case KNIGHT:
            
            cmask = knight_moves[capture_square];
            cma = cmask & game->board_pieces[side];

            break;
        case BISHOP:

            cmask = fetch_bishop_moves(game, capture_square, our_pieces_before |their_pieces_before);
            cma = cmask & game->board_pieces[side];
            break;
        case ROOK:
            cmask = fetch_rook_moves(game, capture_square, our_pieces_before |their_pieces_before);
            cma = cmask & game->board_pieces[side];
            break;
        case QUEEN:
            cmask = fetch_queen_moves(game, capture_square, our_pieces_before |their_pieces_before);
            cma = cmask & game->board_pieces[side];
            break;
        case KING:
            cmask = king_moves[capture_square];
            cma = cmask & game->board_pieces[side];
            break;
        
    }

    int cc = bit_count(cmask & ~their_pieces_before);
    if (undo){
        game->mobility_score_mg[!side] += cc * MOBILITY_BONUS_MG[cp];
        game->mobility_score_eg[!side] += cc * MOBILITY_BONUS_EG[cp];

        if (cma){
            PieceType a;
            mva_from_attacker_mask(game, game->pieces[side], cma, side, &a);
            game->valuable_attacker_score[!side] += ATTACKING_HIGHER_VALUE_BONUS[cp][a];
        }
        
    } else {
        game->mobility_score_mg[!side] -= cc * MOBILITY_BONUS_MG[cp];
        game->mobility_score_eg[!side] -= cc * MOBILITY_BONUS_EG[cp];
    
        if (cma){
            PieceType a;
            mva_from_attacker_mask(game, game->pieces[side], cma, side, &a);
            game->valuable_attacker_score[!side] -= ATTACKING_HIGHER_VALUE_BONUS[cp][a];
        }
        
    }
}


static inline void update_incremental_promotion(Game * game, Side side, PieceType pp, int sq, bool undo, uint64_t our_pieces, uint64_t their_pieces){

    uint64_t cmask = 0;
    uint64_t cma = 0;
    switch (pp){
        case PAWN:

            cmask = pawn_captures[side][sq];
            cma = cmask & game->board_pieces[!side];

            break;
        case KNIGHT:
            
            cmask = knight_moves[sq];
            cma = cmask & game->board_pieces[!side];

            break;
        case BISHOP:

            cmask = fetch_bishop_moves(game, sq, game->board_pieces[BOTH]);
            cma = cmask & game->board_pieces[!side];
            break;
        case ROOK:
            cmask = fetch_rook_moves(game, sq, game->board_pieces[BOTH]);
            cma = cmask & game->board_pieces[!side];
            break;
        case QUEEN:
            cmask = fetch_queen_moves(game, sq, game->board_pieces[BOTH]);
            cma = cmask & game->board_pieces[!side];
            break;
        case KING:
            cmask = king_moves[sq];
            cma = cmask & game->board_pieces[!side];
            break;
        
    }
    int cc = bit_count(cmask & ~game->board_pieces[side]);
    if (undo){
        game->mobility_score_mg[side] -= cc * MOBILITY_BONUS_MG[pp];
        game->mobility_score_eg[side] -= cc * MOBILITY_BONUS_EG[pp];
        
        if (cma){
            PieceType a;
            mva_from_attacker_mask(game, game->pieces[!side], cma, side, &a);
            game->valuable_attacker_score[side] -= ATTACKING_HIGHER_VALUE_BONUS[pp][a];
        }
    } else {
        game->mobility_score_mg[side] += cc * MOBILITY_BONUS_MG[pp];
        game->mobility_score_eg[side] += cc * MOBILITY_BONUS_EG[pp];
        if (cma){
            PieceType a;
            mva_from_attacker_mask(game, game->pieces[!side], cma, side, &a);
            game->valuable_attacker_score[side] += ATTACKING_HIGHER_VALUE_BONUS[pp][a];
        }
        
    }
}

#endif
