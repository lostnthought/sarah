#ifndef EVAL_H
#define EVAL_H

#include "types.h"
#include "utils.h"
#include "math.h"
#include "magic.h"
#include "game.h"
#include "move_generation.h"

/* 
    @brief used to initialize evaluation and phase values when we set the board to a new fen. used to be way more useful, since our material is no longer incremental due to computation demands.
*/
void init_evaluate(Game * game);

/* @brief static exchange evaluation. this is actually the heart of the engine, this controls the moves we let into our qsearch and helps in overall move ordering, by pushing obviously bad captures down. currently about 20% of our call graph */

// int see(Game *game, Move * move);

static inline int see(Game *game, Move * move) {
    // if (move->type != CAPTURE && move->promotion_capture != true && move->type != EN_PASSANT) return -1;
    int sq = move->end_index;
    int from = move->start_index;
    PieceType capture_piece = move->capture_piece;

    int gain[32];
    int depth = 0;

    int side = game->side_to_move;

    uint64_t occ = game->board_pieces[BOTH]; 
    uint64_t temp_pieces[COLOR_MAX][PIECE_TYPES];

    for (int i = 0; i < PIECE_TYPES; i++){
        temp_pieces[BLACK][i] = game->pieces[BLACK][i];
        temp_pieces[WHITE][i] = game->pieces[WHITE][i];
    }
    
    gain[depth] = piece_values_mg[capture_piece];

    occ ^= (1ULL << sq);
    temp_pieces[!side][capture_piece] ^= (1ULL << sq);  
    
    uint64_t attackers[2];
    attackers[WHITE] = attacker_mask_for_square(game, WHITE, temp_pieces[WHITE], move->end_index, occ);
    attackers[BLACK] = attacker_mask_for_square(game, BLACK, temp_pieces[BLACK], move->end_index, occ);

    side = !side; 

    while (true) {
        uint64_t att = attackers[side];
        if (!att || depth > 30) break;
        // if (!att) break;
        
        PieceType lva = (PieceType)0;
        int next_from = lva_from_attacker_mask(temp_pieces[side], att, &lva); 
        if (next_from == -1){
            printf("COULD NOT FIND ATTACKER. MASKS UNSYNCED. ERROR.\n");
            break;
        }
        if (next_from < 0 || next_from >= 64) {
            printf("BAD LVA from returned %d\n", next_from); abort();
        }

        depth++;
        gain[depth] = piece_values_mg[lva] - gain[depth-1];

        occ ^= (1ULL << next_from);
        temp_pieces[side][lva] ^= (1ULL << next_from);
        attackers[side] ^= (1ULL << next_from);

        uint64_t discovered_bishops = fetch_bishop_moves(game, sq, occ) & (temp_pieces[side][BISHOP] | temp_pieces[side][QUEEN]);
        uint64_t discovered_rooks = fetch_rook_moves(game, sq, occ) & (temp_pieces[side][ROOK] | temp_pieces[side][QUEEN]);
        attackers[side] |= discovered_bishops | discovered_rooks;
        
        uint64_t discovered_opp_bishops = fetch_bishop_moves(game, sq, occ) & (temp_pieces[!side][BISHOP] | temp_pieces[!side][QUEEN]);
        
        uint64_t discovered_opp_rooks = fetch_rook_moves(game, sq, occ) & (temp_pieces[!side][ROOK] | temp_pieces[!side][QUEEN]);
        attackers[!side] |= discovered_opp_bishops | discovered_opp_rooks;

        side = !side;
    }

    for (int i = depth - 1; i >= 0; i--) 
        gain[i] = -MAX(-gain[i], gain[i+1]);

    return gain[0];
}

/* 
  @brief does what you would expect, constants found in utils.
*/

