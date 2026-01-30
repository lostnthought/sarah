#ifndef EVAL_H
#define EVAL_H

#include "types.h"
#include "utils.h"
#include "math.h"
#include "magic.h"
#include "game.h"
#include "move_generation.h"
#include "eval_constants.h"

/* 
    @brief used to initialize evaluation and phase values when we set the board to a new fen. used to be way more useful, since our material is no longer incremental due to computation demands.
*/
void init_evaluate(Game * game);


/* @brief this see was transformed from my own implementation to now be closer to stockfish's with early threshold outs. really though, it isn't much of an elo increase in my engine and sacrifices a bit of accuracy. i may end up using both versions later */

static inline bool see(Game *game, Move move, int16_t threshold) {

    // from SF: they propose that other moves such as en passants and promos pass for speed
    if (move_type(move) != NORMAL) return 0 >= threshold;

    uint8_t to = move_to(move);
    uint8_t from = move_from(move);
    bool cap = is_cap(game, move);

    PieceType piece = move_piece(game, move);
    PieceType capture_piece = move_cp(game, move);

    Side original_side = game->side_to_move;
    int side = original_side;

    int16_t val = 0;
    int16_t last_cap = 0;

    val += PVAL[capture_piece] - threshold;


    // if our val is somehow less than 0 (very rare), then even if we aren't captured we're still below the threshold
    if (val < 0) return false;

    val -= PVAL[piece];

    // if our val is greater than zero even after assuming we get captured, then we can early out as positive
    if (val >= 0) return true;
    
    uint64_t occ = game->board_pieces[BOTH]; 
    uint64_t temp_pieces[COLOR_MAX][PIECE_TYPES];

    for (int i = 0; i < PIECE_TYPES; i++){
        temp_pieces[BLACK][i] = game->pieces[BLACK][i];
        temp_pieces[WHITE][i] = game->pieces[WHITE][i];
    }
    if (cap){
        occ ^= bits[to];
        temp_pieces[!side][capture_piece] ^= (bits[to]);  
    }

    last_cap = val;

    occ ^= bits[from];
    temp_pieces[side][piece] ^= bits[from];  
    
    uint64_t attackers[2];
    attackers[WHITE] = attacker_mask_for_square(game, WHITE, temp_pieces[WHITE], to, occ);
    attackers[BLACK] = attacker_mask_for_square(game, BLACK, temp_pieces[BLACK], to, occ);

    side = !side; 
    int c = 0;

    while (true) {
        uint64_t att = attackers[side];

        // get rid of pinned pieces
        if (game->st->pinners[!side] & occ) att &= ~game->st->ci.blockers_for_king[side];
        
        // exit out if no attackers left
        if (!att || c > 30) break;
        
        
        PieceType lva = PAWN;
        int next_from = lva_from_attacker_mask(temp_pieces[side], att, &lva); 
        assert(next_from != -1);
        val = -val - 1 - PVAL[lva]; 

        // we have to swap side preemptively here just in case we need to swap back
        side = !side;

        // if our value becomes positive, due to negamax we can early out. I believe this is why there is a - 1 headroom on the val iteration.
        if (val >= 0){

            // this swaps back to represent the fact that we would be able to capture the king that just captured since the other side still has attackers
            if (lva == KING && attackers[side]) side = !side;
            break;

        }

        occ ^= (bits[next_from]);
        temp_pieces[!side][lva] ^= (bits[next_from]);
        attackers[!side] ^= (bits[next_from]);

        uint64_t discovered_bishops = fetch_bishop_moves(to, occ) & (temp_pieces[side][BISHOP] | temp_pieces[side][QUEEN]);
        uint64_t discovered_rooks = fetch_rook_moves(to, occ) & (temp_pieces[side][ROOK] | temp_pieces[side][QUEEN]);
        attackers[side] |= discovered_bishops | discovered_rooks;
        
        uint64_t discovered_opp_bishops = fetch_bishop_moves(to, occ) & (temp_pieces[!side][BISHOP] | temp_pieces[!side][QUEEN]);
        
        uint64_t discovered_opp_rooks = fetch_rook_moves(to, occ) & (temp_pieces[!side][ROOK] | temp_pieces[!side][QUEEN]);
        attackers[!side] |= discovered_opp_bishops | discovered_opp_rooks;
        c++;

    }

    // if the side isn't our original side, we win since they have no attackers. if it is our original side, we ran out of attackers.
    return original_side != side;
}



