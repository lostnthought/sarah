#include "game.h"
#include "math.h"
#include "move_generation.h"
#include "types.h"
//#include "types_old.h"
#include "utils.h"
#include "magic.h"
#include "zobrist.h"
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include "sort.h"
#include "eval.h"
#include "search.h"
#include "./tables/tbconfig.h"
#include "./tables/tbprobe.h"

uint64_t rook_table[ROOK_TABLE_SIZE];
uint64_t bishop_table[BISHOP_TABLE_SIZE];

uint64_t material_randoms[COLOR_MAX][PIECE_TYPES][12];

int corrhist_p[COLOR_MAX][CORRHIST_SIZE];
int corrhist_nonpawns_w[COLOR_MAX][CORRHIST_SIZE];
int corrhist_nonpawns_b[COLOR_MAX][CORRHIST_SIZE];
int corrhist_kbn[COLOR_MAX][CORRHIST_SIZE];
int corrhist_kqr[COLOR_MAX][CORRHIST_SIZE];
TTBucket tt[TT_SIZE];
PawnHashEntry pawn_hash_table[PAWN_SIZE];
EvalEntry eval_table[EVAL_SIZE];
ThreadData td[MAX_THREADS];

int LMR_PV[64][64];
int LMR_NON_PV[64][64];
int LMP[64];
int FP[64][64];

SearchParams sp = {
    .qdelta_margin = 250,
    .qsee_margin = -100,
    .check_prune_margin = 60,
    .rfp_depth = 5,
    .rfp_base = 10,
    .rfp_mul = 50,
    .rfp_improving = 100,
    .lmr_cap_mul = 0.38,
    .lmr_cap_base = 0.27,
    .lmr_quiet_mul = 0.51,
    .lmr_quiet_base = 0.96,
    .lmp_depth = 5,
    .lmp_base = 8,
    .lmp_improving = 1,
    .lmp_depth_pow = 2,
    .futility_depth = 2,
    .futility_base = 200,
    .futility_mul = 220,
    .futility_hist_mul = 24,
    .futility_improving = 72,
    .razor_depth = 1,
    .razor_base = 203,
    .razor_improving = 100,
    .razor_mul = 136,
    .nmp_base = 46,
    .nmp_mul = 771,
    .nmp_slope = 206,
    .probcut_depth = 4,
    .probcut_improving = 156,
    .probcut_base = 323,
    .iid_depth = 4,
    .chist_depth = 2,
    .chist1_margin = -512,
    .chist2_margin = -512,
    .see_depth = 3,
    .mp_goodcap_margin = -55,
    .see_quiet_margin = -45,
    .see_nonquiet_margin = -83,
    .se_depth = 8,
    .se_depth_margin = 2,
    .qhistory_mul = 150,
    .qhistory_base = 125,
    .qhpen_mul = 150,
    .qhpen_base = 125,
    .chistory_mul = 150,
    .chistory_base = 125,
    .chpen_mul = 150,
    .chpen_base = 125,
    .beta_bonus = 50,
    .corr_depth_base = 1,
    .corrhist_grain = 256,
    .corrhist_weight = 256,
    .corrhist_max = 8192,
    .corr_ch_weight = 256,
    .corr_mat_weight = 256,
    .corr_np_weight = 256,
    .corr_pawn_weight = 256,
    .corr_kbn_weight = 256,
    .corr_kqr_weight = 256,
    .eval_scale = 1,
    
    
};


void init_search_params(){
    sp = (SearchParams){
        .eval_scale = (1.019307),
        .aspiration_base = round(63.118479),
        .aspiration_mul = round(2.986240),
        .qdelta_margin = round(287.355125),
        .qsee_margin = round(-67.062192),
        .rfp_depth = round(4.240283),
        .rfp_base = round(3.793856),
        .rfp_mul = round(36.525337),
        .rfp_improving = round(69.728985),
        .lmr_depth = round(2.525057),
        .lmr_move_start = round(3.167252),
        .lmr_hd = round(16759.866499),
        .lmr_quiet_mul = (0.613093),
        .lmr_quiet_base = (1.224905),
        .lmp_depth = round(3.908177),
        .lmp_base = round(8.140433),
        .lmp_improving = round(0.974697),
        .lmp_depth_pow = round(1.475621),
        .futility_depth = round(2.499304),
        .futility_base = round(209.091976),
        .futility_mul = round(209.003208),
        .futility_hist_mul = round(39.353611),
        .futility_improving = round(20.945189),
        .razor_depth = round(1.281307),
        .razor_base = round(248.283317),
        .razor_mul = round(112.272188),
        .razor_improving = round(132.419649),
        .nmp_base = round(10.846523),
        .nmp_mul = round(625.883057),
        .nmp_slope = round(319.607574),
        .probcut_depth = round(7.202014),
        .probcut_improving = round(73.065723),
        .probcut_base = round(128.960246),
        .iid_depth = round(5.651903),
        .chist_depth = round(3.200053),
        .chist1_margin = round(-895.366252),
        .chist2_margin = round(-915.104552),
        .mp_goodcap_margin = round(-107.621573),
        .chist1_scale = (1.033212),
        .chist2_scale = (3.422037),
        .chist4_scale = (2.577208),
        .chist6_scale = (3.394977),
        .see_depth = round(2.122899),
        .see_quiet_margin = round(-75.950593),
        .see_nonquiet_margin = round(-105.155207),
        .se_depth = round(6.929319),
        .se_depth_margin = (1.257939),
        .qhistory_mul = round(236.463088),
        .qhistory_base = round(155.487110),
        .qhpen_mul = round(163.899758),
        .qhpen_base = round(85.329360),
        .chistory_mul = round(136.236830),
        .chistory_base = round(223.462465),
        .chpen_mul = round(165.492487),
        .chpen_base = round(91.900729),
        .beta_bonus = round(50.227529),
        .corrhist_grain = round(135.298237),
        .corrhist_weight = round(260.425772),
        .corrhist_max = round(8217.611414),
        .corr_depth_base = round(1.111684),
        .corr_ch_weight = round(283.542309),
        .corr_np_weight = round(102.810894),
        .corr_pawn_weight = round(120.802045),
        .corr_kbn_weight = round(5.997313),
        .corr_kqr_weight = round(268.881605),
        // .eval_scale = (0.849076),
        // .aspiration_base = 80,
        // .aspiration_mul = 8,
        // .qdelta_margin = round(243.393743),
        // .qsee_margin = round(-91.101305),
        // .rfp_depth = round(3.710281),
        // .rfp_base = round(8.494243),
        // .rfp_mul = round(43.347654),
        // .rfp_improving = round(93.306239),
        // .lmr_depth = 3,
        // .lmr_move_start = 4,
        // .lmr_hd = 16834,
        // .lmr_quiet_mul = (0.652236),
        // .lmr_quiet_base = (0.873846),
        // .lmp_depth = round(3.183267),
        // .lmp_base = round(7.850076),
        // .lmp_improving = (0.883491),
        // .lmp_depth_pow = (1.434109),
        // .futility_depth = round(2.411216),
        // .futility_base = round(229.153387),
        // .futility_mul = round(214.360421),
        // .futility_hist_mul = round(60.343354),
        // .futility_improving = round(38.951833),
        // .razor_depth = round(2.163725),
        // .razor_base = round(181.764409),
        // .razor_mul = round(143.225551),
        // .razor_improving = round(61.971665),
        // .nmp_base = round(21.943308),
        // .nmp_mul = round(828.867797),
        // .nmp_slope = round(365.493636),
        // .probcut_depth = round(5.562112),
        // .probcut_improving = round(6.841408),
        // .probcut_base = round(172.568515),
        // .iid_depth = round(5.000000),
        // .chist_depth = round(1.837473),
        // .chist1_margin = round(-464.550515),
        // .chist2_margin = round(-233.883457),
        // .mp_goodcap_margin = round(-54.772720),
        // .chist1_scale = 1,
        // .chist2_scale = 1.5,
        // .chist4_scale = 2.2,
        // .chist6_scale = 3,
        // .see_depth = round(2.661332),
        // .see_quiet_margin = round(-54.769775),
        // .see_nonquiet_margin = round(-102.003431),
        // .se_depth = round(6.035922),
        // .se_depth_margin = (1.426201),
        // .qhistory_mul = round(192.837480),
        // .qhistory_base = round(124.039743),
        // .qhpen_mul = round(139.373680),
        // .qhpen_base = round(83.632596),
        // .chistory_mul = round(157.020238),
        // .chistory_base = round(193.782287),
        // .chpen_mul = round(144.779065),
        // .chpen_base = round(89.821511),
        // .beta_bonus = round(35.177930),
        // .corrhist_grain = round(218.374329),
        // .corrhist_weight = round(201.547721),
        // .corrhist_max = round(8164.545318),
        // .corr_depth_base = round(1.333025),
        // .corr_ch_weight = round(225.905064),
        // .corr_np_weight = round(176.767981),
        // .corr_pawn_weight = round(173.414003),
        // .corr_kbn_weight = round(122.453070),
        // .corr_kqr_weight = round(225.481897),

        
    };

}