static inline uint64_t evaluate_pawn_structure(Game * game, Side side, int * mg, int * eg){
    int score_mg = 0, score_eg = 0;

    // pawns
    // double phase = (double)game->phase / MAX_PHASE;
    // double eg_phase = 1.0 - phase;

    uint64_t pawns = game->pieces[side][PAWN];
    uint64_t friendly_pawns = game->pieces[side][PAWN];
    uint64_t enemy_pawns = game->pieces[!side][PAWN];
    for (int i = 0; i < 8; i++){
        int count = __builtin_popcountll(pawns & file_masks[i]);
        // doubled pawn
        if (count > 1) score_mg += (count - 1) * DOUBLED_PAWN_PENALTY_MG;
        if (count > 1) score_eg += (count - 1) * DOUBLED_PAWN_PENALTY_EG;
    }
    

    uint64_t attacks = 0;
    while (pawns){
        int pos = pop_lsb(&pawns);
        attacks |= pawn_captures[side][pos];
        
        int file = pos % 8;
        bool enemy_pawn_on_file = enemy_pawns & in_front_file_masks[side][pos];
        bool enemy_pawn_in_front_adjacent = enemy_pawns & adjacent_in_front_masks[side][pos];
        // isolated
        if(!(game->pieces[side][PAWN] & adjacent_file_masks[file])) {
            // score_mg += weights_mg[params.isolated_pawn[side]];
            // score_eg += weights_eg[params.isolated_pawn[side]];
            score_mg += ISOLATED_PAWN_PENALTY_MG;
            score_eg += ISOLATED_PAWN_PENALTY_EG;
            // t_score += texel_weights[params.isolated_pawn_eg] * eg_phase;
        }
        // passers
        if (!(game->pieces[!side][PAWN] & passed_pawn_masks[side][pos]) && !enemy_pawn_on_file) {
            // score_mg += weights_mg[params.passed_pawn[side]];
            // score_eg += weights_eg[params.passed_pawn[side]];
            score_mg += PASSED_PAWN_BONUS_MG;
            score_eg += PASSED_PAWN_BONUS_EG;
        }

        // backward pawn
        bool has_supporting_adjacent = (friendly_pawns & (adjacent_file_masks[file] & in_front_ranks_masks[side][pos])) != 0;


        // check if a forward square is is controlled by an attacking enemy pawn
        int forward_sq = push_direction[side] * 8 + pos;
        bool enemy_controls_front = false;
        
        if (forward_sq >= 0 && forward_sq < 64){
            // from our side looking foward diagonally
            enemy_controls_front = pawn_captures[side][forward_sq] & enemy_pawns;
        }

        // is backward
        if (!has_supporting_adjacent && (enemy_controls_front || game->piece_at[forward_sq] == PAWN)){
            score_mg += BACKWARD_PAWN_PENALTY_MG;
            score_eg += BACKWARD_PAWN_PENALTY_EG;
        }

        // candidate passer

        if (!enemy_pawn_on_file && !enemy_pawn_in_front_adjacent){
            score_mg += CANDIDATE_PASSER_BONUS_MG;
            score_eg += CANDIDATE_PASSER_BONUS_EG;
        }

        // chained pawn
        if (friendly_pawns & pawn_captures[side][pos]){
            
            score_mg += CHAINED_PAWN_BONUS_MG;
            score_eg += CHAINED_PAWN_BONUS_EG;
        }

        
        int kfile = game->pieces[!side][KING] % 8;
    }
    return attacks;
}

/* 
  @brief gets the space between both pawn lines and gives a bonus based on files that are closest to the middle 
  the goal with this function was to incentivize fighting for the center, especially when playing black, because I noticed a tendency to get caught in a blocked off position if we don't find early initiative. now with an opening book it doesn't *really* matter but I think it's still cool
*/

static inline int evaluate_pawn_space(Game * game, int * white, int  * black){
    
    uint64_t enemy_pawns = game->pieces[BLACK][PAWN];
    uint64_t friendly_pawns = game->pieces[WHITE][PAWN];
    int ws = 0, bs = 0;
    int sum_cnt_white = 0;
    int sum_cnt_black = 0;
    for (int f = 0; f < 8; f++){
        int fr = 0;
        int er = 0;

        uint64_t fp = (friendly_pawns & file_masks[f]);
        if (fp){
            int pos = bit_scan_forward(&fp);
            fr = abs(7-(pos/8));
            ws += (fr - 2) * PAWN_SPACE_FILE_WEIGHTS[f];
            sum_cnt_white++;
        }
        uint64_t ep = (enemy_pawns & file_masks[f]);
        if (ep){
            // find lowest
            int pos = bit_scan_backward(&ep);
            // invert to compare
            er = pos/8;
            bs += (er - 2) * PAWN_SPACE_FILE_WEIGHTS[f];
            sum_cnt_black++;
        }
    }

    *white = (int )ws / 8*4;
    *black = (int )bs / 8*4;
    return ws - bs;
}


/* @brief connected rooks and open file bonus eval */