/* 
  @brief the pawn evaluation! used to be a lot more complicated, with bonuses for space, but now is very simple. we accumulate masks for passers and pawn attacks, and store them within the pawn hash table
*/

static inline Score evaluate_pawn_structure(Game * game, Side side, EvalMasks * masks){

    Score s = 0;

    uint64_t pawns = game->pieces[side][PAWN];
    uint64_t friendly_pawns = game->pieces[side][PAWN];
    uint64_t enemy_pawns = game->pieces[!side][PAWN];
    for (int i = 0; i < 8; i++){
        // doubled pawns
        uint8_t count = __builtin_popcountll(pawns & file_masks[i]);
        if (count > 1) s += (count - 1) * P_DOUBLED;
    }

    // cache pawn attacks
    uint64_t ra = !side ? (pawns << 7 & ~file_masks[7]) : pawns >> 9 & ~file_masks[7];
    uint64_t la = !side ? (pawns << 9 & ~file_masks[0]) : pawns >> 7 & ~file_masks[0];
    uint64_t attacks = ra | la;
    masks->am_p[side][PAWN] = attacks;

    // TODO pawn penalty for being attacked by pawns on the sides. to represent how easy it is to defend

    while (pawns){
        uint8_t pos = __builtin_ctzll(pawns);
        pawns = pawns & (pawns - 1);

        uint8_t file = SQ_TO_FILE[pos];
        uint8_t rank = side ? SQ_TO_RANK[pos] : SQ_TO_RANK[flip_square(pos)];
        bool enemy_pawn_on_file = enemy_pawns & in_front_file_masks[side][pos];
        bool enemy_pawn_in_front_adjacent = enemy_pawns & adjacent_in_front_masks[side][pos];
        bool is_passer = false;

        // passers
        // there was an idea here to evaluate isolated passers separately since they are commonly better than just isolated pawns, but it never worked in testing
        if (!(game->pieces[!side][PAWN] & passed_pawn_masks[side][pos]) && !enemy_pawn_on_file) {
            
            s += eval_params[ep_idx.passed_pawn[rank]];
            masks->passers[side] |= bits[pos];
            is_passer = true;

        }

        // isolated
        if(!(game->pieces[side][PAWN] & adjacent_file_masks[file])) {
                s += eval_params[ep_idx.isolated_pawn[rank]];
        }

        // backward pawn
        bool has_supporting_adjacent = (friendly_pawns & (adjacent_file_masks[file] & in_front_ranks_masks[side][pos]));


        // check if a forward square is is controlled by an attacking enemy pawn
        int forward_sq = push_direction[side] * 8 + pos;
        bool enemy_controls_front = false;
        
        if (forward_sq >= 0 && forward_sq < 64){
            // from our side looking foward diagonally
            enemy_controls_front = pawn_captures[side][forward_sq] & enemy_pawns;
        }

        if (!has_supporting_adjacent && (enemy_controls_front || (bits[forward_sq] & game->pieces[!side][PAWN]))){
            s += eval_params[ep_idx.backward_pawn[rank]];
        }

        // candidate passer
        if (!enemy_pawn_on_file && !enemy_pawn_in_front_adjacent && !is_passer){
            s += eval_params[ep_idx.candidate_passer[rank]];
        }

        // chained pawn
        if (friendly_pawns & pawn_captures[side][pos]){
            s += eval_params[ep_idx.chained_pawn];
        }

    }
    return s;
}