// TODO for when weights have been finalized
void init_search_tables(){

    // LMR
    for (int i = 0; i < 64; i++){
        for (int m = 0; m < 64; m++){
            LMR_PV[i][m] = (log(i) * log(m)) * 0.32 + 0.1;
            LMR_NON_PV[i][m] = (log(i) * log(m)) * 0.51 + 0.9;
        }
    }

    // LMP
    
    for (int i = 0; i < 64; i++){
        LMP[i] = 8 + pow(i, 2);
    }

    // FUTILITY
    
    for (int i = 0; i < 64; i++){
        for (int m = 0; m < 64; m++){
            // FP[i][m] = 120 * (log((double)(i * i) / 2) / log(2.0) + 1) - m * 5 + 80;        
            // FP[i][m] = 175 - 50   
        }
    }

    
}

int quiesce(NodeType nt, Game * game, SearchStack * stack, int alpha, int beta, SearchData * search_data, int16_t depth, int16_t ply, int q_ply, ThreadData * td, bool checked){
    
    RootData * root = td->root;
    StateInfo st;
    const int QS_CHECK_DEPTH = -1;

    if (search_data->stop) {
        return 0;
    } else {
        bool stop = atomic_load(&root->stop);
        if (stop) {
            search_data->stop = true;
            return 0;
        }
    }

    search_data->pv_length[ply] = 0;
    bool pv_node = nt == PV;
    int old_alpha = alpha;

    Move tt_move = 0;
    assert(alpha < beta);
    assert(pv_node || alpha == beta - 1);
    search_data->tt_probes += 1;

    bool is_in_check = false;

    if (in_check(game, game->side_to_move)) is_in_check = true;
    int eval = -INT_MAX;
    int best_score = -INT_MAX;
    int futility_stand = -INT_MAX;
    int threats = 0;
    if (ply >= 63){
        return 0;
    }
    
    TTEntry * tt_entry = search_for_tt_entry(game, game->st->key);
    if (tt_entry){
        search_data->tt_hits += 1;
        tt_move = tt_entry->move;

        if (!pv_node){
    
            int adjusted_score = adjust_mate_score_from_tt(tt_entry->score, tt_entry->ply, ply);
            if (tt_entry->type == EXACT) return adjusted_score;
            if (tt_entry->type == UPPER && adjusted_score <= alpha) return adjusted_score;
            if (tt_entry->type == LOWER && adjusted_score >= beta) return adjusted_score;
        }
            

    }

    if (!is_in_check){
        EvalEntry * eval_entry = NULL;

        eval_entry = search_for_eval(game, game->st->key);
        if (eval_entry){
            if (eval_entry->side != game->side_to_move){
                eval= -eval_entry->eval;
            } else {
                eval = eval_entry->eval;
            }
        } else {
            bool lazy = false;
            eval = evaluate(game, td, stack, game->side_to_move, search_data, depth,alpha, beta, &lazy, pv_node, false);
            if (!lazy){
                hash_eval(game, game->st->key, eval, game->side_to_move);
    
            }
        }
        

        best_score = eval;
        
        if (best_score >= beta){
            // create_new_tt_entry(game, game->st->key, best_score, LOWER, depth, ply, 0);
            return best_score;
        } 

        if (best_score > alpha){
            alpha = best_score;
        }
        futility_stand = eval + sp.qdelta_margin;

    }

    Move new_best = 0;
    TTType hash_flag = UPPER;
    MovePicker mp;
    Undo undo;
    const PickType Q_STAGES[3] = {
        PICK_TT_MOVE,
        // bad cap is just captures
        PICK_BAD_CAP,
        PICK_QUIET,
    };
    int s_count = 0;
    mp.move_count = 0;
    mp.current_index = 0;
    mp.tt_move = tt_move;
    if (is_in_check){
        generate_moves(game, game->side_to_move, mp.moves, &mp.move_count);
    } else {
        generate_non_quiet_moves(game, game->side_to_move, mp.moves, &mp.move_count, depth < QS_CHECK_DEPTH ? false : true);
    }
    init_move_picker(&mp);
    mp.stage = Q_STAGES[0];
    int legal_moves = 0;
    bool exhausted = false;
    for (int i = 0; i < mp.move_count; i++){
        if (search_data->stop) {
            return 0;
        } else {
            bool stop = atomic_load(&root->stop);
            if (stop) {
                search_data->stop = true;
                return 0;
            }
        }
        bool is_tt_move = false;


        int16_t midx = -1;
        while (midx == -1){
            midx = pick_next_move(game, &mp, stack, td, ply);
            if (midx != -1) break;
            s_count += 1;
            if (s_count >= 3) {
                exhausted = true; break;
            }
            mp.stage = Q_STAGES[s_count];
        }
        if (exhausted || midx == -1) break;
        if (mp.stage == PICK_TT_MOVE) is_tt_move = true;
        Move m = mp.moves[midx];
        bool is_checking = move_causes_check(game, m);
        bool is_capture = is_cap(game, m);
        if (!is_capture && !is_checking && !is_in_check) continue;
        MoveType mt = move_type(m);

        // delta pruning
        if (   !pv_node
            && !is_in_check
            && !is_checking
            && !is_tt_move
            && mt != PROMOTION)
        {

            int futility = futility_stand + (mt == ENPASSANT ? PVAL[PAWN] : PSQT_EG[!game->side_to_move][move_cp(game, m)][move_to(m)]);

            if (futility <= alpha){
               search_data->qdelta_prunes += 1;
                continue;
            }
        }

        // see pruning
        if (   !pv_node
            && !is_in_check
            && !is_checking
            && mt != PROMOTION
            && !is_tt_move)
        {
            if (!see(game, m, sp.qsee_margin)){
                search_data->q_see_prunes += 1;
                continue;
            }
        }

        // // prune non-dangerous checks
        // if (   !pv_node
        //     && !is_in_check
        //     && is_checking
        //     && !is_tt_move
        //     && mt != PROMOTION
        //     && !is_capture
        //     && !check_is_dangerous(game, m, futility_stand, beta)
        //     && eval + sp.check_prune_margin < beta) {
        //         search_data->check_prunes += 1;
        //         continue;
        //     }

        PieceType p = move_piece(game, m);
        if(!make_move(game, m, &st, stack)){
            undo_move(game, m, stack);


        } else {
            search_data->qnodes += 1;
            legal_moves += 1;

            stack->ch = &td->continuation_history[p][move_to(m)];
            stack->cr = &td->contcorr[p][move_to(m)];

            int score = -quiesce(nt, game, stack + 1, -beta, -alpha, search_data, depth - 1, ply + 1, q_ply, td, is_checking);
            undo_move(game, m, stack);
            assert(score > -INT_MAX && score < INT_MAX);

            if (score > best_score){
                best_score = score;
                if (score > alpha){
                    if (score >= beta){
                        create_new_tt_entry(game, game->st->key, score, LOWER, depth, ply, m);
                        return score;
                    }
                    hash_flag = EXACT;
                    new_best = m;
                    store_pv(ply, new_best, search_data);
                    alpha = score;
                }
            }


        }
    }

    if (legal_moves == 0 && is_in_check){
        // if no legal quiet moves, we should check the rest to determine if we are in checkmate

        return -MATE_SCORE + ply;
        
    }
    
    assert(best_score != -INT_MAX && best_score != INT_MAX);
    create_new_tt_entry(game, game->st->key, best_score, hash_flag, depth, ply, new_best);
    return best_score;
    
}


