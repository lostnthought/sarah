#include "game.h"
#include "math.h"
#include "move_generation.h"
#include "types.h"
#include "utils.h"
#include "magic.h"
#include "zobrist.h"
#include <string.h>
#include "search.h"
#include "eval.h"


// uint64_t init_material_eval(Game * game, Side side){
    
//     uint64_t attack_mask = 0;
//     uint64_t our_pieces = game->board_pieces[side];
//     uint64_t other_pieces = game->board_pieces[!side];

//     uint64_t pawns = game->pieces[side][PAWN];
//     while (pawns){

//         int pos = pop_lsb(&pawns);
//         uint64_t moves = pawn_moves[side][pos] & ~our_pieces;

//         uint64_t atk = moves & other_pieces;
//         if (atk){
//             PieceType p;
//             int pos = mva_from_attacker_mask(game, game->pieces[!side], atk, side, &p);

//             game->valuable_attacker_score[side] += ATTACKING_HIGHER_VALUE_BONUS[PAWN][p];
//         }

//         attack_mask |= pawn_captures[side][pos];
//     }
//     uint64_t knights = game->pieces[side][KNIGHT];
//     while (knights){

//         int pos = pop_lsb(&knights);
//         uint64_t moves = knight_moves[pos] & ~our_pieces;
//         int count = __builtin_popcount(moves);
//         game->mobility_score_mg[side] += count * MOBILITY_BONUS_MG[KNIGHT];
//         game->mobility_score_eg[side] += count * MOBILITY_BONUS_EG[KNIGHT];

//         uint64_t atk = moves & other_pieces;
//         if (atk){
//             PieceType p;
//             int pos = mva_from_attacker_mask(game, game->pieces[!side], atk, side, &p);

//             game->valuable_attacker_score[side] += ATTACKING_HIGHER_VALUE_BONUS[KNIGHT][p];
//         }

//         attack_mask |= knight_moves[pos];
//     }
//     uint64_t bishops = game->pieces[side][BISHOP];
//     while (bishops){

//         int pos = pop_lsb(&bishops);
//         uint64_t raw_moves = fetch_bishop_moves(game, pos, game->board_pieces[BOTH]);
//         uint64_t moves = raw_moves & ~our_pieces;
//         int count = __builtin_popcount(moves);

//         game->mobility_score_mg[side] += count * MOBILITY_BONUS_MG[BISHOP];
//         game->mobility_score_eg[side] += count * MOBILITY_BONUS_EG[BISHOP];

//         uint64_t atk = moves & other_pieces;
//         if (atk){
//             PieceType p;
//             int pos = mva_from_attacker_mask(game, game->pieces[!side], atk, side, &p);

//             game->valuable_attacker_score[side] += ATTACKING_HIGHER_VALUE_BONUS[BISHOP][p];
//         }

//         attack_mask |= raw_moves;
//     }
//     uint64_t rooks = game->pieces[side][ROOK];
//     while (rooks){

//         int pos = pop_lsb(&rooks);
//         uint64_t raw_moves = fetch_rook_moves(game, pos, game->board_pieces[BOTH]);
//         uint64_t moves = raw_moves & ~our_pieces;
//         int count = __builtin_popcount(moves);

//         game->mobility_score_mg[side] += count * MOBILITY_BONUS_MG[ROOK];
//         game->mobility_score_eg[side] += count * MOBILITY_BONUS_EG[ROOK];

//         uint64_t atk = moves & other_pieces;
//         if (atk){
//             PieceType p;
//             int pos = mva_from_attacker_mask(game, game->pieces[!side], atk, side, &p);

//             game->valuable_attacker_score[side] += ATTACKING_HIGHER_VALUE_BONUS[ROOK][p];
//         }

//         attack_mask |= raw_moves;
//     }
//     uint64_t queens = game->pieces[side][QUEEN];
//     while (queens){