static inline void generate_attack_mask_and_eval_mobility(Game * game, Side side, EvalMasks * masks, uint64_t piece_attacks[COLOR_MAX][PIECE_TYPES][PIECE_MAX]){
    
    uint64_t attacked_squares = 0;
    uint64_t datk = 0;

    // pawns calced and cached in hash
    masks->am_nk[side] |= masks->am_p[side][PAWN];
    attacked_squares |= masks->am_p[side][PAWN];

    uint64_t occ = game->board_pieces[BOTH];
        
    
    // iterate over the piece list and push moves to respective piece masks
    for (int p = KNIGHT; p < KING; p++){
        uint8_t * pl = game->piece_list[side][p];
        uint8_t idx = 0;
        for (uint8_t pos = *pl; pos != SQ_NONE; pos = *++pl){

            uint64_t moves = moves_at(side, pos, (PieceType)p, occ);
            // square was already attacked -> double attack
            datk |= attacked_squares & moves;

            // attack mask for side
            attacked_squares |= moves;

            // attack mask (piece type)
            masks->am_p[side][p] |= moves;

            // attack mask (no king)                
            masks->am_nk[side] |= moves;

            // push masks onto per piece masks
            piece_attacks[side][p][idx] = moves;

            // used to determine queen attack batteries. if a queen moves to a square, we need to know if it is defended, but we can only know that if we remove the queen and check behind it.
            if (p == BISHOP || p == ROOK){
                uint64_t xr = moves_at(side, pos, (PieceType)p, occ ^ game->pieces[side][QUEEN]);
                masks->am_xr_nq[side] |= xr;
            }
            idx++;
    
        }
    }

    // king moves
    uint64_t moves = king_moves[game->st->k_sq[side]];
    datk |= attacked_squares & moves;
    attacked_squares |= moves;
    masks->am_p[side][KING] |= moves;
    piece_attacks[side][KING][game->piece_index[game->st->k_sq[side]]] = moves;
    masks->am[side] = attacked_squares;
    masks->datk[side] = datk;
    
    // attack mask (minors)
    masks->am_m[side] = masks->am_p[side][BISHOP] | masks->am_p[side][KNIGHT];

    // threat deficit penalty (squares we attack less)
    masks->tdp[side] = (masks->am[!side] & ~masks->am[side]) | (masks->datk[!side] & ~masks->datk[side]);

    // smaller tdp
    masks->safe[side] = ~masks->am[!side] | masks->am[side];

    // pawn pushes (with occupancy)
    uint64_t pp = pawn_pushes(side, game->pieces[side][PAWN], game->board_pieces[BOTH]);

    // safe pawn push attacks
    masks->sppa[side] = pawn_attacks(side, pp & masks->safe[side]);
    
    // weak squares (used for outposts later)
    masks->wsq[!side] = masks->am_p[side][PAWN] & ~masks->am_p[!side][PAWN];

}


/* king threat is evaluated as positive for the aggressor's side, so side is the attacking side and !side is the defending side. */