int search(NodeType nt, Game * game, SearchStack * stack, int alpha, int  beta, int16_t depth, SearchData * search_data, int16_t ply, int extensions, bool skip_null_move, ThreadData * td, bool checked, bool cut_node){
    
    RootData * root = td->root;
    StateInfo st;

    if (search_data->stop) {
        return 0;
    } else {
        bool stop = atomic_load(&root->stop);
        if (stop) {
            search_data->stop = true;
            return 0;
        }
    }
    
    if (search_data->max_depth > 1){
        if (search_data->nodes_enabled && search_data->node_count > search_data->node_max){
            atomic_store(&root->stop, true);
            search_data->stop = true;
            return 0;
        } else {
    
            double elapsed_time = (double)(now_seconds() - search_data->start_time); 
            if (elapsed_time >= search_data->max_time) {
                atomic_store(&root->stop, true);
                search_data->stop = true;
                return 0;
            }
        }
        
    }
    int old_alpha = alpha;
    Move tt_move = 0;
    bool is_in_check = in_check(game, game->side_to_move);

    bool pv_node = nt == PV;
    int tt_depth = depth;
    TTEntry * tt_entry = NULL;
    search_data->pv_length[ply] = 0;

    assert(alpha < beta);
    assert(pv_node || alpha == beta - 1);

    // TODO detect other draws. i have the stuff already
    if (threefold(game->st) || game->st->rule50 >= 102){
        return 0;
    }

    // if 3-5 men, then you need 7-9 more for 12 angry men
    // we probe the tables here
    // int pc_count = __builtin_popcountll(game->board_pieces[BOTH]);
    // bool no_castling = game->castle_flags[0][0] == 0 && game->castle_flags[0][1] == 0 && game->castle_flags[1][0] == 0 && game->castle_flags[1][1] == 0;
    // int stm = 0;
    // if (game->side_to_move == WHITE) stm = 1;
    // if (pc_count <= TB_LARGEST && no_castling && game->en_passant_index == -1){
    //     int s = tb_probe_wdl(
    //         game->board_pieces[WHITE], 
    //         game->board_pieces[BLACK], 
    //         game->pieces[WHITE][KING] | game->pieces[BLACK][KING], 
    //         game->pieces[WHITE][QUEEN] | game->pieces[BLACK][QUEEN], 
    //         game->pieces[WHITE][ROOK] | game->pieces[BLACK][ROOK], 
    //         game->pieces[WHITE][BISHOP] | game->pieces[BLACK][BISHOP], 
    //         game->pieces[WHITE][KNIGHT] | game->pieces[BLACK][KNIGHT], 
    //         game->pieces[WHITE][PAWN] | game->pieces[BLACK][PAWN], 
    //         game->fifty_move_clock, 0, 0, stm);
    //     if (s){
    //         if (s == TB_WIN){
    //             return MATE_SCORE - ply;
    //         } else if (s == TB_LOSS){
    //             return -MATE_SCORE + ply;
    //         }
    //     }
    // }
    


    // qsearch
    if (depth <= 0){

        return quiesce(nt, game, stack, alpha, beta, search_data, 0, ply, ply, td, checked);
    }

    // probe tt
    search_data->tt_probes += 1;
    tt_entry = search_for_tt_entry(game, game->st->key);
    if (tt_entry){
        search_data->tt_hits += 1;
        tt_move = tt_entry->move;

        if (tt_entry->depth >= depth && !pv_node && stack->excluded_move == 0){

            int adjusted_score = adjust_mate_score_from_tt(tt_entry->score, tt_entry->ply, ply);
            if (tt_entry->type == EXACT) return adjusted_score;
            if (tt_entry->type == UPPER && adjusted_score <= alpha) return adjusted_score;
            if (tt_entry->type == LOWER && adjusted_score >= beta) return adjusted_score;
        }
    }
        
    // eval
    int stand= 0;
    int threats = 0;
    EvalEntry * eval_entry = NULL;

    // probe eval hash
    eval_entry = search_for_eval(game, game->st->key);
    if (eval_entry){
        if (eval_entry->side != game->side_to_move){
            stand = -eval_entry->eval;
        } else {
            stand = eval_entry->eval;
        }
    } else {
        bool lazy = false;
        stand = evaluate(game, td, stack, game->side_to_move, search_data, depth, alpha, beta, &lazy, pv_node, false);
        if (!lazy){
            hash_eval(game, game->st->key, stand, game->side_to_move);
        }
    }

    stack->eval = stand;
    // bail out
    if (ply >= 63){
        return stand;
    }


    // find if we are improving from 2 moves ago
    bool improving = false;
    if (!is_in_check){
        if (ply >= 2){
            if (stand > (stack-2)->eval) improving = true;
        }
    }

    // reverse futility pruning
    // the reverse of futility pruning. if our eval - margin * depth still clears beta, we can early out on the same premise as NMP.
    if (   !pv_node
        && depth <= sp.rfp_depth
        && !is_in_check
        && stack->excluded_move == 0
        && stand < 2000)
    {
        int eval = stand - (depth * sp.rfp_mul + sp.rfp_base + sp.rfp_improving * improving);
        if (eval >= beta) {
            search_data->rfp += 1;
            return eval;
        }
    }




    // razoring
    // the pre-moves loop version of futility pruning, which verifies using qsearch
    if (   !pv_node
        && depth <= sp.razor_depth
        && !is_in_check
        && stand + sp.razor_base + depth * sp.razor_mul + improving * sp.razor_improving < alpha
        && !is_mate(alpha)
        && abs(alpha) < 2000
        && stack->excluded_move == 0)
    {
        search_data->razoring += 1;
        int q = quiesce(NON_PV, game, stack, alpha, alpha+1, search_data, 0, ply, ply, td, checked);

        if (q < alpha){
            return q;
        }
    }
    

    int score = -INT_MAX;

    // null move pruning
    // if we don't make a move and we still clear beta, we can return lower bound, since that should never be the case (except for in endgame, which is why it's gated by phase)
    if (   depth >= 2
        && !is_in_check
        && game->phase > 4
        && !pv_node
        && !is_mate(beta)
        && !is_mate(alpha)
        && !is_mate(stand)
        && stand >= beta
        && stack->excluded_move == 0
        && !skip_null_move
        && (stack-1)->current_move != 0
        && (ply >= td->nmp_min_ply || game->side_to_move != td->nmp_side))
    {
        int r = (sp.nmp_mul * depth + sp.nmp_base) / sp.nmp_slope;
        r += MIN((float)(stand - beta) / 200, 3);
        r += improving;

        if (game->st->en_passant_index != -1){
            if (pawn_captures[!game->side_to_move][game->st->en_passant_index] & game->pieces[game->side_to_move][PAWN]) {

                game->st->key ^= get_en_passant_random(game->st->en_passant_index);
            }

        }
        game->side_to_move = !game->side_to_move;
        game->st->key ^= get_turn_random();
        int old_en_passant_index = game->st->en_passant_index;
        game->st->en_passant_index = -1;

        
        stack->ch = &td->continuation_history[PIECE_NONE][0];
        stack->cr = &td->contcorr[PIECE_NONE][0];
        int null_score = -INT_MAX;
        stack->current_move = 0;
        null_score = -search(NON_PV, game, stack+1, -beta, -beta + 1, depth-r, search_data, ply + 1, extensions, false, td, checked, !cut_node);
        
        game->side_to_move = !game->side_to_move;
        game->st->key ^= get_turn_random();
        game->st->en_passant_index = old_en_passant_index;
        

        if (game->st->en_passant_index != -1){
            if (pawn_captures[!game->side_to_move][game->st->en_passant_index] & game->pieces[game->side_to_move][PAWN]) {

                game->st->key ^= get_en_passant_random(game->st->en_passant_index);
            }

        }
        if (null_score >= beta){
            search_data->null_prunes += 1;
            if (null_score > MATE_SCORE - 200) null_score = beta;
            if (depth < 10 && abs(beta) < 2000) return null_score;

            // if depth >= 10, perform a verification search with a null window around beta. if still above beta, then we prune

            td->nmp_min_ply = ply + 3 * (depth-r) / 4;
            td->nmp_side = game->side_to_move;

            null_score = search(NON_PV, game, stack, beta-1, beta, depth-r, search_data, ply, extensions, false, td, false, false);

            td->nmp_min_ply = 0;
            
            if (null_score >= beta){
                return null_score;
            }

        }
    }
    
    MovePicker mp;
    mp.move_count = 0;
    mp.stage = 0;
    mp.tt_move = tt_move;
    generate_moves(game, game->side_to_move, mp.moves, &mp.move_count);
    init_move_picker(&mp);
    stack->move_count = mp.move_count;
    Undo undo;


    // probcut
    // create a beta + margin, check the first few moves with a null window around it to see if we are so good that we are most likely going to fail high anyway
    // we are *probably-a-cut* (probcut)
    if (   depth >= sp.probcut_depth
        && !pv_node
        && game->phase > 4
        && !is_mate(beta)
        && stack->excluded_move == 0)
    {
        
        int rbeta = beta + sp.probcut_base - sp.probcut_improving * improving;
        bool exhausted = false;
        
        int lm = 0;
        while (!exhausted || lm > 3 + 2 * cut_node || mp.stage >= PICK_QUIET){
            int16_t midx = pick_next_move(game, &mp, stack, td, ply);
            while (midx == -1){
                mp.stage += 1;
                if (mp.stage >= PICK_QUIET) {
                    exhausted = true;
                    break;
                }
                midx = pick_next_move(game, &mp, stack, td, ply);
            }
            if (exhausted || midx == -1) break;
            Move m = mp.moves[midx];
            bool is_checking = move_causes_check(game, m);


            PieceType p = move_piece(game, m);
            if (!make_move(game, m, &st, stack)){
                undo_move(game, m, stack);
            } else {
                lm++;
                stack->ch = &td->continuation_history[p][move_to(m)];
                stack->cr = &td->contcorr[p][move_to(m)];
                int q = -quiesce(NON_PV, game, stack+1, -rbeta, -rbeta+1, search_data, 0, ply+1, ply+1, td, is_checking);
                if (q >= rbeta){
                    
                    int v = -search(NON_PV, game, stack+1, -rbeta, -rbeta+1, depth - 3, search_data, ply+1, extensions, true, td, is_checking, !cut_node);
                    
                    if (v >= rbeta){
                        undo_move(game, m, stack);
                        return v;
                    }
                }
                undo_move(game, m, stack);
            }
            
        }
    }


    // internal iterative deepening
    if (depth >= sp.iid_depth && !tt_move) {

        search(nt, game, stack, alpha, beta, depth - (sp.iid_depth - 1), search_data, ply, extensions, true, td, checked, cut_node);

        TTEntry * new_entry = search_for_tt_entry(game, game->st->key);
        if (new_entry){
            if (new_entry->move){
                search_data->successful_iids += 1;
                tt_move = new_entry->move;
            }
        }
    }




    // cache pawn hash for passed pawn ext

    const ContinuationHistory * chist[4] = {(stack-1)->ch, (stack-2)->ch, (stack-4)->ch, (stack-6)->ch};
    
    init_move_picker(&mp);
    mp.stage = PICK_TT_MOVE;
    mp.current_index = 0;
    TTType hash_type = UPPER;
    Move new_best = 0;
    Move new_tt = 0;
    int legal_moves = 0;
    int searched_size = 0;
    bool exhausted = false;

    Move nonquiets[256];
    uint8_t nq_cnt = 0;
    Move quiets[256];
    uint8_t q_cnt = 0;

    while (!exhausted){
        int16_t midx = -1;
        while (midx == -1){
            midx = pick_next_move(game, &mp, stack, td, ply);
            if (midx != -1) break;
            
            mp.stage += 1;
            if (mp.stage > PICK_BAD_CAP) {
                exhausted = true;
                break;
            }
        }

        if (exhausted || midx == -1) {
            break;
        }

        Move m = mp.moves[midx];
        if (stack->excluded_move != 0){
            if (m == stack->excluded_move){
                continue;
            }
        }
        searched_size++;

        int extension = 0;


        int16_t mscore = mp.scores[midx];
        PieceType piece = move_piece(game, m);
        MoveType mt = move_type(m);
        bool is_capture = is_cap(game, m);
        bool cap_or_promo = is_capture || mt == PROMOTION;
        bool is_checking = move_causes_check(game, m);
        


        // LMR
        // extensions
        int e = 0;
        if (is_checking){ // extend on pv node checks or see positive checks
            if (pv_node){
                e = 1;
            } else {
                if (see(game, m, 0)){
                    e = 1;
                }
            }
        }
        int reduction = 1;

        // calculate depth after extension
        int new_depth = depth + e;
        search_data->extensions += e;
        bool dangerous = is_in_check || is_checking || mt == CASTLE || mt == PROMOTION;
        // move is legal

        if (depth >= sp.lmr_depth && legal_moves >= sp.lmr_move_start + pv_node && !cap_or_promo){

            float r = 0;
            r = (log(depth) * log(legal_moves)) * sp.lmr_quiet_mul + sp.lmr_quiet_base;
        
            // scale based on history, yes this also goes negative
            r -= (float)mscore / sp.lmr_hd;
            r -= pv_node * 2;
            r -= is_checking || is_in_check;
            r += cut_node * 2;
            
            // -2 because reduction is initialized to 1
            r = MAX(MIN(r, new_depth - 2), 0);
            reduction += (int)(r + 0.5f);

            search_data->reductions += reduction;
        }

        int pred_depth = MAX(new_depth - reduction, 1);
        
        // Move Pruning
        if (mt != PROMOTION
            && legal_moves > 1
            && !is_in_check
            && !is_mate(score)
            && !pv_node
            && !is_checking){

            int lmp = (sp.lmp_base + pow(depth, sp.lmp_depth_pow)) * (improving + sp.lmp_improving);
            if (depth <= sp.lmp_depth && legal_moves > lmp && !is_capture){
                search_data->late_move_prunes += 1;
                continue;
            }

            if (pred_depth <= sp.futility_depth && !is_capture){
        
                 if (stand + sp.futility_base +
                     sp.futility_mul * pred_depth +
                     ((float)mp.scores[midx] / 16834) * sp.futility_hist_mul + improving * sp.futility_improving < alpha){
                     search_data->futility_prunes += 1;
                     continue;
                 }
            }

            if (pred_depth <= sp.chist_depth && (*chist[0])[piece][move_to(m)] < sp.chist1_margin && (*chist[1])[piece][move_to(m)] < sp.chist2_margin && !is_capture){
                search_data->chist_prunes += 1;
                continue;
            }

            // see pruning
            // discards moves if they lose statically, margin scales with depth
            if (depth <= sp.see_depth){

                if (!is_capture){
                    if (!see(game, m, depth * sp.see_quiet_margin)){
                        search_data->see_prunes += 1;
                        continue;
                    }
                } else {
    
                    if (!see(game, m, depth * sp.see_nonquiet_margin)){
                        search_data->see_prunes += 1;
                        continue;
                    }
                }
            }
    
        }
        // singular extensions
        // check if the tt move is much better than all other moves by excluding it from the search
        if (   depth >= sp.se_depth
            && tt_entry
            && tt_move
            && m == tt_move
            && stack[ply].excluded_move == 0
            && e == 0)
        {
            
            if (tt_entry->type == LOWER
                && !is_mate(tt_entry->score)
                && tt_entry->depth >= depth - 3)
            {
                int singular_beta = MAX((tt_entry->score - depth * sp.se_depth_margin), -MATE_SCORE + 1);
                int singular_depth = depth / 2;

                stack[ply].excluded_move = m;
                int s = search(NON_PV, game, stack, singular_beta - 1, singular_beta, singular_depth, search_data, ply, extensions, true, td, checked, cut_node);
                stack[ply].excluded_move = 0;

                if (s < singular_beta){
                    extension += 1;
                }

                // if the score was above beta, and the tt score was so good that our margin'd beta is still above beta, we can be sure that the whole node is failing high.
                // (or not since this has only lost me elo)

                if (s >= singular_beta && singular_beta >= beta){
                    return singular_beta;
                }
            }
            new_depth += extension;
        }

        if (!make_move(game, m, &st, stack)){
            undo_move(game, m, stack);
        } else {
            legal_moves += 1;
            search_data->node_count += 1;

            stack->ch = &td->continuation_history[piece][move_to(m)];
            stack->cr = &td->contcorr[piece][move_to(m)];

            int current_score = -INT_MAX;

            bool lmr_research = false;
                
            // lmr search
            if (reduction > 1){
                current_score = -search(NON_PV, game, stack+1, -(alpha+1), -alpha, new_depth - reduction, search_data, ply + 1, extensions + extension, false, td, is_checking, true);
                if (current_score > alpha) lmr_research = true;
            }

            // full depth search (for lmr research, non pv nodes without lmr, or pv nodes past move one)
            if ((reduction == 1 && (!pv_node || legal_moves > 1)) || lmr_research){

                current_score = -search(NON_PV, game, stack+1, -(alpha+1), -alpha , new_depth - 1, search_data, ply + 1, extensions + extension, false, td, is_checking, !cut_node);
            }

            // pv search / research
            if (pv_node && (legal_moves == 1 || (current_score > alpha))){
                current_score = -search(PV, game, stack+1, -beta, -alpha , new_depth - 1, search_data, ply + 1, extensions + extension, false, td, is_checking, false);
            }

                
            // if (current_score == INT_MAX || current_score == -INT_MAX){
            //     printf("CURRENT SCORE INT MAX\n");
            //     fflush(stdout);
            // }
            // assert(current_score != -INT_MAX && current_score != INT_MAX);
            undo_move(game, m, stack);


            bool raised = false;
            if (current_score > score){
                score = current_score;
                new_best = m;
                if (current_score > alpha){
                    raised = true;
                    new_tt = m;
                    if (current_score >= beta){
                        if (legal_moves == 1) search_data->ordering_success += 1;
        
                        if (!is_capture){
    
                            stack->killers[1] = stack->killers[0];
                            stack->killers[0] = new_tt;

                        }
                        break;
                    }

                    // otherwise we raise alpha
                    if (legal_moves == 1) search_data->ordering_success += 1;
                    hash_type = EXACT;
                    store_pv(ply, new_best, search_data);
                    alpha = score;
                }
            
            }
            
            if (!raised){
                if (cap_or_promo){
                    nonquiets[nq_cnt++] = m;
                } else {
                    quiets[q_cnt++] = m;
                }
                
            }
        }
    }

    if (legal_moves == 0 && stack->excluded_move != 0){
        return alpha;
    }
    if (legal_moves == 0 && is_in_check){

        return -MATE_SCORE + ply;
    
    } else if (legal_moves == 0){
        // may be stalemate, either way no moves were processed
        return 0;
    } else if (stack->excluded_move == 0){
        
        if (score >= beta) {
            hash_type = LOWER;
        } else if (pv_node && new_tt) {
            hash_type = EXACT;
        } else {
            hash_type = UPPER;
        }

        create_new_tt_entry(game, game->st->key, score, hash_type, depth, ply, new_tt);
            
    }
    assert(score > -INT_MAX  && score < INT_MAX);
    if (new_tt){

        MoveType mt = move_type(new_tt);
        bool cap_or_promo = is_cap(game, new_tt) || mt == PROMOTION;
        int cbonus = sp.chistory_mul * depth + sp.chistory_base;
        int chpen = sp.chpen_mul * depth + sp.chpen_base;
        
        if (!cap_or_promo){
            int qbonus = sp.qhistory_mul * depth + sp.qhistory_base + (score >= beta ? sp.beta_bonus : 0);
            int qpen = sp.qhpen_mul * depth + sp.qhpen_base + (score >= beta ? sp.beta_bonus : 0);

            for (int l = 0; l < q_cnt; l++){

                update_history(game, td, stack, ply, quiets[l], -qpen);
            }
            update_history(game, td, stack, ply, new_tt, qbonus);
            
        } else {
            
            PieceType cp = mt == ENPASSANT ? PAWN : move_cp(game, new_tt);
            update_cap_hist(td, move_piece(game, new_tt), move_to(new_tt), cp, cbonus);

        }

        for (int l = 0; l < nq_cnt; l++){
            Move cm = nonquiets[l];
            PieceType cp = move_type(cm) == ENPASSANT ? PAWN : move_cp(game, cm);
            update_cap_hist(td, move_piece(game, cm), move_to(cm), cp, -chpen);
        }


        // if score > beta, score must be > static eval
        // this must cause some edge case where we are failing high but our static eval was already so incredibly good that the value is somehow underneath it, not quite sure
        // if we didn't raise alpha or fail high, the score must be less than the static eval. what i think this signifies is that we need to be moving in the direction of the bound. if we do not raise alpha, we have to be moving downward, if we fail high, we need to be moving upward. it makes some sense to remove other entries from corrhist since they might give incorrect indications for score bias.
        if (!is_in_check
        && !cap_or_promo
        && (hash_type == EXACT
        || (score >= beta && score >= stand)
        || score <= alpha && score <= stand)){
            update_corrhist(game, td, stack, game->side_to_move, depth, score - stand);
        }

    }


    
    return score;
    
}



