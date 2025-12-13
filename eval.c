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


uint64_t init_material_eval(Game * game, Side side){
    
    uint64_t attack_mask = 0;
    uint64_t our_pieces = game->board_pieces[side];
    uint64_t other_pieces = game->board_pieces[!side];

    uint64_t pawns = game->pieces[side][PAWN];
    while (pawns){

        int pos = pop_lsb(&pawns);
        uint64_t moves = pawn_moves[side][pos] & ~our_pieces;

        uint64_t atk = moves & other_pieces;
        if (atk){
            PieceType p;
            int pos = mva_from_attacker_mask(game, game->pieces[!side], atk, side, &p);

            game->valuable_attacker_score[side] += ATTACKING_HIGHER_VALUE_BONUS[PAWN][p];
        }

        attack_mask |= pawn_captures[side][pos];
    }
    uint64_t knights = game->pieces[side][KNIGHT];
    while (knights){

        int pos = pop_lsb(&knights);
        uint64_t moves = knight_moves[pos] & ~our_pieces;
        int count = __builtin_popcount(moves);
        game->mobility_score_mg[side] += count * MOBILITY_BONUS_MG[KNIGHT];
        game->mobility_score_eg[side] += count * MOBILITY_BONUS_EG[KNIGHT];

        uint64_t atk = moves & other_pieces;
        if (atk){
            PieceType p;
            int pos = mva_from_attacker_mask(game, game->pieces[!side], atk, side, &p);

            game->valuable_attacker_score[side] += ATTACKING_HIGHER_VALUE_BONUS[KNIGHT][p];
        }

        attack_mask |= knight_moves[pos];
    }
    uint64_t bishops = game->pieces[side][BISHOP];
    while (bishops){

        int pos = pop_lsb(&bishops);
        uint64_t raw_moves = fetch_bishop_moves(game, pos, game->board_pieces[BOTH]);
        uint64_t moves = raw_moves & ~our_pieces;
        int count = __builtin_popcount(moves);

        game->mobility_score_mg[side] += count * MOBILITY_BONUS_MG[BISHOP];
        game->mobility_score_eg[side] += count * MOBILITY_BONUS_EG[BISHOP];

        uint64_t atk = moves & other_pieces;
        if (atk){
            PieceType p;
            int pos = mva_from_attacker_mask(game, game->pieces[!side], atk, side, &p);

            game->valuable_attacker_score[side] += ATTACKING_HIGHER_VALUE_BONUS[BISHOP][p];
        }

        attack_mask |= raw_moves;
    }
    uint64_t rooks = game->pieces[side][ROOK];
    while (rooks){

        int pos = pop_lsb(&rooks);
        uint64_t raw_moves = fetch_rook_moves(game, pos, game->board_pieces[BOTH]);
        uint64_t moves = raw_moves & ~our_pieces;
        int count = __builtin_popcount(moves);

        game->mobility_score_mg[side] += count * MOBILITY_BONUS_MG[ROOK];
        game->mobility_score_eg[side] += count * MOBILITY_BONUS_EG[ROOK];

        uint64_t atk = moves & other_pieces;
        if (atk){
            PieceType p;
            int pos = mva_from_attacker_mask(game, game->pieces[!side], atk, side, &p);

            game->valuable_attacker_score[side] += ATTACKING_HIGHER_VALUE_BONUS[ROOK][p];
        }

        attack_mask |= raw_moves;
    }
    uint64_t queens = game->pieces[side][QUEEN];
    while (queens){

        int pos = pop_lsb(&queens);
        uint64_t raw_moves = fetch_queen_moves(game, pos, game->board_pieces[BOTH]);
        uint64_t moves = raw_moves & ~our_pieces;
        int count = __builtin_popcount(moves);

        game->mobility_score_mg[side] += count * MOBILITY_BONUS_MG[QUEEN];
        game->mobility_score_eg[side] += count * MOBILITY_BONUS_EG[QUEEN];

        uint64_t atk = moves & other_pieces;
        if (atk){
            PieceType p;
            int pos = mva_from_attacker_mask(game, game->pieces[!side], atk, side, &p);

            game->valuable_attacker_score[side] += ATTACKING_HIGHER_VALUE_BONUS[QUEEN][p];
        }

        attack_mask |= raw_moves;
    }
    int kpos = bit_scan_backward(&game->pieces[side][KING]);
    
    attack_mask |= king_moves[kpos];

    return attack_mask;

}

void init_evaluate(Game * game){
    
    Side side = game->side_to_move;
    game->material_evaluation_mg[WHITE] = 0;
    game->material_evaluation_eg[WHITE] = 0;
    game->material_evaluation_mg[BLACK] = 0;
    game->material_evaluation_eg[BLACK] = 0;
    game->psqt_evaluation_mg[WHITE] = 0;
    game->psqt_evaluation_eg[WHITE] = 0;
    game->psqt_evaluation_mg[BLACK] = 0;
    game->psqt_evaluation_eg[BLACK] = 0;
    game->phase = 0;

    for (int c = 0; c < 2; c++){

        for (int i = 0; i < PIECE_TYPES; i++){
            game->phase += phase_values[i] * bit_count(game->pieces[c][i]);
        }
        
    }
    double phase = (double)game->phase / MAX_PHASE;
    double eg_phase = 1.0 - phase;

    for (int i = 0; i < PIECE_TYPES; i++){
        uint64_t b = game->pieces[side][i];
        int count = 0;
        while(b){
            int pos = pop_lsb(&b);
            count += 1;

            game->psqt_evaluation_mg[side] += PSQT_MG[side][i][pos];
            game->psqt_evaluation_eg[side] += PSQT_EG[side][i][pos];
        }
        // game->material_evaluation_mg[side] += piece_values_mg[i] * count;
        // game->material_evaluation_eg[side] += piece_values_eg[i] * count;
    }
    for (int i = 0; i < PIECE_TYPES; i++){
        uint64_t b = game->pieces[!side][i];
        int count = 0;
        while(b){
            int pos = pop_lsb(&b);
            count += 1;
            game->psqt_evaluation_mg[!side] += PSQT_MG[!side][i][pos];
            game->psqt_evaluation_eg[!side] += PSQT_EG[!side][i][pos];
        }
        // game->material_evaluation_mg[!side] += piece_values_mg[i] * count;
        // game->material_evaluation_eg[!side] += piece_values_eg[i] * count;
    }
    if (game->phase > 24){
        game->phase = 24;
    }

    game->attack_mask[side] = init_material_eval(game, side);
    game->attack_mask[!side] = init_material_eval(game, !side);
    
}