static inline void evaluate_rook_files(Game * game, int side, int  * mg, int  * eg) {

    uint64_t pawns = game->pieces[side][PAWN];
    uint64_t enemy_pawns = game->pieces[!side][PAWN];
    uint64_t rooks = game->pieces[side][ROOK];

    
    while (rooks) {
        int sq = pop_lsb(&rooks);
        int file = sq % 8;
        int rank = sq / 8;

        bool mine = pawns & file_masks[file];
        bool theirs = enemy_pawns & file_masks[file];

        if (!mine) {
            if (!theirs) {
                *mg += ROOK_OPEN_FILE_BONUS;
                *eg += ROOK_OPEN_FILE_BONUS;
            } else {
                *mg += ROOK_SEMI_OPEN_FILE_BONUS;
                *eg += ROOK_SEMI_OPEN_FILE_BONUS;
            }
        }
        bool connected_rooks = fetch_rook_moves(game, sq, game->board_pieces[BOTH]) & game->pieces[side][ROOK];
        if (connected_rooks){
            *mg += CONNECTED_ROOK_BONUS;
            *eg += CONNECTED_ROOK_BONUS;
            
        }
    }
}

/* @brief just a function that gives a bonus for late game king activity. this is supposed to just be handled by psqt, but I like to keep the king near to promoting pawns, especially stopping enemy promotions, with our lack of endgame tables */

static inline int evaluate_endgame_king(Game * game, Side side){
    
    const int MAX_MANHATTAN_DIST = 14;
    const int MAX_CENTER_MANHATTAN_DIST = 6;
    
    // get closest pawns to promotion and give a bonus for distance to it
    // TODO make the curve exponential and not linear
    int score = 0;
    int king_pos = __builtin_ctzll(game->pieces[side][KING]);
    int enemy_king_pos = __builtin_ctzll(game->pieces[!side][KING]);
    uint64_t pawns = game->pieces[side][PAWN];
    while (pawns){
        int pos = pop_lsb(&pawns);
        int pawn_val = PAWN_STORM_PSQT_EG[side][pos];

        // not close enough to promotion to be considered
        if (pawn_val < 4) continue;
        int dist = manhattan_distance[pos][king_pos];
        score += ((MAX_MANHATTAN_DIST - dist) * pawn_val) / 3;
        int enemy_dist = manhattan_distance[pos][enemy_king_pos];
        score -= ((MAX_MANHATTAN_DIST - enemy_dist) * pawn_val) / 3;
    }
    int dist_to_center = center_manhattan_distance[king_pos];
    score += DIST_TO_CENTER_BONUS * (MAX_CENTER_MANHATTAN_DIST - dist_to_center);

    return score;
}


static inline int evaluate_early_game_development(Game * game, Side side){

    int eval = 0;
    for (int i = KNIGHT; i < KING; i++){
        if (STARTING_SQUARES[side][i] & game->pieces[side][i]){
            eval += STARTING_SQUARE_VALUES[i];
        }
    }
    return eval;
    
}



/* 
  @brief evaluates pins to a side's queen and king
  @return a score 
*/
static inline int  evaluate_pins(Game * game, Side side){

    int ksq = 0;
    int qsq = 0;
    uint64_t queens = game->pieces[side][QUEEN];
    uint64_t pins = 0;
    while (queens){
        int pos = pop_lsb(&queens);
        pins |= piece_is_pinned(game, side, pos);
    }
    ksq = bit_scan_forward(&game->pieces[side][KING]);
    pins |= piece_is_pinned(game, side, ksq);
    int  pin_score = 0.0f;
    pin_score += PIN_PENALTY * bit_count(pins);
    return pin_score;
}

/* 
    @brief material and mobility eval, also to avoid unnecessary looping, we compute additional things such as tempo attacks based on piece values, and batteries and bishop pair etc.
    @param enemy_pawn_attacks attack mask from the pawn hash that helps us compute a penalty for pawns that restrict our mobility
    @return an attack mask used for later tempo eval 
*/
// uint64_t evaluate_material_weighted(Game * game, Side side, int  * out_mg, int  * out_eg, uint64_t enemy_pawn_attacks);