void add_search_data_to_root(ThreadData * td){
    
    SearchData * search_data = &td->search_data;
    RootData * root = td->root;
    pthread_mutex_lock(&root->sd_lock);
    root->sd.aspiration_fail += td->search_data.aspiration_fail;
    root->sd.tt_hits+= td->search_data.tt_hits;
    root->sd.tt_probes+= td->search_data.tt_probes;
    root->sd.aspiration_fail+= td->search_data.aspiration_fail;
    root->sd.lmrs_tried+= td->search_data.lmrs_tried;
    root->sd.lmrs_researched+= td->search_data.lmrs_researched;
    root->sd.null_prunes+= td->search_data.null_prunes;
    root->sd.qnodes+= td->search_data.qnodes;
    root->sd.ordering_success+= td->search_data.ordering_success;
    root->sd.futility_prunes+= td->search_data.futility_prunes;
    root->sd.see_prunes+= td->search_data.see_prunes;
    root->sd.beta_prunes+= td->search_data.beta_prunes;
    root->sd.pawn_hash_probes+= td->search_data.pawn_hash_probes;
    root->sd.pawn_hash_hits+= td->search_data.pawn_hash_hits;
    root->sd.late_move_prunes+= td->search_data.late_move_prunes;
    root->sd.q_see_prunes+= td->search_data.q_see_prunes;
    root->sd.rfp+= td->search_data.rfp;
    root->sd.razoring+= td->search_data.razoring;
    root->sd.check_extensions+= td->search_data.check_extensions;
    root->sd.delta_prunes+= td->search_data.delta_prunes;
    root->sd.qdelta_prunes+= td->search_data.qdelta_prunes;
    root->sd.lazy_cutoffs_s1+= td->search_data.lazy_cutoffs_s1;
    root->sd.lazy_cutoffs_s2+= td->search_data.lazy_cutoffs_s2;
    root->sd.lazy_cutoffs_s3+= td->search_data.lazy_cutoffs_s3;
    root->sd.highest_mat_reached+= td->search_data.highest_mat_reached;
    root->sd.fast_evals += td->search_data.fast_evals;
    root->sd.node_count += td->search_data.node_count;
    root->sd.extensions += td->search_data.extensions;
    root->sd.reductions += td->search_data.reductions;
    root->sd.check_prunes+= td->search_data.check_prunes;
    root->sd.chist_prunes+= td->search_data.chist_prunes;
    if (td->search_data.flags.three_fold_repetition) root->sd.flags.three_fold_repetition = true;

    pthread_mutex_unlock(&root->sd_lock);
}