static inline Score evaluate_king_threat(Game * game, Side side, EvalMasks * masks, uint64_t piece_attacks[COLOR_MAX][PIECE_TYPES][PIECE_MAX]){

    Score s = 0;

    // init values
    uint8_t enemy_king_sq = game->st->k_sq[!side];
    uint64_t ek = game->pieces[!side][KING];
    uint64_t enemy_king_zone = king_zone_masks[!side][enemy_king_sq];


    // make masks for each checking mask (TODO: just use the state->checkinfo for this)
    uint64_t checks[PIECE_TYPES];
    checks[PAWN] = pawn_captures[!side][enemy_king_sq];
    checks[KNIGHT] = knight_moves[enemy_king_sq];
    uint64_t bm = fetch_bishop_moves(enemy_king_sq, game->board_pieces[BOTH]);
    uint64_t rm = fetch_rook_moves(enemy_king_sq, game->board_pieces[BOTH]);
    checks[BISHOP] = bm;
    checks[ROOK] = rm;
    checks[QUEEN] = bm | rm;
    checks[KING] = 0;
    bool is_stm = side == game->side_to_move;

    uint64_t weak_sq = masks->am[side] & ~masks->datk[!side] & (~masks->am[!side] | masks->am_p[!side][QUEEN] | masks->am_p[!side][KING]);
    uint64_t safe = masks->tdp[!side];

    uint64_t safe_attacks = weak_sq & enemy_king_zone;
    uint8_t sa = __builtin_popcountll(safe_attacks);
    uint64_t double_safe = masks->datk[side] & weak_sq & enemy_king_zone;
    uint8_t dsafe = __builtin_popcountll(double_safe);
    sa -= dsafe;

    // double and safe king attacks to the king zone, since most of the rest only deals with the exact squares around the king. this provides enough to decide our presence next to the king
    s += eval_params[ep_idx.ks_safe_king_attacks[MIN(sa, 15)]];
    s += eval_params[ep_idx.ks_double_safe_king_attacks[MIN(__builtin_popcountll(double_safe), 15)]];
    
    // checks and contact checks
    // a check is safe if it is not tdped
    uint64_t pcheck = checks[PAWN] & masks->am_p[side][PAWN];
    if (pcheck){
        if (pcheck & safe){
            s += eval_params[ep_idx.ks_safe_checks[PAWN]];
        } else {
            
            s += eval_params[ep_idx.ks_unsafe_checks[PAWN]];
        }
    }
    uint64_t ncheck = checks[KNIGHT] & masks->am_p[side][KNIGHT];
    if (ncheck){
        if (ncheck & safe){
            s += eval_params[ep_idx.ks_safe_checks[KNIGHT]];
        } else {
            s += eval_params[ep_idx.ks_unsafe_checks[KNIGHT]];
        }
    }

    

    uint64_t bcheck = checks[BISHOP] & masks->am_p[side][BISHOP];
    if (bcheck){
        if (bcheck & safe){
            s += eval_params[ep_idx.ks_safe_checks[BISHOP]];
        } else {
            s += eval_params[ep_idx.ks_unsafe_checks[BISHOP]];
        }
    }

    uint64_t rcheck = checks[ROOK] & masks->am_p[side][ROOK];
    if (rcheck){
        if (rcheck & safe){
            s += eval_params[ep_idx.ks_safe_checks[ROOK]];
        } else {
            
            s += eval_params[ep_idx.ks_unsafe_checks[ROOK]];
        }
        // TODO this should probably be masks->datk since we already attack this square with the rook
        if (rcheck & king_moves[enemy_king_sq] & (~masks->am_nk[!side] & masks->am[side])){
            s += eval_params[ep_idx.ks_rook_contact_check];
        }
    }
    uint64_t qcheck = checks[QUEEN] & masks->am_p[side][QUEEN];
    if (qcheck){
        if (qcheck & safe){
            s += eval_params[ep_idx.ks_safe_checks[QUEEN]];
        } else {
            
            s += eval_params[ep_idx.ks_unsafe_checks[QUEEN]];
        }

        // uses the xr mask to determine if we have backup on the contact square
        uint64_t qm = qcheck & king_moves[enemy_king_sq];
        if (qm & ((~masks->am_nk[!side] & masks->am[side]) | (qm & masks->am_xr_nq[side]))){
            s += eval_params[ep_idx.ks_queen_contact_check];
        }
    }

    // correction for hanging pieces
    // extra correction for hanging pieces next to the king. they might blow up threat but actually just be hanging and taken in the next turn. so this avoids the horizon issue
    for (int p = KNIGHT; p < KING; p++){
        uint8_t * pl = game->piece_list[side][p];
        for (uint8_t pos = *pl; pos != SQ_NONE; pos = *++pl){
            uint64_t mz = piece_attacks[side][p][game->piece_index[pos]] & enemy_king_zone;
            if (mz){

                if ((bits[pos] & masks->tdp[side] & king_moves[enemy_king_sq]) && side != game->side_to_move){
                    s += eval_params[ep_idx.ks_threat_correction[p]];

                } else if (mz & king_moves[enemy_king_sq]){
                    s += eval_params[ep_idx.ks_akz[p]];
                }
            }
        }
    }

    // emtpy squares the king can escape to
    uint8_t empty = __builtin_popcountll(king_moves[enemy_king_sq] & ~game->board_pieces[BOTH] & ~masks->am[side]);

    // defended squares in the king zone
    uint8_t enemy_king_defended_squares = __builtin_popcountll(enemy_king_zone & masks->am_nk[!side] | (king_moves[enemy_king_sq] & ~masks->am[side]));

    // pawn shield only relevant early and actually gets negated late
    uint64_t pawn_shield =  game->pieces[!side][PAWN] & pawn_shield_masks[!side][enemy_king_sq];    
    uint8_t shield = __builtin_popcountll(pawn_shield);

    // king moves defended by a minor piece (minor = better)
    uint8_t def_minor = __builtin_popcountll(masks->am_m[!side] & king_moves[enemy_king_sq]);

    // double attacked squares next to the king
    uint64_t wksq = masks->datk[side] & ~masks->am_nk[!side] & king_moves[enemy_king_sq];

    // nonpawn attackers in the king zone
    uint8_t aikz = __builtin_popcountll(game->board_pieces[side] & enemy_king_zone & ~game->pieces[side][PAWN]);

    // sum all of them up
    s += eval_params[ep_idx.ks_aikz[MIN(aikz, 7)]];
    s += eval_params[ep_idx.ks_defended_squares_kz[MIN(enemy_king_defended_squares, 23)]];
    s += eval_params[ep_idx.ks_def_minor[MIN(def_minor, 7)]];
    if (more_than_one(wksq)){
        s += eval_params[ep_idx.ks_weak];
    }
    s += eval_params[ep_idx.ks_pawn_shield[MIN(shield, 3)]];
    s += eval_params[ep_idx.ks_empty[MIN(empty, 7)]];

    return s;
}