//         int pos = pop_lsb(&queens);
//         uint64_t raw_moves = fetch_queen_moves(game, pos, game->board_pieces[BOTH]);
//         uint64_t moves = raw_moves & ~our_pieces;
//         int count = __builtin_popcount(moves);

//         game->mobility_score_mg[side] += count * MOBILITY_BONUS_MG[QUEEN];
//         game->mobility_score_eg[side] += count * MOBILITY_BONUS_EG[QUEEN];

//         uint64_t atk = moves & other_pieces;
//         if (atk){
//             PieceType p;
//             int pos = mva_from_attacker_mask(game, game->pieces[!side], atk, side, &p);

//             game->valuable_attacker_score[side] += ATTACKING_HIGHER_VALUE_BONUS[QUEEN][p];
//         }

//         attack_mask |= raw_moves;
//     }
//     int kpos = bit_scan_forward(&game->pieces[side][KING]);
    
//     attack_mask |= king_moves[kpos];

//     return attack_mask;

// }

void init_masks_per_piece(Game * game, Side side){
    
    uint64_t pawns = game->pieces[side][PAWN];
    
    int ekpos = __builtin_ctzll(game->pieces[!side][KING]);
    game->os.k_sq[!side] = ekpos;
    
    int kpos = __builtin_ctzll(game->pieces[side][KING]);
    game->os.k_sq[side] = kpos;
}

void init_evaluate(Game * game){
    
    Side side = game->side_to_move;
    game->os.psqt_score[WHITE] = 0;
    game->os.psqt_score[BLACK] = 0;
    game->os.material_score[WHITE] = 0;
    game->os.material_score[BLACK] = 0;
    game->phase = 0;

    for (int c = 0; c < 2; c++){

        for (int i = 0; i < PIECE_TYPES; i++){
            game->phase += phase_values[i] * __builtin_popcountll(game->pieces[c][i]);
        }
        
    }
    // double phase = (double)game->phase / MAX_PHASE;
    // double eg_phase = 1.0 - phase;
    ASSERT(side != BOTH);

    for (int i = 0; i < PIECE_TYPES; i++){
        uint64_t b = game->pieces[side][i];
        int count = 0;
        while(b){
            int pos = pop_lsb(&b);
            count += 1;

            game->os.psqt_score[side] += PSQT[side][i][pos];
            game->os.material_score[side] += eval_params[ep_idx.piece_values[i]];
            if (i == KING){
                
                game->os.k_sq[side] = pos;
            }
        }
    }
    for (int i = 0; i < PIECE_TYPES; i++){
        uint64_t b = game->pieces[!side][i];
        int count = 0;
        while(b){
            int pos = pop_lsb(&b);
            count += 1;
            game->os.psqt_score[!side] += PSQT[!side][i][pos];
            // game->os.material_score[!side] += PIECE_VALUES[i];
            game->os.material_score[!side] += eval_params[ep_idx.piece_values[i]];
            if (i == KING){
                game->os.k_sq[!side] = pos;
            }
        }
    }
    if (game->phase > 24){
        game->phase = 24;
    }

    // int ekpos = __builtin_ctzll(game->pieces[!side][KING]);
    // game->os.k_sq[!side] = 0;
    
    // int kpos = __builtin_ctzll(game->pieces[side][KING]);
    // ASSERT(kpos < 64 && kpos >= 0);
    ASSERT(game->pieces[side][KING]);
    ASSERT(game->pieces[!side][KING]);
    // ASSERT(kpos < 64 && kpos >= 0);
    // ASSERT(ekpos < 64 && ekpos >= 0);
    // game->os.k_sq[side] = 0;
    // init_masks_per_piece(game, WHITE);
    // init_masks_per_piece(game, BLACK);

    // for (int i = 0; i < 32; i++){
    //     printf("PIECE: %c at %d\n", PNAME[game->piece_info[i].p], game->piece_info[i].pos);
    //     print_board(game->piece_attacks[i], WHITE_KNIGHT);
        
    // }
    
}