void * search_thread(void * thread_data){

    ThreadData * td = (ThreadData * )thread_data;
    SearchData * search_data = &td->search_data;
    RootData * root = td->root;
    bool pv_node = true;
    td->game.gen += 1;
    
    // if we have a last state, set our state to the linked lists's last state
    if (td->last_state >= 1) {
        td->game.st = &td->state_stack[td->last_state - 1];
    }
    // draw detection using that new state link
    if (threefold(td->game.st) || td->game.st->rule50 >= 102){
        search_data->flags.three_fold_repetition = true;
        atomic_store(&root->game_end, true);
        atomic_store(&root->stop, true);
    }
    if (search_data->max_depth > 1){
        if (search_data->stop) {
            return 0;
        } else {
            bool stop = atomic_load(&root->stop);
            if (stop) {
                search_data->stop = true;
                return 0;
            }
        }
        
    }
    
    int stand= 0;
    int threats = 0;
    EvalEntry * eval_entry = NULL;

    // probe eval hash
    bool lazy= false;

    // initialized conthist and contcorr pointers to essentially zero for a few plies in the negative so that we aren't indexing negative when we look at plies less than 0
    SearchStack stack[MAX_PLY + 10];
    memset(stack, 0, sizeof(stack));
    for (int i = 0; i <= 7; i++){
        stack[i].ch = &td->continuation_history[PIECE_NONE][0];
        stack[i].cr = &td->contcorr[PIECE_NONE][0];
    }
    // cache eval for improving
    int eval = evaluate(&td->game, td, stack+7, td->game.side_to_move, search_data, search_data->max_depth, root->alpha, root->beta, &lazy, pv_node, false);
    stack[7].eval = eval;
    search_data->pv_length[0] = 0;
    TTEntry * entry = NULL;
    // set stack pointer to a safe value
    SearchStack * s = stack+8;

    int score = -INT_MAX;
    Move best_move = 0;
    search_data->ply = 0;
    
    int legal_moves = 0;
    bool is_in_check = in_check(&td->game, td->game.side_to_move);

    while (1){
        // get our next index
        int c = atomic_fetch_add(&root->next_index, 1);
        if (c >= root->move_count) break;
        Move move = root->move_list[c];
        Game game;
        // fresh game instance
        memcpy(&game, &td->game, sizeof(Game));
        // link the game st to the thread stack
        if (td->last_state >= 1) {
            game.st = &td->state_stack[td->last_state - 1];
        }
        StateInfo st;
        bool is_checking = move_causes_check(&game, move);
        if (make_move(&game, move, &st, s-1)){
            legal_moves = atomic_fetch_add(&root->legal_moves, 1);
        
            int extension = 0;
            bool extend = false;
            int reduction = 0;
            int current_score = 0;

            // set new cont pointers for the current move.
            (s-1)->ch = &td->continuation_history[move_piece(&game, move)][move_to(move)];
            (s-1)->cr = &td->contcorr[move_piece(&game, move)][move_to(move)];

            // pass down s (we are s-1)
            current_score = -search(PV, &game, s, -root->beta, -root->alpha, search_data->max_depth, search_data, 1, 0, false, td, is_checking, false);
            int old = atomic_load(&root->best_score);

            // if our score beats the root score, store the pv and copy it into the root mutex
            if (current_score > old){
                if (atomic_compare_exchange_strong(&root->best_score, &old, current_score)) {
                    store_pv(0, move, search_data);
                    root->best_move = move;
                    atomic_store(&root->best_score, current_score);

                    pthread_mutex_lock(&root->pv_lock);
                    root->pv_length = td->search_data.pv_length[0];
                    memcpy(root->pv, td->search_data.pv_table[0],
                           root->pv_length * sizeof(Move));
                    pthread_mutex_unlock(&root->pv_lock);
                }
            }

        }
        undo_move(&game, move, &stack[7]);
    
        if (search_data->max_depth > 1){
            if (search_data->stop) {
                break;
            } else {
                bool stop = atomic_load(&root->stop);
                if (stop) {
                    search_data->stop = true;
                    break;
                }
            }
            
        }

    }
    legal_moves = atomic_load(&root->legal_moves);
    add_search_data_to_root(td);
    
    return NULL;
}