// this idea was taken straight from kohai but isn't in use anymore. it found if a king was able to stop a pawn in endgame, but in texel tuning I found this was often getting poor signal and sometimes even getting heavily negated. the code definitely worked but I think there are too many edge cases for it to work properly.

static inline bool material_is_lone_king(Game * game, Side side){
    if (game->pieces[side][KNIGHT] == 0 && game->pieces[side][BISHOP] == 0 && game->pieces[side][ROOK] == 0 && game->pieces[side][QUEEN] == 0) return true;
    return false;
}
static inline bool passer_is_unstoppable(Game * game, Side side, int pos, EvalMasks * masks){
    if (!material_is_lone_king(game, (Side)!side)) return false;

    if (game->board_pieces[BOTH] & in_front_file_masks[side][pos] || masks->am[!side] & in_front_file_masks[side][pos]) return false; 

    if (manhattan_distance[game->st->k_sq[!side]][pos] >= 2) return true;
    if (side ? SQ_TO_RANK[pos] >= 5 : SQ_TO_RANK[pos] <= 2 && (king_moves[game->st->k_sq[!side]] & in_front_file_masks[side][pos]) == 0) return true;

    return false;
}


static inline int evaluate_passers(Game * game, Side side, EvalMasks * masks){

    Score s = 0;
    uint64_t passers = masks->passers[side];
    int passers_on_seven = 0;
    uint8_t pr_cnt = 0;

    // loop through all passers
    while (passers){

        int pos = __builtin_ctzll(passers);
        passers = passers & (passers - 1);
        
        int rank = SQ_TO_RANK[pos];
        if (rank == (side ? 6 : 1) || (passers_on_seven && side ? rank >= 5 : rank <= 2)){
            passers_on_seven++;
        }
        int file = SQ_TO_FILE[pos];
        int push = rank + push_direction[side];
        uint64_t f_sq = rank_masks[push] & file_masks[file];
        // if we are defended in front (can push)
        if (f_sq & masks->tdp[side]){
            s += eval_params[ep_idx.passer_deficit];
        } else if (f_sq & masks->am[side]){
            s += eval_params[ep_idx.passer_supported];
        }
        // if we are blocked by an enemy piece
        if (in_front_file_masks[side][pos] & game->board_pieces[!side]){
            s += eval_params[ep_idx.passer_blocked];
        }

        // if we are defended and about to promote, we are most likely locking enemy pieces into defending our promotion square. this bonus reflects that
        if (bits[pos] & ~masks->tdp[side] && (rank == 6 || rank == 1)){
            if (promotion_ranks[side] & game->pieces[!side][ROOK]){
                s += eval_params[ep_idx.opponent_rook_stuck_on_promo_rank];
            }
            
        }
        uint64_t rf = file_masks[file] & game->pieces[side][ROOK];
        
        if (rf){
            if (rf & ~in_front_file_masks[side][pos]){
                s += eval_params[ep_idx.rook_on_passer_file];
            }
            uint8_t rpos = __builtin_ctzll(rf);

            // defer this until after to check for other passers about to promote, since that makes this irrelevant
            if ((f_sq & bits[rpos]) && (rank == 6 || rank == 1)){
                pr_cnt += 1;
            }
            
        }
    }

    // more than one passer about to promote
    if (passers_on_seven > 1) s += eval_params[ep_idx.extra_passers_on_seven];

    // if there aren't multiple pawns about to promote, our rook might be stuck defending the pawn, which is especially bad if it is directly in front of it. not all of the time, though
    if (passers_on_seven <= 1 && pr_cnt) s += eval_params[ep_idx.rook_stuck_in_front_of_passer];

    return s;
    
}