static inline uint64_t evaluate_material_weighted(Game * game, Side side, int  * out_mg, int  * out_eg, uint64_t enemy_pawn_attacks){
    
    const int MAX_MANHATTAN_DIST = 14;
    uint64_t both = game->board_pieces[BOTH];
    uint64_t our_pieces = game->board_pieces[side];
    uint64_t other_pieces = game->board_pieces[!side];
    int  mg = 0, eg = 0;
    uint64_t pawns = game->pieces[side][PAWN];
    int pawn_count = bit_count(pawns);
    uint64_t attack_mask = 0;

    int psqt_sign = 1;

    
    // this is here because of the texel tuning convention that our psqts end up negative for black. we still evaluate positive = white, but for simplicity's sake, we only subtract *after* this function returns
    if (side == BLACK) psqt_sign = -1;

    int moves_blocked_by_enemy_pawns = 0;
    int enemy_king_sq = bit_scan_forward(&game->pieces[!side][KING]);


    uint64_t enemy_king_zone = king_zone_masks[enemy_king_sq];
    int  king_threat_level = 0;
    
    
    while (pawns){
        int pos = pop_lsb(&pawns);
        uint64_t moves = pawn_moves[side][pos] & ~our_pieces;
        mg += PSQT_MG[side][PAWN][pos];
        eg += PSQT_EG[side][PAWN][pos];
        uint64_t atk = moves & other_pieces;
        
        if (atk){
            // int attack_pos = pop_lsb(&atk);
            PieceType p;
            mva_from_attacker_mask(game, game->pieces[!side], atk, side, &p);
            mg += ATTACKING_HIGHER_VALUE_BONUS[PAWN][p];
            eg += ATTACKING_HIGHER_VALUE_BONUS[PAWN][p];
        }

        attack_mask |= pawn_captures[side][pos];
    }
    uint64_t knights = game->pieces[side][KNIGHT];
    int knight_count = bit_count(knights);
    while (knights){
        int pos = pop_lsb(&knights);
        uint64_t moves = knight_moves[pos] & ~our_pieces;
        int count = __builtin_popcountll(moves);
        mg += PSQT_MG[side][KNIGHT][pos];
        eg += PSQT_EG[side][KNIGHT][pos];
        mg += count * MOBILITY_BONUS_MG[KNIGHT];
        eg += count * MOBILITY_BONUS_EG[KNIGHT];
        attack_mask |= knight_moves[pos];
        uint64_t attack = knight_moves[pos] & other_pieces;
        if(attack){
            PieceType p;
            mva_from_attacker_mask(game, game->pieces[!side], attack, side, &p);
            mg += ATTACKING_HIGHER_VALUE_BONUS[KNIGHT][p];
            eg += ATTACKING_HIGHER_VALUE_BONUS[KNIGHT][p];
        }
    }
    uint64_t bishops = game->pieces[side][BISHOP];
    int bishop_count = 0;
    while (bishops){
        int pos = pop_lsb(&bishops);
        bishop_count++;
        uint64_t raw_moves = fetch_bishop_moves(game, pos, both);
        uint64_t moves = raw_moves & ~our_pieces;
        int count = bit_count(moves);
        mg += PSQT_MG[side][BISHOP][pos];
        eg += PSQT_EG[side][BISHOP][pos];
        mg += count * MOBILITY_BONUS_MG[BISHOP];
        eg += count * MOBILITY_BONUS_EG[BISHOP];
        attack_mask |= raw_moves;
        uint64_t attack = raw_moves & other_pieces;
        if(attack){
            PieceType p;
            mva_from_attacker_mask(game, game->pieces[!side], attack, side, &p);
            mg += ATTACKING_HIGHER_VALUE_BONUS[BISHOP][p];
            eg += ATTACKING_HIGHER_VALUE_BONUS[BISHOP][p];
        }
    }
    if  (bishop_count >= 2){
        mg += BISHOP_PAIR_BONUS;
        eg += BISHOP_PAIR_BONUS;
    }
    uint64_t rooks = game->pieces[side][ROOK];
    int rook_count = bit_count(rooks);
    while (rooks){
        int pos = pop_lsb(&rooks);
        uint64_t raw_moves = fetch_rook_moves(game, pos, both);
        uint64_t moves = raw_moves & ~our_pieces;
        int count = bit_count(moves);
        mg += PSQT_MG[side][ROOK][pos];
        eg += PSQT_EG[side][ROOK][pos];
        mg += count * MOBILITY_BONUS_MG[ROOK];
        eg += count * MOBILITY_BONUS_EG[ROOK];
        attack_mask |= raw_moves;
        // we are attacking
        uint64_t attack = raw_moves & other_pieces;
        if(attack){
            PieceType p;
            mva_from_attacker_mask(game, game->pieces[!side], attack, side, &p);
            mg += ATTACKING_HIGHER_VALUE_BONUS[ROOK][p];
            eg += ATTACKING_HIGHER_VALUE_BONUS[ROOK][p];
        }
    }
    uint64_t queens = game->pieces[side][QUEEN];
    int queen_count = bit_count(queens);
    while (queens){
        int pos = pop_lsb(&queens);
        uint64_t rook_moves = fetch_rook_moves(game, pos, both);
        uint64_t bishop_moves =  fetch_bishop_moves(game, pos, both);
        uint64_t raw_moves = rook_moves | bishop_moves;
        uint64_t moves = raw_moves & ~our_pieces;
        int count = bit_count(moves);
        // int swap_pos = pos;
        mg += PSQT_MG[side][QUEEN][pos];
        eg += PSQT_EG[side][QUEEN][pos];
        mg += count * MOBILITY_BONUS_MG[QUEEN];
        eg += count * MOBILITY_BONUS_EG[QUEEN];
        attack_mask |= raw_moves;
        if (rook_moves & game->pieces[side][ROOK]){
            if (rook_moves & king_zone_masks[enemy_king_sq]){
                mg += KING_ZONE_BATTERY_BONUS;
                eg += KING_ZONE_BATTERY_BONUS;
            }
            
            mg += QUEEN_ROOK_CONNECTED_BONUS;
            eg += QUEEN_ROOK_CONNECTED_BONUS;
        }
        if (bishop_moves & game->pieces[side][BISHOP]){
            if (bishop_moves & king_zone_masks[enemy_king_sq]){
                mg += KING_ZONE_BATTERY_BONUS;
                eg += KING_ZONE_BATTERY_BONUS;
            }
            mg += QUEEN_BISHOP_CONNECTED_BONUS;
            eg += QUEEN_BISHOP_CONNECTED_BONUS;
        }
        // we are attacking
        uint64_t attack = raw_moves & other_pieces;
        if(attack){
            PieceType p;
            mva_from_attacker_mask(game, game->pieces[!side], attack, side, &p);
            mg += ATTACKING_HIGHER_VALUE_BONUS[QUEEN][p];
            eg += ATTACKING_HIGHER_VALUE_BONUS[QUEEN][p];
        }
    }
        
    int kpos = bit_scan_forward(&game->pieces[side][KING]);
    int swap_wkpos = kpos;
    // uint64_t king_moves = king_moves[kpos];
    mg += PSQT_MG[side][KING][kpos];
    eg += PSQT_EG[side][KING][kpos];
    attack_mask |= king_moves[kpos];
    

    
    *(out_mg) += mg;
    *(out_eg) += eg;
    return attack_mask;
}