// resets the root data in between searches
void reset_root(RootData * data){
    atomic_store(&data->next_index, 0);
    atomic_store(&data->best_score, -INT_MAX);
    atomic_store(&data->legal_moves, 0);
    atomic_store(&data->game_end, 0);
    memset(data->pv, 0, sizeof(data->pv));
    data->pv_length = 0;
    data->alpha = 0;
    data->beta = 0;
}

// on init / new game, we simply just set all corrhist and conthist to 0
void init_threads(){
    for (int i = 0; i < MAX_THREADS; i++){
        ThreadData * t = &td[i];
        memset(t->continuation_history, 0, sizeof(ContinuationHistory) * (PIECE_TYPES + 1) * BOARD_MAX);
        memset(t->contcorr, 0, sizeof(ContinuationHistory) * (PIECE_TYPES + 1) * BOARD_MAX);
        memset(t->history, 0, sizeof(int16_t) * COLOR_MAX * BOARD_MAX * BOARD_MAX);
        memset(t->cap_hist, 0, sizeof(int16_t) * PIECE_TYPES * BOARD_MAX * (PIECE_TYPES + 1));
        memset(t->corrhist_p, 0, sizeof(int16_t) * COLOR_MAX * CORRHIST_SIZE);
        memset(t->corrhist_nonpawns_w, 0, sizeof(int16_t) * COLOR_MAX * CORRHIST_SIZE);
        memset(t->corrhist_nonpawns_b, 0, sizeof(int16_t) * COLOR_MAX * CORRHIST_SIZE);
        memset(t->corrhist_kbn, 0, sizeof(int16_t) * COLOR_MAX * CORRHIST_SIZE);
        memset(t->corrhist_kqr, 0, sizeof(int16_t) * COLOR_MAX * CORRHIST_SIZE);
        memset(t->corrhist_material, 0, sizeof(int16_t) * COLOR_MAX * CORRHIST_SIZE);
        t->id = i;
        t->root = NULL;
        t->nmp_side = 0;
        t->nmp_min_ply = 0;
    }
}