/* @brief this used to be a much much bigger function using threat deficit and attacking weak pawns etc but now it is much smaller. due to testing and texel tuning, I discovered these heuristics weren't actually getting good signal at all, and the engine performs better without them. it's a shame since I think they were neat. I may try again some day. */

static inline Score evaluate_threats(Game * game, Side side, EvalMasks * masks){


    
    Score s = 0;
    // enemy pieces (no pawns)
    uint64_t ep_np = game->board_pieces[!side] & ~game->pieces[!side][PAWN];
    
    uint64_t b = ep_np & (masks->am_p[side][KNIGHT] | masks->am_p[side][BISHOP]);
    while (b){
        uint8_t pos = __builtin_ctzll(b);
        b = b & (b - 1);

        s += eval_params[ep_idx.threat_by_minor[game->piece_at[pos]]];
    }

    b = ep_np & (masks->am_p[side][ROOK] | masks->am_p[side][QUEEN]);

    while (b){
        uint8_t pos = __builtin_ctzll(b);
        b = b & (b - 1);

        s += eval_params[ep_idx.threat_by_major[game->piece_at[pos]]];
    }

    uint64_t edef = ep_np & (masks->am_p[!side][PAWN] | masks->datk[!side]);
    uint64_t weak = ep_np & ~edef & masks->tdp[!side];

    // if they have more than one weak piece (threat deficit, we are attacking multiple nonpawns at once)
    if (more_than_one(weak)){
        s += eval_params[ep_idx.weak_pieces];
    }
   
    // pawn push attacks against nonpawns
    uint8_t pp_c = __builtin_popcountll(masks->sppa[side] & ep_np);
    s += eval_params[ep_idx.safe_pawn_push_attacks[MIN(pp_c, 15)]];

    // safe pawns attacking nonpawns (they cannot be captured easily)
    uint64_t sp = game->pieces[side][PAWN] & masks->safe[side];
    uint8_t sp_c = __builtin_popcountll(pawn_attacks(side, sp) & ep_np);
    s += eval_params[ep_idx.safe_pawn_attacks[MIN(sp_c, 15)]];
    

    return s;
}

static inline Score evaluate_material(Game * game, Side side, EvalMasks * masks, uint64_t piece_attacks[COLOR_MAX][PIECE_TYPES][PIECE_MAX]){

    Score s = 0;
    uint64_t fp = game->board_pieces[side];
    uint64_t ep = game->board_pieces[!side];
    int phase = game->phase;
    uint64_t fpawns = game->pieces[side][PAWN];
    uint64_t epawns = game->pieces[!side][PAWN];
    uint8_t enemy_king_sq = __builtin_ctzll(game->pieces[!side][KING]);

    // bishop pair only when opposite colored
    int bc = 0;
    if (game->pieces[side][BISHOP] & COLOR_SQUARES[BLACK]){
        bc += 1;
    }
    if (game->pieces[side][BISHOP] & COLOR_SQUARES[WHITE]){
        bc += 1;
    }
    if (bc == 2){
        s += eval_params[ep_idx.bishop_pair];
    }

    // this idea is straight from stockfish, remove pawns that are stuck from our mobility count, since they are obviously less valuable
    uint64_t pfsq = game->pieces[side][PAWN] & (side ? game->board_pieces[BOTH] << 8 : game->board_pieces[BOTH] >> 8);
    // determine outpost mask (needs pawn push attacks from the enemy side to determine if the outpost could be refuted easily by simply pushing a pawn)
    uint64_t out = masks->wsq[!side] & center_squares_mask & ~masks->sppa[!side];

    // piece list iteration (SF style)
    for (int p = KNIGHT; p < KING; p++){
        uint8_t * pl = game->piece_list[side][p];
        int c = 0;
        for (uint8_t pos = *pl; pos != SQ_NONE; pos = *++pl){
            c++;
            uint64_t m = piece_attacks[side][p][game->piece_index[pos]];

            uint64_t mob = m & ~masks->am_p[!side][PAWN];
            uint64_t smob = m & ~masks->am[!side] & ~masks->tdp[side];
            int count = __builtin_popcountll(mob);
            uint64_t b = bits[pos];


            // mobility count
            s += eval_params[ep_idx.mobility[p][count]];

            // outpost eval
            if (p == BISHOP || p == KNIGHT){
                int file = SQ_TO_FILE[pos];
                if (p == KNIGHT){
                    if (b & out){
                        s += eval_params[ep_idx.outpost_occ];
                    }
                }

                // if (m & masks->out[side]){
                //     s += eval_params[ep_idx.outpost_control];
                // }
                // can we enter enemy territory? (3 ranks)
                if (!(mob & ET3[side])){
                    s += eval_params[ep_idx.minor_cannot_enter_et];
                }
            }


            // if no safe mobility, and we are already in a threat deficit square, give a penalty
            if (smob == 0 && (masks->tdp[side] & b)){
                s += eval_params[ep_idx.unstoppable_attack];
            }


            if (p == ROOK){
            
                int file = SQ_TO_FILE[pos];
                int rank = SQ_TO_RANK[pos];


                if (!(fpawns & file_masks[file])) {
                    if (!(epawns & file_masks[file])) {
                        s += eval_params[ep_idx.rook_open];
                    } else {
                        s += eval_params[ep_idx.rook_semi_open];
                    }
                }
                if (m & game->pieces[side][ROOK]){
                    s += eval_params[ep_idx.connected_rook];
            
                }
                
            } else if (p == QUEEN){
            
                // are we connected to sliders?
                if (rook_moves[pos] & m & game->pieces[side][ROOK]){
                    s += eval_params[ep_idx.qr_connected];
                }
                if (bishop_moves[pos] & m & game->pieces[side][BISHOP]){
                    s += eval_params[ep_idx.qb_connected];
                }

                // idea is from SF, using check info we can detect pins
                uint64_t qp;
                if (slider_blockers(game, (Side)!side, game->pieces[!side][ROOK] | game->pieces[!side][BISHOP], pos, &qp)){
                    s += eval_params[ep_idx.weak_queen];
                }
            }
            
        }
    }

    
    return s;

}