/* @brief evaluates open files and diagonals near the king, and checks for attackers on them. it used to be much more verbose, which is why that n exists as a diagonal
TODO check forward diagonals in king push direction, not just + 8 */

 // void evaluate_king_pawn_safety(Game * game, Side side, int  * out_mg, int  * out_eg);
static inline void evaluate_king_pawn_safety(Game * game, Side side, int  * out_mg, int  * out_eg){
    
    int king_sq = bit_scan_forward(&game->pieces[side][KING]);
    uint64_t our_pawns = game->pieces[side][PAWN];
    uint64_t their_pawns = game->pieces[!side][PAWN];
    uint64_t pawn_shield =  game->pieces[side][PAWN] & pawn_shield_masks[side][king_sq];    
    int  mg = 0, eg = 0;
    int sign = 1;
    if (side == BLACK) sign = -1;
    int shield = bit_count(pawn_shield);
    int open_file_penalties = 0;

    int file = king_sq % 8;
    if (file >= 1){
        bool semi_open = !(our_pawns & file_masks[file - 1]);
        if (semi_open){
            if (!(their_pawns & file_masks[file - 1])){
                if ((file_masks[file - 1] & game->pieces[!side][ROOK]) || file_masks[file - 1] & game->pieces[!side][QUEEN]){
                    mg += OPEN_FILE_NEAR_KING_WITH_ATTACKER;
                    eg += OPEN_FILE_NEAR_KING_WITH_ATTACKER;
                    
                } else {
                    mg += OPEN_FILE_NEAR_KING_PENALTY;
                    eg += OPEN_FILE_NEAR_KING_PENALTY;
                    
                }
            } else {
                mg += SEMI_OPEN_FILE_NEAR_KING_PENALTY;
                eg += SEMI_OPEN_FILE_NEAR_KING_PENALTY;
            }
        }
    }
    if (file <= 6) {
        
        bool semi_open = !(our_pawns & file_masks[file + 1]);
        if (semi_open){
            if (!(their_pawns & file_masks[file + 1])){
                if ((file_masks[file + 1] & game->pieces[!side][ROOK]) || file_masks[file + 1] & game->pieces[!side][QUEEN]){
                    mg += OPEN_FILE_NEAR_KING_WITH_ATTACKER;
                    eg += OPEN_FILE_NEAR_KING_WITH_ATTACKER;
                    
                } else {
                    mg += OPEN_FILE_NEAR_KING_PENALTY;
                    eg += OPEN_FILE_NEAR_KING_PENALTY;
                    
                }
            } else {
                mg += SEMI_OPEN_FILE_NEAR_KING_PENALTY;
                eg += SEMI_OPEN_FILE_NEAR_KING_PENALTY;
            }
        }
    }

    bool semi_open = !(our_pawns & file_masks[file]);
    if (semi_open){
        if (!(their_pawns & file_masks[file])){
                if ((file_masks[file] & game->pieces[!side][ROOK]) || file_masks[file] & game->pieces[!side][QUEEN]){
                    mg += OPEN_FILE_FROM_KING_WITH_ATTACKER;
                    eg += OPEN_FILE_FROM_KING_WITH_ATTACKER;
                    
                } else {
                    mg += OPEN_FILE_NEAR_KING_PENALTY;
                    eg += OPEN_FILE_NEAR_KING_PENALTY;
                    
                }
                mg += OPEN_MIDDLE_FILE_NEAR_KING_ADDITIONAL_PENALTY;
                eg += OPEN_MIDDLE_FILE_NEAR_KING_ADDITIONAL_PENALTY;
            } else {
            mg += SEMI_OPEN_FILE_NEAR_KING_PENALTY;
            eg += SEMI_OPEN_FILE_NEAR_KING_PENALTY;
        }
    }

    // diagonals
    int open_diagonal_penalties = 0;
    int n = king_sq + push_direction[side] * 8;
    // int e = king_sq + 1;
    // int w = king_sq - 1;
    // int s = king_sq - 8;
    if (n >= 0 && n <= 63){
        uint64_t diagonals_near_king_without_blockers = bishop_moves[n];
        bool semi_open_diagonal = !(diagonals_near_king_without_blockers & our_pawns);
        if (semi_open_diagonal){
            if (!(diagonals_near_king_without_blockers & their_pawns)){
                if ((diagonals_near_king_without_blockers & game->pieces[!side][QUEEN]) || diagonals_near_king_without_blockers & game->pieces[!side][BISHOP]){
                    mg += OPEN_DIAGONAL_NEAR_KING_WITH_ATTACKER;
                    eg += OPEN_DIAGONAL_NEAR_KING_WITH_ATTACKER;
                
                } else {
                    mg += OPEN_DIAGONAL_NEAR_KING_PENALTY;
                    eg += OPEN_DIAGONAL_NEAR_KING_PENALTY;
                
                }
            } else {
                mg += SEMI_OPEN_DIAGONAL_NEAR_KING_PENALTY;
                eg += SEMI_OPEN_DIAGONAL_NEAR_KING_PENALTY;
            
            }
        }
    }
    
    uint64_t diagonals_from_king_without_blockers = bishop_moves[king_sq];
    if (!(diagonals_from_king_without_blockers & our_pawns)){
        if (!(diagonals_from_king_without_blockers & their_pawns)){
            if ((diagonals_from_king_without_blockers & game->pieces[!side][QUEEN]) || diagonals_from_king_without_blockers & game->pieces[!side][BISHOP]){
                mg += OPEN_DIAGONAL_FROM_KING_WITH_ATTACKER;
                eg += OPEN_DIAGONAL_FROM_KING_WITH_ATTACKER;
                
            } else {
                mg += OPEN_DIAGONAL_FROM_KING_PENALTY;
                eg += OPEN_DIAGONAL_FROM_KING_PENALTY;
                
            }
        } else {
            mg += SEMI_OPEN_DIAGONAL_NEAR_KING_PENALTY;
            eg += SEMI_OPEN_DIAGONAL_NEAR_KING_PENALTY;
            
        }
    }
    
    // make this other piece bonus have a bigger ring than just the ring so it doesn't make us cramp our king
    // int shield_score = (shield) * texel_weights[params.pawn_shield] * sign; 
    // return ss + open_file_penalties + open_diagonal_penalties + shield;
    *out_mg += mg;
    *out_eg += eg;
}