Move iterative_search(Game * game, SearchFlags * flags){

    int MAX_DEPTH = 50;
    Move current_best;

    double start_time = now_seconds();
    double max_time = flags->max_time;

    // thread count for now is simply hardcoded, TODO i need to write a way to set it in a command
    int thread_count = MAX(MIN(flags->threads, 8), 1);

    int best_depth = 1;

    double elapsed_time = 0; 

    int best_move_score = -INT_MAX;

    int delta = 70;
    int alpha = -INT_MAX;
    int beta = INT_MAX;
    int A = -INT_MAX, B = INT_MAX;

    RootData root;
    root.move_count = 0;
    atomic_store(&root.next_index, 0);
    atomic_store(&root.best_score, -INT_MAX);
    atomic_store(&root.game_end, false);
    pthread_mutex_init(&root.pv_lock, NULL);
    pthread_mutex_init(&root.sd_lock, NULL);
    atomic_store(&root.stop, false);

    bool game_end = false;

    // generate moves into the root's data
    generate_moves(game, game->side_to_move, root.move_list, &root.move_count);

    // memcpy the state stack from the global stack. we use the global stack when we are playing moves given to us from the uci, but we need each thread to have a separate copy of the linked list so that we don't have race condition issues. learned that the hard way
    for (int t = 0; t < thread_count; t++){
        memcpy(td[t].state_stack, state_stack, sizeof(state_stack));
        td[t].last_state = st_idx;
        td[t].state_stack[0].pst = NULL;
        printf("IDX: %d\n", st_idx);
        for (int i = 1; i < td[t].last_state; i++){
            td[t].state_stack[i].pst = &td[t].state_stack[i-1];
        }
    }


    
    // if we have opening book turned on, I simply uncomment this out, TODO need to make this a command

    // if (game->history_count < 12){
    //     TTEntry * tt_entry = search_for_tt_entry(game, game->st->key);
    //     if (tt_entry){
            // if (tt_entry->is_opening_book){
    //             Move m; 
    //             // unpack_move(tt_entry->move32, &m);

    //             m = tt_entry->move;

    //             // print_move_full(&m);
    //             fflush(stdout);
    //             bool promo = move_type(m)== PROMOTION;
    //             Move resolved_move = find_move(root.move_list, root.move_count, move_from(m), move_to(m), promo, move_promotion_type(m));

    //             if (resolved_move){
    //                 // fflush(stdout);
    //                 printf("bestmove ");
    //                 print_move_algebraic(game->side_to_move, resolved_move);
    //                 printf("\n");
    //                 fflush(stdout);
    //                return resolved_move; 
    //             }
    //         }
    //     }
        
    // }

    SearchData * search_data = &root.sd;
    
    // this is for syzygy but is disabled for tuning / testing currently
    bool syzygy_found = false;
    int pc_count = __builtin_popcountll(game->board_pieces[BOTH]);
    // bool no_castling = game->castle_flags[0][0] == 0 && game->castle_flags[0][1] == 0 && game->castle_flags[1][0] == 0 && game->castle_flags[1][1] == 0;
    // if (pc_count <= TB_LARGEST && no_castling && game->en_passant_index == -1){


     // unsigned results[TB_MAX_MOVES];
    //     unsigned s = tb_probe_root(
    //         game->board_pieces[WHITE], 
    //         game->board_pieces[BLACK], 
    //         game->pieces[WHITE][KING] | game->pieces[BLACK][KING], 
    //         game->pieces[WHITE][QUEEN] | game->pieces[BLACK][QUEEN], 
    //         game->pieces[WHITE][ROOK] | game->pieces[BLACK][ROOK], 
    //         game->pieces[WHITE][BISHOP] | game->pieces[BLACK][BISHOP], 
    //         game->pieces[WHITE][KNIGHT] | game->pieces[BLACK][KNIGHT], 
    //         game->pieces[WHITE][PAWN] | game->pieces[BLACK][PAWN], 
    //         game->fifty_move_clock, 0, 0, game->side_to_move, results);

    //     int best = -1;
    //     for (int i = 0; i < root.move_count; i++){
    //      unsigned f = TB_GET_FROM(results[i]);
    //      unsigned t = TB_GET_TO(results[i]);
    //      unsigned p = TB_GET_PROMOTES(results[i]);

    //         Move * m = find_move(root.move_list, root.move_count, f, t, p, p);
    //         if (m){
    //             unsigned res = TB_GET_WDL(results[i]);
    //             unsigned z = TB_GET_DTZ(results[i]);
    //             if (res == TB_WIN){
    //                 m->score = 1000000 - z;
                    
    //             } else if (res == TB_LOSS){
    //                 m->score = -1000000 + z;
    //             }
    //         } else {
    //         }
    //     }
    //     qsort(root.move_list, root.move_count, sizeof(Move), compare_moves);
    //     syzygy_found = true;
    // }

    // the main search loop, generation is incremented here before the game is copied to age the tt
    int current_score = 0;
    for (int i = 1; i < MAX_DEPTH; i++){
        game->gen += 1;
        
        init_search_data(search_data, flags, start_time, max_time, i);

        atomic_store(&root.node_count, 0);

        // aspiration setup happens outside of the thread split
        if (i > 5){
            delta = sp.aspiration_base + i * sp.aspiration_mul;
            alpha = current_score - delta;
            beta = current_score + delta;
        }

        reset_root(&root);
        root.alpha = alpha;
        root.beta = beta;

        pthread_t threads[thread_count];

        for (int t = 0; t < thread_count; t++){
            memcpy(&td[t].game, game, sizeof(Game));
            init_search_data(&td[t].search_data, flags, start_time, max_time, i);
            td[t].root = &root;
            if (td[t].id == 0) {
                td[t].search_data.enable_time = true;
            }
            pthread_create(&threads[t], NULL, search_thread, &td[t]);
        }


        
        for (int i = 0; i < thread_count; i++) {
            pthread_join(threads[i], NULL);
        }


        if (i == 1){
            int legal_moves = atomic_load(&root.legal_moves);
            if (legal_moves == 0 && in_check(game, game->side_to_move)){
                game_end = true;
                break;
            } else if (legal_moves == 0){
                game_end = true;
                break;
            }
            bool ge = atomic_load(&root.game_end);
            if (ge){
                game_end = true;
                break;
            }
            
        }
        current_score = atomic_load(&root.best_score);
        bool stop = atomic_load(&root.stop);
        game_end = atomic_load(&root.game_end);
        double end_time = now_seconds();
        double elapsed_time = (double)(end_time - start_time); 
        // we only exit out after a full iteration at depth greater than 1
        if (i > 1){
            if (elapsed_time >= max_time || stop || game_end) {
                break;
            }
        }
        if (i > 5){
            int new_best = atomic_load(&root.best_score);
            bool fail = false;
            if (new_best <= alpha){
                alpha = A;
                fail = true;
            } else if (new_best >= beta){
                beta = B;
                fail = true;
            }
            if (fail){
                
                reset_root(&root);
                root.alpha = alpha;
                root.beta = beta;

                pthread_t threads[thread_count];
                ThreadData td[thread_count];

                for (int t = 0; t < thread_count; t++){
                    memcpy(&td[t].game, game, sizeof(Game));
                    init_search_data(&td[t].search_data, flags, start_time, max_time, i);
                    td[t].root = &root;
                    if (td[t].id == 0) {
                        td[t].search_data.enable_time = true;
                    }
                    pthread_create(&threads[t], NULL, search_thread, &td[t]);
                }
                for (int i = 0; i < thread_count; i++) {
                    pthread_join(threads[i], NULL);
                }
            }
            current_score = atomic_load(&root.best_score);
        }

        // game_end = atomic_load(&root.game_end);
        // if (!syzygy_found){
        //     qsort(move_list, move_count, sizeof(Move), compare_moves);
        // }

        // if we ran out of time, exit and discard that move
        stop = atomic_load(&root.stop);
        end_time = now_seconds();
        elapsed_time = (double)(end_time - start_time); 
        if (i > 1){
            if (elapsed_time >= max_time || stop || game_end) {
                break;
            }
        }
        best_depth = i;
        // best_move_score = current_score;

        // debug info!
        printf("Depth %d nodes %d qnodes %d reductions %.1f extensions %.1f tt hits %d tt probes %d asp fails %d fast %d lazy 1 %d 2 %d 3 %d lmr tried %d lmr research %d nmp %d ordering success %d futility prunes %d lmp %d cprunes %d rfp %d razor %d check ext %d delta %d qdelta %d see prunes %d q see prunes %d ch prunes %d pawn hash probes %d hits %d nps %f\n",
        search_data->max_depth,
        search_data->node_count,
        search_data->qnodes,
        search_data->reductions,
        search_data->extensions,
        search_data->tt_hits,
        search_data->tt_probes,
        search_data->aspiration_fail,
        search_data->fast_evals,
        search_data->lazy_cutoffs_s1,
        search_data->lazy_cutoffs_s2,
        search_data->lazy_cutoffs_s3,
        search_data->lmrs_tried,
        search_data->lmrs_researched,
        search_data->null_prunes,
        search_data->ordering_success,
        search_data->futility_prunes,
        search_data->late_move_prunes,
        search_data->chist_prunes,
        search_data->rfp,
        search_data->razoring,
        search_data->check_extensions,
        search_data->delta_prunes,
        search_data->qdelta_prunes,
        search_data->see_prunes,
        search_data->q_see_prunes,
        search_data->check_prunes,
        search_data->pawn_hash_probes,
        search_data->pawn_hash_hits,
        (search_data->node_count + search_data->qnodes) / elapsed_time);


        // uci printouts
        if (!game_end){
            current_best = root.best_move;
            best_move_score = atomic_load(&root.best_score);
            char dbg[100000];
            pthread_mutex_lock(&root.sd_lock);
            printf("info depth %d score cp %d nodes %d pv ", i, best_move_score, root.sd.node_count);
            // sprintf(dbg, "depth %d score cp %d nodes %d pv ", i, best_move_score, root.sd.node_count);
            pthread_mutex_unlock(&root.sd_lock);
            pthread_mutex_lock(&root.pv_lock);
            for (int i = 0; i < root.pv_length; i++) {
                // char mv[24];
                // get_move_algebraic(game->side_to_move, root.pv[i], mv);
                // int len = strlen(mv);
                // mv[len] = ' ';
                // mv[len + 1] = '\0';
                // strcat(dbg, mv);
                print_move_algebraic(game->side_to_move, root.pv[i]);
                printf(" ");
            }
            pthread_mutex_unlock(&root.pv_lock);
            printf("\n");
            fflush(stdout);
            printf("%s\n", dbg);
            
        }

    }

    // if game hasn't ended, print out our move
    if (!game_end){
        print_game_board(game);
        fflush(stdout);
        printf("bestmove ");
        print_move_algebraic(game->side_to_move, current_best);
        printf("\n");
        fflush(stdout);
        
    } else {
        // otherwise, check if it's a draw or a mate
        if (flags){
            pthread_mutex_lock(&root.sd_lock);

            if (root.sd.flags.three_fold_repetition || root.sd.flags.draw || (atomic_load(&root.legal_moves) == 0 && !in_check(game, game->side_to_move)) ){
                flags->draw = true;
            } else {
                
                flags->mate = true;
            }
            pthread_mutex_unlock(&root.sd_lock);
            
        }
    }
    
    return current_best;
    
}