/* apply corrective history based on various different hashed score bonuses. basically the idea behind this is to minimize out evaluation's bias based on how far off we were during search */
static inline int corrhist_eval(Game * game, ThreadData * td, SearchStack * stack, Side side){
    const int w = 256;
    const int pw = 128;

    // pawns
    int16_t cp = td->corrhist_p[side][game->st->piece_key[PAWN] & CORRHIST_MASK] * sp.corr_pawn_weight / sp.corrhist_weight;

    // nonpawns
    int16_t cnp_w = td->corrhist_nonpawns_w[side][game->st->nonpawn_key[WHITE] & CORRHIST_MASK] * sp.corr_np_weight / sp.corrhist_weight;
    int16_t cnp_b = td->corrhist_nonpawns_b[side][game->st->nonpawn_key[BLACK] & CORRHIST_MASK] * sp.corr_np_weight / sp.corrhist_weight;

    // material
    // not in use
    int16_t cm = td->corrhist_material[side][game->st->material_key & CORRHIST_MASK] * sp.corr_mat_weight / sp.corrhist_weight;
    
    // kbn and kqr are taken straight from yukari. these are the highest performing corrhists and I can't thank them enough for the idea.

    // king bishop knight
    uint64_t kbn = game->st->piece_key[KING] ^ game->st->piece_key[BISHOP] ^ game->st->piece_key[KNIGHT];

    int ckbn = td->corrhist_kbn[side][kbn & CORRHIST_MASK] * sp.corr_kbn_weight / sp.corrhist_weight;

    // king queen rook
    uint64_t kqr = game->st->piece_key[KING] ^ game->st->piece_key[QUEEN] ^ game->st->piece_key[ROOK];
    int ckqr = td->corrhist_kqr[side][kqr & CORRHIST_MASK] * sp.corr_kqr_weight/ sp.corrhist_weight;

    // continuation correction
    // this one's interesting. it is based on the same premise as continuation history, but applied to eval. basically, after a specific move that was responded to with another move, we are typically off by some amount, and we apply that here
    Move m = (stack-1)->current_move;
    int cr = 0;
    if (m){
        uint8_t to = move_to(m);
        cr = (*(stack-2)->cr)[game->piece_at[to]][to] * sp.corr_ch_weight / sp.corrhist_weight;
        
    }
    
    int c = (cp + cnp_w + cnp_b + ckqr + ckbn + cr) / sp.corrhist_grain;
    return c;
}