/* @brief evaluates overall safety by usual defender-attacker ideas, used to use manhattan distance but it was overengineered. needs work though */
// void evaluate_king_safety(Game *game, Side side, int  * mg, int  * eg, uint64_t our_attack_mask, uint64_t enemy_attack_mask);
static inline void evaluate_king_safety(Game *game, Side side, int  * mg, int  * eg, uint64_t our_attack_mask, uint64_t enemy_attack_mask){
    // if (game->phase < 10) return 0;

    int king_sq = bit_scan_forward(&game->pieces[side][KING]);

    int  attack_score = 0;
    // small penalty for squares attacked inside of the king zone
    int far_attacks = bit_count(king_zone_masks[king_sq] & enemy_attack_mask);
    int zone_attacks = bit_count(king_zone_masks[king_sq] & game->board_pieces[!side]);
    attack_score += zone_attacks * KING_ZONE_ATTACK_PENALTY;
    


    int  defense_score = 0;
    int close_defenders = bit_count(king_zone_masks[king_sq] & our_attack_mask);
    int zone_defense = bit_count(king_zone_masks[king_sq] & game->board_pieces[side]);
    defense_score += zone_defense * KING_ZONE_DEFENSE_SCORE + close_defenders * 0.25;
    

    
    *mg += attack_score + defense_score;
    *eg += attack_score + defense_score;
}
/* 
    @brief the giant eval function. uses the pawn hash to help out on the load. used to be in int space but now due to texel complication uses int s. i intend to go back, however. returns based on side to move, but everything is calculated from white as positive
    @param king_threat_value a pointer used for reducing pruning when a king is under heavy threat (wip)
*/