uint64_t perft(Game * game, SearchStack * stack, int depth){

    if (depth == 0) {
        return 1ULL;
    }
    
    Move move_list[256];
    uint8_t move_count = 0;
    generate_moves(game, game->side_to_move, move_list, &move_count);
    uint64_t nodes = 0;
    StateInfo st;
    for (int i = 0; i < move_count; i++){
        
        Move move = move_list[i];

        // if illegal move
        if (!make_move(game, move, &st, stack)){

            undo_move(game, move, stack);

        } else {
            uint64_t move_nodes = perft(game, stack+1, depth - 1);
            nodes += move_nodes;

            undo_move(game, move, stack);
        }
    }
    return nodes;
}

void perft_root(Game * game, int depth){

    clock_t start_time = clock();
    Move move_list[256];
    uint8_t move_count = 0;
    generate_moves(game, game->side_to_move, move_list, &move_count);
    uint64_t nodes = 0;
    SearchStack stack[MAX_PLY + 10];
    SearchStack * s = &stack[0];
    memset(stack, 0, sizeof(stack));

    for (int i = 0; i < move_count; i++){
        
        Move move = move_list[i];

        StateInfo new_st;
        // if illegal move
        if (!make_move(game, move, &new_st, s)){

            undo_move(game, move, s);

        } else {

            uint64_t move_nodes = perft(game, s+1, depth - 1);
            print_move_algebraic(game->side_to_move, move);
            printf(" Nodes: %ld\n", move_nodes);
            nodes += move_nodes;

            undo_move(game, move, s);
        }
    }

    clock_t end_time = clock();
    
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC; 
    
    printf("Depth %d Nodes: %ld and NPS: %f\n", depth, nodes, nodes / elapsed_time);
}