/* 
    @brief the giant eval function. uses the pawn hash to help out on the load. used to be in int space but now due to texel complication uses int s. i intend to go back, however. returns based on side to move, but everything is calculated from white as positive
*/

static inline int evaluate(Game * game, ThreadData * td, SearchStack * stack, Side side, SearchData * search_data, int depth, int alpha, int beta, bool * lazy, bool pv_node, bool debug){

    EvalDebug dbg;

    // make sure masks are init to zero since we or onto them
    EvalMasks masks;
    memset(&masks, 0, sizeof(EvalMasks));
    Score s = 0;
    int phase = game->phase;
    dbg = (EvalDebug){0};

    // search for pawn hash entry
    PawnHashEntry * entry = NULL;
    entry = search_for_pawn_hash_entry(game, game->st->piece_key[PAWN]);
    search_data->pawn_hash_probes += 1;

    if (entry){

        // entry retrieved, we will use the masks later (lazy things)
        search_data->pawn_hash_hits += 1;
        s += entry->s;
        
    } else {
        
        masks.passers[WHITE] = 0;
        masks.passers[BLACK] = 0;
        // get scores for black and white
        Score pw = evaluate_pawn_structure(game, WHITE, &masks); 
        Score pb = evaluate_pawn_structure(game, BLACK, &masks);

        // create a pawn hash entry with our masks and score
        create_new_pawn_hash_entry(game, game->st->piece_key[PAWN], pw - pb, &masks);
        s += pw - pb;
    }


    s += game->st->psqt_score[WHITE] - game->st->psqt_score[BLACK];
    s += game->st->material_score[WHITE] - game->st->material_score[BLACK];



    
    int corrhist = corrhist_eval(game, td, stack, side);
    
    // this logic is sort of expensive but necessary, we need to set up masks for eval before evaluating material because we need to know where each side attacks. it is actually possible to split this up a bit more and have another lazy stage (had it before), but would require refactor, retuning, and probably sacrificing some positional accuracy in evaluate_material

    // per piece attack masks indexed by piece list index (piece count when piece was set)
    uint64_t piece_attacks[COLOR_MAX][PIECE_TYPES][PIECE_MAX];
    if (entry){
        masks.am_p[WHITE][PAWN] = entry->w_atk;
        masks.am_p[BLACK][PAWN] = entry->b_atk;
        masks.passers[WHITE] = entry->w_passers;
        masks.passers[BLACK] = entry->b_passers;
        
    }
    generate_attack_mask_and_eval_mobility(game, WHITE, &masks, piece_attacks);
    generate_attack_mask_and_eval_mobility(game, BLACK, &masks, piece_attacks);


    

    // misc per piece eval
    Score mw = evaluate_material(game, WHITE, &masks, piece_attacks);
    Score mb = evaluate_material(game, BLACK, &masks, piece_attacks);

    // threats we have on enemy pieces
    Score tw = evaluate_threats(game, WHITE, &masks);
    Score tb = evaluate_threats(game, BLACK, &masks);

    // evaluates individual passers
    Score pw = evaluate_passers(game, WHITE, &masks);
    Score pb = evaluate_passers(game, BLACK, &masks);
    
    // evaluates the threat we have on the enemy king
    Score kw = evaluate_king_threat(game, WHITE, &masks, piece_attacks);
    Score kb = evaluate_king_threat(game, BLACK, &masks, piece_attacks);

    s += mw - mb;
    s += tw - tb;
    s += pw - pb;
    s += kw - kb;


    int mg = 0, eg = 0;
    mg = MG(s);
    eg = EG(s);
    int e = ((eg * (MAX_PHASE - phase)) + (mg * phase)) / MAX_PHASE;


    
    const int MAX_DEPTH = 32;
    if (side == WHITE){
        
        int ee = corrhist + e;
        return ee * sp.eval_scale;
    } else {
        int ee = corrhist - e;
        return ee * sp.eval_scale;
        
    }
}

// detects if a score is mate (used for pruning)
static inline bool is_mate(int score){
    return score < -MATE_SCORE + 500 || score > MATE_SCORE - 500;
}




#endif