static inline int evaluate(Game * game, Side side, SearchData * search_data, int * king_threat_value){
    int w_mat_mg = 0;
    int w_mat_eg = 0;
    int b_mat_mg = 0;
    int b_mat_eg = 0;
    int sign = 1;
    int mg = 0;
    int eg = 0;


    int p_mg = 0; int p_eg = 0;
    int king_pawn_shield = 0;
    // search for pawn hash entry
    int  pawn_score = 0;
    PawnHashEntry * entry = NULL;
    uint64_t black_pawn_attacks = 0;
    uint64_t white_pawn_attacks = 0;
    int ktv = 0;
    entry = search_for_pawn_hash_entry(game, game->pawn_key);
    search_data->pawn_hash_probes += 1;
    if (entry){
        search_data->pawn_hash_hits += 1;
        black_pawn_attacks = entry->b_pawn_attacks;
        white_pawn_attacks = entry->w_pawn_attacks;
        king_pawn_shield = entry->king_safety;
        mg += entry->mg_score;
        eg += entry->eg_score;
    } else {
        
        int w_ps_mg = 0, w_ps_eg = 0;
        int b_ps_mg = 0, b_ps_eg = 0;
        white_pawn_attacks = evaluate_pawn_structure(game, WHITE, &w_ps_mg, &w_ps_mg); 
        black_pawn_attacks = evaluate_pawn_structure(game, BLACK, &b_ps_mg, &b_ps_eg);
        int b_sp = 0;
        int w_sp = 0;
        // evaluate_pawn_space(game,&w_sp, &b_sp);
        int pawn_mg = w_ps_mg - b_ps_mg + w_sp - b_sp;
        int pawn_eg = w_ps_eg - b_ps_eg + w_sp - b_sp;
        create_new_pawn_hash_entry(game, game->pawn_key, pawn_mg, pawn_eg, king_pawn_shield, white_pawn_attacks, black_pawn_attacks);
        mg += pawn_mg;
        eg += pawn_eg;
    }

    uint64_t w_attack_mask = evaluate_material_weighted(game, WHITE, &w_mat_mg, &w_mat_eg, black_pawn_attacks);
    uint64_t b_attack_mask = evaluate_material_weighted(game, BLACK, &b_mat_mg, &b_mat_eg, white_pawn_attacks);

    // // evaluate hanging pieces
    uint64_t w_undefended = game->board_pieces[WHITE] ^ (game->board_pieces[WHITE] & w_attack_mask);
    uint64_t b_undefended = game->board_pieces[BLACK] ^ (game->board_pieces[BLACK] & b_attack_mask);
    int w_hanging = 0;
    int b_hanging = 0;
    while (w_undefended){
        int pos = pop_lsb(&w_undefended);
        if ((1ULL << pos) & b_attack_mask){
            
            w_hanging += HANGING_PIECE_PENALTY[game->piece_at[pos]] * 2;
        } else {
            
            w_hanging += HANGING_PIECE_PENALTY[game->piece_at[pos]];
        }
    }
    while (b_undefended){
        int pos = pop_lsb(&b_undefended);
        if ((1ULL << pos) & w_attack_mask){
            
            b_hanging += HANGING_PIECE_PENALTY[game->piece_at[pos]] * 2;
        } else {
            
            b_hanging += HANGING_PIECE_PENALTY[game->piece_at[pos]];
        }
    }
    mg += w_hanging - b_hanging;
    eg += w_hanging - b_hanging;


    
    int w_attacked_by_pawns = bit_count(black_pawn_attacks & (game->board_pieces[WHITE] & ~game->pieces[WHITE][PAWN]));
    int b_attacked_by_pawns = bit_count(white_pawn_attacks & (game->board_pieces[BLACK] & ~game->pieces[BLACK][PAWN]));
    mg += w_attacked_by_pawns * PAWN_BLOCKERS_PENALTY - b_attacked_by_pawns * PAWN_BLOCKERS_PENALTY;
    eg += w_attacked_by_pawns * PAWN_BLOCKERS_PENALTY - b_attacked_by_pawns * PAWN_BLOCKERS_PENALTY;
    
    mg += w_mat_mg - b_mat_mg;
    eg += w_mat_eg - b_mat_eg;
    
    // mg += game->psqt_evaluation_mg[WHITE] - game->psqt_evaluation_mg[BLACK];
    // eg += game->psqt_evaluation_eg[WHITE] - game->psqt_evaluation_eg[BLACK];

    int w_rf_mg = 0, w_rf_eg = 0;
    int b_rf_mg = 0, b_rf_eg = 0;
    evaluate_rook_files(game, WHITE, &w_rf_mg, &w_rf_eg); 
    evaluate_rook_files(game, BLACK, &b_rf_mg, &b_rf_eg);
    mg += w_rf_mg - b_rf_mg;
    eg += w_rf_eg - b_rf_eg;
    int w_ks_mg = 0, w_ks_eg = 0;
    int b_ks_mg = 0, b_ks_eg = 0;
    evaluate_king_safety(game, WHITE, &w_ks_mg, &w_ks_eg, w_attack_mask, b_attack_mask);  
    evaluate_king_safety(game, BLACK, &b_ks_mg, &b_ks_eg, b_attack_mask, w_attack_mask);
    // evaluate_king_pawn_safety(game, WHITE, &w_ks_mg, &w_ks_eg);  
    // evaluate_king_pawn_safety(game, BLACK, &b_ks_mg, &b_ks_eg);
    if (side == WHITE){
        ktv += w_ks_mg;
    } else {
        ktv += b_ks_mg;
    }
    mg += w_ks_mg - b_ks_mg;
    eg += w_ks_eg - b_ks_eg;

    if (game->phase < 10){
        int  eg_king = evaluate_endgame_king(game, WHITE) - evaluate_endgame_king(game, BLACK);
        eg += eg_king;
        
    }

    // if (game->phase > 20){
    //     int eval = evaluate_early_game_development(game, WHITE) - evaluate_early_game_development(game, BLACK);
    //     mg += eval;
    //     eg += eval;
    // }
    
    int  pin_score = evaluate_pins(game, WHITE) - evaluate_pins(game, BLACK);
    mg += pin_score;
    eg += pin_score;


    int e = ((eg * (MAX_PHASE - game->phase)) + (mg * game->phase)) / MAX_PHASE;
    
    if (side == WHITE){
        
        return e;
    } else {
        
        return -e;
    }
}

static inline bool is_mate(int score){
    return score < -MATE_SCORE + 1 || score > MATE_SCORE - 1;
}
static inline bool is_safe(Game * game, Move * move){
    if (move->piece == KING){
        return true;
    } else if ((move->type == CAPTURE || move->promotion_capture) && piece_values_mg[move->capture_piece] >= piece_values_mg[move->piece]){
        return true;
    } else if (move->type == PROMOTION && move->promotion_type != QUEEN){
        return false;
    } else {
        return see(game, move) > 100;
    }
}
#endif
