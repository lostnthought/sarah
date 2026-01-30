#ifndef TYPES_H
#define TYPES_H

// #include "magic.h"
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <assert.h>
#include <stdatomic.h>
#include <pthread.h>

#define ASSERT(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, \
            "\nASSERTION FAILED\n" \
            "Condition: %s\n" \
            "File: %s\n" \
            "Line: %d\n", \
            #cond, __FILE__, __LINE__); \
        abort(); \
    } \
} while (0)




#define ROOK_TABLE_SIZE 102400

#define BISHOP_TABLE_SIZE 5248

#define MAX_MOVES 400
#define MAX_FEN 87
#define BOARD_MAX 64
#define PIECE_TYPES 6
#define PIECE_COLOR_TYPES 12
#define COLOR_MAX 2
#define COLOR_CASTLESIDE_MAX 4
#define CASTLESIDE_MAX 2
#define RANK_MAX 8
#define FILE_MAX 8
#define MATE_SCORE 100000000
#define MAX_PHASE 24
#define HISTORY_MAX 8192
#define HISTORY_ADDED 14336
// #define CAP_HIST_MAX 4000
#define CAP_HIST_MAX 8192
// #define CAP_HIST_MAX 16384
#define CORRHIST_SIZE 16384
#define CORRHIST_MASK 16383
#define MAX_THREADS 8
#define BUCKET_SIZE 4
#define MAX_PLY 128
#define PIECE_MAX 16

#define SCORE_NONE -32000
#define DRAW_SCORE -32000
#define VALUE_NONE 32001

#define PIECE_OFFSET 0
#define CASTLING_OFFSET 768
#define EN_PASSANT_OFFSET 772
#define TURN_OFFSET 780
#define TT_BIT 23
#define TT_SIZE (1ULL << TT_BIT)
#define TT_MASK (TT_SIZE - 1ULL)
#define EVAL_BIT 22
#define EVAL_SIZE (1ULL << EVAL_BIT)
#define EVAL_MASK (EVAL_SIZE - 1ULL)
#define PAWN_BIT 20
#define PAWN_SIZE (1ULL << PAWN_BIT)
#define PAWN_MASK (PAWN_SIZE - 1ULL)
#define COUNTERMOVE_BIT 21
#define COUNTERMOVE_TABLE_SIZE  (1ULL << COUNTERMOVE_BIT) 
#define COUNTERMOVE_MASK (COUNTERMOVE_TABLE_SIZE - 1ULL)
#define REFUTATION_BIT 21
#define REFUTATION_TABLE_SIZE  (1ULL << REFUTATION_BIT) 
#define REFUTATION_MASK (REFUTATION_TABLE_SIZE - 1ULL)

#define MOVE_NONE 0
#define MOVE_NULL 65
#define SQ_NONE 65

typedef enum MoveType {
    NORMAL    = 0 << 14,
    PROMOTION = 1 << 14,
    ENPASSANT = 2 << 14,
    CASTLE = 3 << 14 
} MoveType;

struct ParamIndex;
typedef enum StartEnd {

  START,
  END
  
} StartEnd;

typedef enum Relation {
  
  FRIEND,
  ENEMY

} Relation;

typedef enum Side {
  
  BLACK, // 0
  WHITE, // 1
  BOTH // 2
  
} Side;

typedef enum LateralDirection {
  NORTH,
  EAST,
  SOUTH,
  WEST
} LateralDirection;

typedef enum DiagonalDirection {
  NORTHEAST,
  SOUTHEAST,
  SOUTHWEST,
  NORTHWEST
} DiagonalDirection;

typedef enum KingLocation {

  KING_QUEENSIDE,
  KING_KINGSIDE,
  KING_CENTER
  
} KingLocation;

typedef enum UCICommand {
  
  UCI_ISREADY,
  UCI_NEWGAME,
  UCI_SETOPTION,
  UCI_POSITION,
  UCI_GO,
  UCI_STOP,
  UCI_QUIT
  
} UCICommand;


typedef enum CommandType {

  COMMAND_MOVE,
  COMMAND_PERFT,
  COMMAND_FEN,
  COMMAND_DISPLAY_BOARD,
  COMMAND_GENERATE_MOVES,
  COMMAND_BESTMOVE,
  COMMAND_GET_KEY,
  COMMAND_AUTOMATE,
  COMMAND_DEBUG_CHECK_HASH,
  COMMAND_DEBUG_EVALUATE,
  COMMAND_UCI,
  COMMAND_QUIT

} CommandType;


typedef enum PickType {

  PICK_TT_MOVE,
  PICK_GOOD_CAP,
  PICK_PROMO,
  PICK_KILLER,
  PICK_QUIET,
  PICK_BAD_CAP

} PickType;
typedef enum PieceType {
  
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING,
  PIECE_NONE
  
} PieceType;

typedef enum Piece {
  
  WHITE_PAWN,
  BLACK_PAWN,
  WHITE_KNIGHT,
  BLACK_KNIGHT,
  WHITE_BISHOP,
  BLACK_BISHOP,
  WHITE_ROOK,
  BLACK_ROOK,
  WHITE_QUEEN,
  BLACK_QUEEN,
  WHITE_KING,
  BLACK_KING

} Piece;

typedef enum Rank {
  
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8

} Rank;

typedef enum File {
  
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H

} File;

typedef int16_t ContinuationHistory[PIECE_TYPES + 1][BOARD_MAX];
typedef int32_t Score;

typedef enum NodeType {
  PV,
  NON_PV
} NodeType;

typedef struct ivec2 { int x; int y; } ivec2;
typedef struct vec2 { float x; float y; } vec2;


typedef struct MagicEntry {
    uint64_t mask;
    uint64_t magic;
    uint8_t shift;
    uint32_t offset;
} MagicEntry;

typedef struct Player {
  
  
} Player;

typedef struct ThreatInfo {
  PieceType p;
  int sq;
  uint64_t attacks;
} ThreatInfo;

typedef struct PieceData {

  uint64_t board;
  int locations[12];
  int piece_count;
  
} PieceData;

typedef enum PromotionType {

  PROMOTION_NONE,
  PROMOTION_KNIGHT,
  PROMOTION_BISHOP,
  PROMOTION_ROOK,
  PROMOTION_QUEEN
  
} PromotionType;

typedef enum ColorCastleSide {
  
  BLACK_QUEENSIDE,
  BLACK_KINGSIDE,
  WHITE_QUEENSIDE,
  WHITE_KINGSIDE
  
} ColorCastleSide;

typedef enum CastleSide {
  
  QUEENSIDE,
  KINGSIDE,
  BOTHSIDE,
  NONESIDE

} CastleSide;


typedef struct {
    int w, b;
} EvalPair;

typedef struct {
    EvalPair pawn;
    EvalPair material;
    EvalPair hanging;
    EvalPair king_threat;
    EvalPair king_safety_raw;
    EvalPair king_safety_scaled;
    EvalPair rook_files;
    EvalPair pins;
    EvalPair development;
    EvalPair endgame_king;

    int mg_total;
    int eg_total;
    int final_score;
    int phase;
} EvalDebug;
        
typedef struct PolyglotEntry {
  uint64_t key;
  uint16_t move;
  uint16_t weight;
  uint32_t learn;
} PolyglotEntry;

typedef struct {
    char fen[256];
    char bestmove[16];
    char id[128];
} TestPos;


typedef enum CastlingRights {
    CWK = 1 << 0,
    CWQ = 1 << 1,
    CBK = 1 << 2,
    CBQ = 1 << 3
} CastlingRights;

typedef struct OpeningBook {

  // PolyglotEntry * entries;
  // size_t count, capacity;
  char path[128];

  
} OpeningBook;

typedef struct MovementMasks{
  
  uint64_t our_attack_mask;
  uint64_t enemy_attack_mask;
  uint64_t our_attacked_pieces;
  uint64_t enemy_attacked_pieces;
  uint64_t our_defended_pieces;
  uint64_t enemy_defended_pieces;
  uint64_t our_hanging_pieces;
  uint64_t enemy_hanging_pieces;
  // pieces pinned to our king / queen
  uint64_t our_pinned_pieces;
  // pieces that are pinned to enemy king / queen
  uint64_t enemy_pinned_pieces;
  
} MovementMasks;

typedef struct SearchParams {
  
  int16_t qdelta_margin;
  int16_t qsee_margin;
  int16_t check_prune_margin;
  int16_t rfp_mul;
  int16_t rfp_base;
  int16_t rfp_improving;
  uint8_t rfp_depth;
  uint8_t razor_depth;
  int16_t razor_base;
  int16_t razor_mul;
  int16_t razor_improving;
  int16_t nmp_mul;
  int16_t nmp_base;
  int16_t nmp_slope;
  uint8_t probcut_depth;
  int16_t probcut_base;
  int16_t probcut_improving;
  uint8_t iid_depth;
  uint8_t lmr_depth;
  uint8_t lmr_move_start;
  uint16_t lmr_hd;
  double lmr_cap_mul;
  double lmr_cap_base;
  double lmr_quiet_mul;
  double lmr_quiet_base;
  uint8_t lmp_depth;
  uint8_t lmp_base;
  double lmp_improving;
  double lmp_depth_pow;
  uint8_t futility_depth;
  int16_t futility_base;
  int16_t futility_mul;
  int16_t futility_hist_mul;
  int16_t futility_improving;
  uint8_t chist_depth;
  int16_t chist1_margin;
  int16_t chist2_margin;
  uint8_t see_depth;
  int16_t mp_goodcap_margin;
  double chist1_scale;
  double chist2_scale;
  double chist4_scale;
  double chist6_scale;
  int16_t see_quiet_margin;
  int16_t see_nonquiet_margin;
  uint8_t se_depth;
  double se_depth_margin;
  int16_t qhistory_base;
  int16_t qhistory_mul;
  int16_t qhpen_base;
  int16_t qhpen_mul;
  int16_t chistory_base;
  int16_t chistory_mul;
  int16_t chpen_base;
  int16_t chpen_mul;
  int16_t beta_bonus;
  int16_t corr_depth_base;
  int16_t corrhist_grain;
  int16_t corrhist_weight;
  int16_t corrhist_max;
  int16_t corr_pawn_weight;
  int16_t corr_np_weight;
  int16_t corr_mat_weight;
  int16_t corr_ch_weight;
  
  int16_t corr_kbn_weight;
  int16_t corr_kqr_weight;
  double eval_scale;
  int16_t aspiration_base;
  int16_t aspiration_mul;

} SearchParams;

typedef struct EvalParams {

  Score PIECE_VALUES[PIECE_TYPES];
  Score P_ISOLATED[8];
  Score P_DOUBLED;
  Score P_BACKWARD[8];
  Score P_CANDIDATE_PASSER[8];
  Score P_CHAIN;
  Score MOBILITY[PIECE_TYPES][64];
  Score BISHOP_PAIR; 
  Score DEFENDED_PIECE[8];
  Score OUTPOST_OCC; 
  Score OUTPOST_CONTROLLED; 
  Score MINOR_CANNOT_ENTER_ET; 
  Score SLIDER_BOXED_IN; 
  Score UNSTOPPABLE_ATTACK; 
  Score RK_OPEN; 
  Score RK_SEMI_OPEN; 
  Score RK_CONNECTED; 
  Score QR_CONNECTED; 
  Score QB_CONNECTED; 
  Score WK_Q; 
  Score TDP_WEAK_SQ[16];
  Score TDP_OED_PEN[16];
  Score ATK_WEAK_P[8];
  Score SAFE_PAWN_PUSH_ATTACKS[16];
  Score SAFE_PAWN_ATTACKS[16];
  Score THREAT_BY_MINOR[PIECE_TYPES];
  Score THREAT_BY_MAJOR[PIECE_TYPES];
  Score WEAK_PC[8];
  Score PASSER_TDP;
  Score PASSER_SUPPORTED;
  Score PASSER_BLOCKED;
  Score OPPONENT_ROOK_STUCK_ON_PROMOTION_RANK;
  Score ROOK_ON_PASSER_FILE;
  Score ROOK_STUCK_IN_FRONT_OF_PASSER;
  Score EXTRA_PASSERS_ON_SEVEN[4];
  Score SAFE_CHECKS[PIECE_TYPES];
  Score UNSAFE_CHECKS[PIECE_TYPES];
  
  
} EvalParams;



typedef struct ParamIndex{

    int piece_values[PIECE_TYPES];

    int psqt[PIECE_TYPES][64];

    int isolated_pawn[8];
    int passed_isolated_pawn[8];
    int doubled_pawn;
    int passed_pawn[8];
    int connected_passer;
    int backward_pawn[8];
    int candidate_passer[8];
    int chained_pawn;

    int mobility[PIECE_TYPES][64];
    int bishop_pair;
    int bishop_pawn_color[8];
    int bishop_pawn_color_w_attacker[8];
    int defended_pc[8];
    int outpost_occ;
    int outpost_control;
    int minor_cannot_enter_et;
    int slider_boxed_in;
    int unstoppable_attack;
    int rook_open;
    int rook_semi_open;
    int connected_rook;
    int qr_connected;
    int qb_connected;
    int weak_queen;
    int tdp_weak_sq[16];
    int tdp_oed_pen[16];
    int unsafe[16];
    int awp[8];
    int safe_pawn_push_attacks[16];
    int safe_pawn_attacks[16];
    int weak_pieces;
    int threat_by_minor[PIECE_TYPES];
    int threat_by_major[PIECE_TYPES];
    int passer_is_unstoppable;
    int passer_deficit;
    int passer_supported;
    int passer_blocked;
    int opponent_stuck_in_front_of_passer;
    int opponent_rook_stuck_on_promo_rank;
    int rook_on_passer_file;
    int rook_stuck_in_front_of_passer;
    int extra_passers_on_seven;

    int ks_safe_checks[PIECE_TYPES];
    int ks_unsafe_checks[PIECE_TYPES];
    int ks_double_safe_king_attacks[16];
    int ks_safe_king_attacks[16];
    int ks_threat_correction[PIECE_TYPES];
    int ks_rook_contact_check;
    int ks_queen_contact_check;
    int ks_akz[PIECE_TYPES];
    int ks_aikz[8];
    int ks_weak;
    int ks_defended_squares_kz[24];
    int ks_def_minor[8];
    int ks_ek_defenders[8];
    int ks_pawn_shield[3];
    int ks_empty[8];

    
    

    
    int total_params;
} ParamIndex;

typedef struct AttackUndo {

  uint8_t id;
  uint64_t attacks;
  uint8_t mob_count;
  
} AttackUndo;

typedef struct Undo {

  uint8_t p_id;
  uint8_t cap_id;
  PieceType p;
  PieceType cp;

} Undo;

typedef uint16_t Move;


typedef struct PackedMove {
  
  uint8_t start;
  uint8_t end;
  uint8_t piece;
  uint8_t capture_piece;
  uint8_t type;
  uint8_t promo_piece;
  Side side;
} PackedMove;


typedef enum TTType {
  EXACT,
  LOWER,
  UPPER,
  PV_EXACT
  
} TTType;



typedef struct {
    uint64_t key;   
    Move move;
    int score;
    int16_t depth;     
    uint8_t type;
    int16_t ply;
    uint16_t gen;
    uint16_t weight;
    bool is_opening_book;
} TTEntry; 
typedef struct {
  TTEntry entries[BUCKET_SIZE];
} TTBucket;


typedef struct {

    Move current_move;
    Move killers[2];
    Move excluded_move;
    int eval;
    uint8_t move_count; 
    PieceType moved_piece;
    PieceType cap_piece;
    ContinuationHistory * ch;
    ContinuationHistory * cr;
    int stat_score;
    uint8_t p_id;
    uint8_t cap_id;

} SearchStack;

typedef struct {

  uint64_t blockers_for_king[COLOR_MAX];
  uint64_t check_sq[PIECE_TYPES];
  
} CheckInfo;

typedef struct StateInfo{

  uint64_t pinners[COLOR_MAX];

  
  uint64_t nonpawn_key[COLOR_MAX];
  uint64_t material_key;
  uint64_t piece_key[PIECE_TYPES];
  Score psqt_score[COLOR_MAX];
  Score material_score[COLOR_MAX];
  uint8_t k_sq[COLOR_MAX];
  uint8_t castle_flags;
  int8_t en_passant_index;
  int rule50;

  uint64_t key;
  CheckInfo ci;
  struct StateInfo * pst;
  
} StateInfo;

typedef struct EvalMasks {
  
  uint64_t am[COLOR_MAX];
  uint64_t am_nk[COLOR_MAX];
  // uint64_t am_np[COLOR_MAX];
  uint64_t am_m[COLOR_MAX];
  uint64_t am_xr_nq[COLOR_MAX];
  uint64_t sppa[COLOR_MAX];
  uint64_t safe[COLOR_MAX];
  // uint64_t am_empty;
  uint64_t am_p[COLOR_MAX][PIECE_TYPES];
  uint64_t datk[COLOR_MAX];
  uint64_t wsq[COLOR_MAX];
  uint64_t out[COLOR_MAX];
  uint64_t tdp[COLOR_MAX];
  uint64_t passers[COLOR_MAX];
  // uint64_t passers_front[COLOR_MAX];
  // uint64_t passers_file[COLOR_MAX];

} EvalMasks;

typedef struct PawnHashEntry {

  uint64_t key;
  Score s;
  uint64_t w_atk;
  uint64_t b_atk;
  uint64_t w_passers;
  uint64_t b_passers;
  
} PawnHashEntry;

typedef struct EvalEntry {

  uint64_t key;
  int eval;
  Side side;
  
} EvalEntry;


typedef struct SearchFlags {

  double max_time;
  bool check_hash;
  bool uci;
  bool mate;
  bool draw;
  bool three_fold_repetition;
  bool use_opening_book;
  int threads;
  int max_depth;
  int nodes;
  
} SearchFlags;

typedef struct SearchData {
  
  double start_time; 
  double max_time; // seconds
  bool has_extended;
  bool nodes_enabled;
  int node_max;


  Move pv_table[64][64];
  int pv_length[64];

  int ply; // ply from current move
  int max_depth; // starting max depth

  int node_count;
  Move current_best;

  SearchFlags flags;

  bool stop;


  int tt_hits;
  int tt_probes;
  int aspiration_fail;
  int lmrs_tried;
  int lmrs_researched;
  int null_prunes;
  int qnodes;
  int ordering_success;
  int futility_prunes;
  int see_prunes;
  int beta_prunes;
  int pawn_hash_probes;
  int pawn_hash_hits;
  int late_move_prunes;
  int q_see_prunes;
  int rfp;
  int razoring;
  int check_extensions;
  int delta_prunes;
  int qdelta_prunes;
  int lazy_cutoffs_s1;
  int lazy_cutoffs_s2;
  int lazy_cutoffs_s3;
  int highest_mat_reached;
  int fast_evals;
  int successful_iids;
  int chist_prunes;
  int check_prunes;
  
  float extensions;
  float reductions;
  bool enable_time;

  // to avoid pv shortening in small aspiration windows
  bool disable_writes;

  double current_best_score;

  bool use_opening_book;
} SearchData;

typedef struct DatasetEntry {
  
  char fen[MAX_FEN + 20];
  double result;
  int param_count;
  long offset; // offset into extraction file

} DatasetEntry;

typedef struct TunerParameter {
  
  char name[128];
  int index;
  
} TunerParameter;


// for tuning
typedef struct Dataset {

  DatasetEntry * entries;
  int count; int capacity;

  
} Dataset;



typedef struct DiscoveredSlider {
  int pos;
  PieceType p;
} DiscoveredSlider;

typedef struct PieceInfo {
  
  PieceType p;
  Side side;
  uint8_t pos;
  bool alive;
  
} PieceInfo;

typedef struct MobilityInfo {
  uint64_t mask;
  PieceType p;
  uint8_t pos;
} MobilityInfo;

typedef struct MovePicker {

  int16_t scores[256];
  int16_t see_scores[256];
  Move moves[256];
  Move bcap[256];
  uint8_t bcap_cnt;
  uint8_t current_bcap;
  uint8_t ordering[256];
  uint8_t move_count;
  uint8_t current_index;
  PickType stage;
  Move tt_move;
  
} MovePicker;



typedef struct Game {
  
  int move_counter;
  Side side_to_move;


  uint64_t board_pieces[COLOR_MAX + 1];
        

  int halfmove;
  int fullmove;

  uint64_t pieces[COLOR_MAX][PIECE_TYPES];

  PieceType piece_at[64];
  

  uint8_t piece_count[COLOR_MAX][PIECE_TYPES];
  uint8_t piece_list[COLOR_MAX][PIECE_TYPES][PIECE_MAX];
  uint8_t piece_index[BOARD_MAX];


  char fen[MAX_MOVES][MAX_FEN];

  bool uci_mode;
  
  uint16_t gen;

  int phase;

  uint64_t key_history[2048];
  bool has_valid_last_move;
  int history_count;

  int fifty_move_clock;
  int fifty_move_clock_was;

  uint8_t piece_uid;

  StateInfo os;
  
  StateInfo * st;
  
} Game;

typedef struct RootData {

    Move move_list[256];
    uint8_t move_count;

    int alpha, beta;

    _Atomic int next_index;

    _Atomic int best_score;
    _Atomic bool game_end;
    Move best_move;
    _Atomic int legal_moves;
    _Atomic int node_count;

    pthread_mutex_t pv_lock;
    Move pv[64];
    int pv_length;
    
    _Atomic bool stop;

    pthread_mutex_t sd_lock;
    SearchData sd;
    
} RootData;

typedef struct ThreadData {

  Game game;
  SearchData search_data;
  RootData * root;

  StateInfo state_stack[1024];
  int last_state;

  int16_t history[COLOR_MAX][BOARD_MAX * BOARD_MAX];
  int16_t cap_hist[PIECE_TYPES][BOARD_MAX][PIECE_TYPES + 1];
  ContinuationHistory continuation_history[PIECE_TYPES + 1][BOARD_MAX];
  ContinuationHistory contcorr[PIECE_TYPES + 1][BOARD_MAX];

  int16_t corrhist_p[COLOR_MAX][CORRHIST_SIZE];
  int16_t corrhist_nonpawns_w[COLOR_MAX][CORRHIST_SIZE];
  int16_t corrhist_nonpawns_b[COLOR_MAX][CORRHIST_SIZE];
  int16_t corrhist_material[COLOR_MAX][CORRHIST_SIZE];
  int16_t corrhist_kbn[COLOR_MAX][CORRHIST_SIZE];
  int16_t corrhist_kqr[COLOR_MAX][CORRHIST_SIZE];
  
  
  uint8_t nmp_min_ply;
  uint8_t nmp_side;

  int id;
  uint64_t seed;
  
} ThreadData;


extern SearchParams sp;
extern Score eval_params[2048];
extern ParamIndex ep_idx;
extern Score PSQT[COLOR_MAX][PIECE_TYPES][BOARD_MAX];

extern uint64_t rook_table[ROOK_TABLE_SIZE];
extern uint64_t bishop_table[BISHOP_TABLE_SIZE];
extern TTBucket tt[TT_SIZE];
extern PawnHashEntry pawn_hash_table[PAWN_SIZE];
extern EvalEntry eval_table[EVAL_SIZE];
extern uint64_t material_randoms[COLOR_MAX][PIECE_TYPES][12];


extern int double_pushed_pawn_squares[COLOR_MAX][64];
extern uint64_t pawn_shield_masks[COLOR_MAX][64];
extern uint64_t adjacent_in_front_masks[COLOR_MAX][64];
extern uint64_t king_zone_masks[COLOR_MAX][64];

// a mask used for determining if our rights will be lost when it's not our turn if one of our rooks gets captured from its starting location
// extern const int rook_starting_locations[COLOR_MAX][2];

extern uint8_t manhattan_distance[64][64];

// start, end
extern uint64_t castle_side_board_halves[CASTLESIDE_MAX];
extern int running;
extern int push_direction[COLOR_MAX];
extern double * weights_mg;
extern double * weights_eg;
extern ParamIndex params;

extern int MOBILITY_BLOCKER_VALUES[2][PIECE_TYPES][PIECE_TYPES];

extern uint64_t castling_rights_masks[BOARD_MAX];

static const uint64_t front[COLOR_MAX][BOARD_MAX] = {
  {
    256ULL,
    512ULL,
    1024ULL,
    2048ULL,
    4096ULL,
    8192ULL,
    16384ULL,
    32768ULL,
    65536ULL,
    131072ULL,
    262144ULL,
    524288ULL,
    1048576ULL,
    2097152ULL,
    4194304ULL,
    8388608ULL,
    16777216ULL,
    33554432ULL,
    67108864ULL,
    134217728ULL,
    268435456ULL,
    536870912ULL,
    1073741824ULL,
    2147483648ULL,
    4294967296ULL,
    8589934592ULL,
    17179869184ULL,
    34359738368ULL,
    68719476736ULL,
    137438953472ULL,
    274877906944ULL,
    549755813888ULL,
    1099511627776ULL,
    2199023255552ULL,
    4398046511104ULL,
    8796093022208ULL,
    17592186044416ULL,
    35184372088832ULL,
    70368744177664ULL,
    140737488355328ULL,
    281474976710656ULL,
    562949953421312ULL,
    1125899906842624ULL,
    2251799813685248ULL,
    4503599627370496ULL,
    9007199254740992ULL,
    18014398509481984ULL,
    36028797018963968ULL,
    72057594037927936ULL,
    144115188075855872ULL,
    288230376151711744ULL,
    576460752303423488ULL,
    1152921504606846976ULL,
    2305843009213693952ULL,
    4611686018427387904ULL,
    9223372036854775808ULL,
    0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 0, 0, 0, 0, 0,
    1ULL,
    2ULL,
    4ULL,
    8ULL,
    16ULL,
    32ULL,
    64ULL,
    128ULL,
    256ULL,
    512ULL,
    1024ULL,
    2048ULL,
    4096ULL,
    8192ULL,
    16384ULL,
    32768ULL,
    65536ULL,
    131072ULL,
    262144ULL,
    524288ULL,
    1048576ULL,
    2097152ULL,
    4194304ULL,
    8388608ULL,
    16777216ULL,
    33554432ULL,
    67108864ULL,
    134217728ULL,
    268435456ULL,
    536870912ULL,
    1073741824ULL,
    2147483648ULL,
    4294967296ULL,
    8589934592ULL,
    17179869184ULL,
    34359738368ULL,
    68719476736ULL,
    137438953472ULL,
    274877906944ULL,
    549755813888ULL,
    1099511627776ULL,
    2199023255552ULL,
    4398046511104ULL,
    8796093022208ULL,
    17592186044416ULL,
    35184372088832ULL,
    70368744177664ULL,
    140737488355328ULL,
    281474976710656ULL,
    562949953421312ULL,
    1125899906842624ULL,
    2251799813685248ULL,
    4503599627370496ULL,
    9007199254740992ULL,
    18014398509481984ULL,
    36028797018963968ULL,
  }
};

static const uint64_t bits[BOARD_MAX] = {
  1ULL,
  2ULL,
  4ULL,
  8ULL,
  16ULL,
  32ULL,
  64ULL,
  128ULL,
  256ULL,
  512ULL,
  1024ULL,
  2048ULL,
  4096ULL,
  8192ULL,
  16384ULL,
  32768ULL,
  65536ULL,
  131072ULL,
  262144ULL,
  524288ULL,
  1048576ULL,
  2097152ULL,
  4194304ULL,
  8388608ULL,
  16777216ULL,
  33554432ULL,
  67108864ULL,
  134217728ULL,
  268435456ULL,
  536870912ULL,
  1073741824ULL,
  2147483648ULL,
  4294967296ULL,
  8589934592ULL,
  17179869184ULL,
  34359738368ULL,
  68719476736ULL,
  137438953472ULL,
  274877906944ULL,
  549755813888ULL,
  1099511627776ULL,
  2199023255552ULL,
  4398046511104ULL,
  8796093022208ULL,
  17592186044416ULL,
  35184372088832ULL,
  70368744177664ULL,
  140737488355328ULL,
  281474976710656ULL,
  562949953421312ULL,
  1125899906842624ULL,
  2251799813685248ULL,
  4503599627370496ULL,
  9007199254740992ULL,
  18014398509481984ULL,
  36028797018963968ULL,
  72057594037927936ULL,
  144115188075855872ULL,
  288230376151711744ULL,
  576460752303423488ULL,
  1152921504606846976ULL,
  2305843009213693952ULL,
  4611686018427387904ULL,
  9223372036854775808ULL,
};

static const uint64_t pawn_moves[COLOR_MAX][BOARD_MAX] = {
  {
    65792ULL,
    131584ULL,
    263168ULL,
    526336ULL,
    1052672ULL,
    2105344ULL,
    4210688ULL,
    8421376ULL,
    16842752ULL,
    33685504ULL,
    67371008ULL,
    134742016ULL,
    269484032ULL,
    538968064ULL,
    1077936128ULL,
    2155872256ULL,
    16777216ULL,
    33554432ULL,
    67108864ULL,
    134217728ULL,
    268435456ULL,
    536870912ULL,
    1073741824ULL,
    2147483648ULL,
    4294967296ULL,
    8589934592ULL,
    17179869184ULL,
    34359738368ULL,
    68719476736ULL,
    137438953472ULL,
    274877906944ULL,
    549755813888ULL,
    1099511627776ULL,
    2199023255552ULL,
    4398046511104ULL,
    8796093022208ULL,
    17592186044416ULL,
    35184372088832ULL,
    70368744177664ULL,
    140737488355328ULL,
    281474976710656ULL,
    562949953421312ULL,
    1125899906842624ULL,
    2251799813685248ULL,
    4503599627370496ULL,
    9007199254740992ULL,
    18014398509481984ULL,
    36028797018963968ULL,
    72057594037927936ULL,
    144115188075855872ULL,
    288230376151711744ULL,
    576460752303423488ULL,
    1152921504606846976ULL,
    2305843009213693952ULL,
    4611686018427387904ULL,
    9223372036854775808ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
  },
  {
        0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    1ULL,
    2ULL,
    4ULL,
    8ULL,
    16ULL,
    32ULL,
    64ULL,
    128ULL,
    256ULL,
    512ULL,
    1024ULL,
    2048ULL,
    4096ULL,
    8192ULL,
    16384ULL,
    32768ULL,
    65536ULL,
    131072ULL,
    262144ULL,
    524288ULL,
    1048576ULL,
    2097152ULL,
    4194304ULL,
    8388608ULL,
    16777216ULL,
    33554432ULL,
    67108864ULL,
    134217728ULL,
    268435456ULL,
    536870912ULL,
    1073741824ULL,
    2147483648ULL,
    4294967296ULL,
    8589934592ULL,
    17179869184ULL,
    34359738368ULL,
    68719476736ULL,
    137438953472ULL,
    274877906944ULL,
    549755813888ULL,
    1103806595072ULL,
    2207613190144ULL,
    4415226380288ULL,
    8830452760576ULL,
    17660905521152ULL,
    35321811042304ULL,
    70643622084608ULL,
    141287244169216ULL,
    282574488338432ULL,
    565148976676864ULL,
    1130297953353728ULL,
    2260595906707456ULL,
    4521191813414912ULL,
    9042383626829824ULL,
    18084767253659648ULL,
    36169534507319296ULL,
  }
};

static const uint64_t pawn_captures[COLOR_MAX][BOARD_MAX] = {
  {
      512ULL,
    1280ULL,
    2560ULL,
    5120ULL,
    10240ULL,
    20480ULL,
    40960ULL,
    16384ULL,
    131072ULL,
    327680ULL,
    655360ULL,
    1310720ULL,
    2621440ULL,
    5242880ULL,
    10485760ULL,
    4194304ULL,
    33554432ULL,
    83886080ULL,
    167772160ULL,
    335544320ULL,
    671088640ULL,
    1342177280ULL,
    2684354560ULL,
    1073741824ULL,
    8589934592ULL,
    21474836480ULL,
    42949672960ULL,
    85899345920ULL,
    171798691840ULL,
    343597383680ULL,
    687194767360ULL,
    274877906944ULL,
    2199023255552ULL,
    5497558138880ULL,
    10995116277760ULL,
    21990232555520ULL,
    43980465111040ULL,
    87960930222080ULL,
    175921860444160ULL,
    70368744177664ULL,
    562949953421312ULL,
    1407374883553280ULL,
    2814749767106560ULL,
    5629499534213120ULL,
    11258999068426240ULL,
    22517998136852480ULL,
    45035996273704960ULL,
    18014398509481984ULL,
    144115188075855872ULL,
    360287970189639680ULL,
    720575940379279360ULL,
    1441151880758558720ULL,
    2882303761517117440ULL,
    5764607523034234880ULL,
    11529215046068469760ULL,
    4611686018427387904ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
  },
  {
      0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    0ULL,
    2ULL,
    5ULL,
    10ULL,
    20ULL,
    40ULL,
    80ULL,
    160ULL,
    64ULL,
    512ULL,
    1280ULL,
    2560ULL,
    5120ULL,
    10240ULL,
    20480ULL,
    40960ULL,
    16384ULL,
    131072ULL,
    327680ULL,
    655360ULL,
    1310720ULL,
    2621440ULL,
    5242880ULL,
    10485760ULL,
    4194304ULL,
    33554432ULL,
    83886080ULL,
    167772160ULL,
    335544320ULL,
    671088640ULL,
    1342177280ULL,
    2684354560ULL,
    1073741824ULL,
    8589934592ULL,
    21474836480ULL,
    42949672960ULL,
    85899345920ULL,
    171798691840ULL,
    343597383680ULL,
    687194767360ULL,
    274877906944ULL,
    2199023255552ULL,
    5497558138880ULL,
    10995116277760ULL,
    21990232555520ULL,
    43980465111040ULL,
    87960930222080ULL,
    175921860444160ULL,
    70368744177664ULL,
    562949953421312ULL,
    1407374883553280ULL,
    2814749767106560ULL,
    5629499534213120ULL,
    11258999068426240ULL,
    22517998136852480ULL,
    45035996273704960ULL,
    18014398509481984ULL,
  }
};
static const uint64_t knight_moves[BOARD_MAX] = {
  132096ULL,
  329728ULL,
  659712ULL,
  1319424ULL,
  2638848ULL,
  5277696ULL,
  10489856ULL,
  4202496ULL,
  33816580ULL,
  84410376ULL,
  168886289ULL,
  337772578ULL,
  675545156ULL,
  1351090312ULL,
  2685403152ULL,
  1075839008ULL,
  8657044482ULL,
  21609056261ULL,
  43234889994ULL,
  86469779988ULL,
  172939559976ULL,
  345879119952ULL,
  687463207072ULL,
  275414786112ULL,
  2216203387392ULL,
  5531918402816ULL,
  11068131838464ULL,
  22136263676928ULL,
  44272527353856ULL,
  88545054707712ULL,
  175990581010432ULL,
  70506185244672ULL,
  567348067172352ULL,
  1416171111120896ULL,
  2833441750646784ULL,
  5666883501293568ULL,
  11333767002587136ULL,
  22667534005174272ULL,
  45053588738670592ULL,
  18049583422636032ULL,
  145241105196122112ULL,
  362539804446949376ULL,
  725361088165576704ULL,
  1450722176331153408ULL,
  2901444352662306816ULL,
  5802888705324613632ULL,
  11533718717099671552ULL,
  4620693356194824192ULL,
  288234782788157440ULL,
  576469569871282176ULL,
  1224997833292120064ULL,
  2449995666584240128ULL,
  4899991333168480256ULL,
  9799982666336960512ULL,
  1152939783987658752ULL,
  2305878468463689728ULL,
  1128098930098176ULL,
  2257297371824128ULL,
  4796069720358912ULL,
  9592139440717824ULL,
  19184278881435648ULL,
  38368557762871296ULL,
  4679521487814656ULL,
  9077567998918656ULL,
};
static const uint64_t king_moves[BOARD_MAX] = {
    770ULL,
  1797ULL,
  3594ULL,
  7188ULL,
  14376ULL,
  28752ULL,
  57504ULL,
  49216ULL,
  197123ULL,
  460039ULL,
  920078ULL,
  1840156ULL,
  3680312ULL,
  7360624ULL,
  14721248ULL,
  12599488ULL,
  50463488ULL,
  117769984ULL,
  235539968ULL,
  471079936ULL,
  942159872ULL,
  1884319744ULL,
  3768639488ULL,
  3225468928ULL,
  12918652928ULL,
  30149115904ULL,
  60298231808ULL,
  120596463616ULL,
  241192927232ULL,
  482385854464ULL,
  964771708928ULL,
  825720045568ULL,
  3307175149568ULL,
  7718173671424ULL,
  15436347342848ULL,
  30872694685696ULL,
  61745389371392ULL,
  123490778742784ULL,
  246981557485568ULL,
  211384331665408ULL,
  846636838289408ULL,
  1975852459884544ULL,
  3951704919769088ULL,
  7903409839538176ULL,
  15806819679076352ULL,
  31613639358152704ULL,
  63227278716305408ULL,
  54114388906344448ULL,
  216739030602088448ULL,
  505818229730443264ULL,
  1011636459460886528ULL,
  2023272918921773056ULL,
  4046545837843546112ULL,
  8093091675687092224ULL,
  16186183351374184448ULL,
  13853283560024178688ULL,
  144959613005987840ULL,
  362258295026614272ULL,
  724516590053228544ULL,
  1449033180106457088ULL,
  2898066360212914176ULL,
  5796132720425828352ULL,
  11592265440851656704ULL,
  4665729213955833856ULL,
};
static const uint64_t bishop_moves[BOARD_MAX] = {
  9241421688590303744ULL,
  36099303471056128ULL,
  141012904249856ULL,
  550848566272ULL,
  6480472064ULL,
  1108177604608ULL,
  283691315142656ULL,
  72624976668147712ULL,
  4620710844295151618ULL,
  9241421688590368773ULL,
  36099303487963146ULL,
  141017232965652ULL,
  1659000848424ULL,
  283693466779728ULL,
  72624976676520096ULL,
  145249953336262720ULL,
  2310355422147510788ULL,
  4620710844311799048ULL,
  9241421692918565393ULL,
  36100411639206946ULL,
  424704217196612ULL,
  72625527495610504ULL,
  145249955479592976ULL,
  290499906664153120ULL,
  1155177711057110024ULL,
  2310355426409252880ULL,
  4620711952330133792ULL,
  9241705379636978241ULL,
  108724279602332802ULL,
  145390965166737412ULL,
  290500455356698632ULL,
  580999811184992272ULL,
  577588851267340304ULL,
  1155178802063085600ULL,
  2310639079102947392ULL,
  4693335752243822976ULL,
  9386671504487645697ULL,
  326598935265674242ULL,
  581140276476643332ULL,
  1161999073681608712ULL,
  288793334762704928ULL,
  577868148797087808ULL,
  1227793891648880768ULL,
  2455587783297826816ULL,
  4911175566595588352ULL,
  9822351133174399489ULL,
  1197958188344280066ULL,
  2323857683139004420ULL,
  144117404414255168ULL,
  360293502378066048ULL,
  720587009051099136ULL,
  1441174018118909952ULL,
  2882348036221108224ULL,
  5764696068147249408ULL,
  11529391036782871041ULL,
  4611756524879479810ULL,
  567382630219904ULL,
  1416240237150208ULL,
  2833579985862656ULL,
  5667164249915392ULL,
  11334324221640704ULL,
  22667548931719168ULL,
  45053622886727936ULL,
  18049651735527937ULL,
};
static const uint64_t rook_moves[BOARD_MAX] = {
  72340172838076926ULL,
  144680345676153597ULL,
  289360691352306939ULL,
  578721382704613623ULL,
  1157442765409226991ULL,
  2314885530818453727ULL,
  4629771061636907199ULL,
  9259542123273814143ULL,
  72340172838141441ULL,
  144680345676217602ULL,
  289360691352369924ULL,
  578721382704674568ULL,
  1157442765409283856ULL,
  2314885530818502432ULL,
  4629771061636939584ULL,
  9259542123273813888ULL,
  72340172854657281ULL,
  144680345692602882ULL,
  289360691368494084ULL,
  578721382720276488ULL,
  1157442765423841296ULL,
  2314885530830970912ULL,
  4629771061645230144ULL,
  9259542123273748608ULL,
  72340177082712321ULL,
  144680349887234562ULL,
  289360695496279044ULL,
  578721386714368008ULL,
  1157442769150545936ULL,
  2314885534022901792ULL,
  4629771063767613504ULL,
  9259542123257036928ULL,
  72341259464802561ULL,
  144681423712944642ULL,
  289361752209228804ULL,
  578722409201797128ULL,
  1157443723186933776ULL,
  2314886351157207072ULL,
  4629771607097753664ULL,
  9259542118978846848ULL,
  72618349279904001ULL,
  144956323094725122ULL,
  289632270724367364ULL,
  578984165983651848ULL,
  1157687956502220816ULL,
  2315095537539358752ULL,
  4629910699613634624ULL,
  9259541023762186368ULL,
  143553341945872641ULL,
  215330564830528002ULL,
  358885010599838724ULL,
  645993902138460168ULL,
  1220211685215703056ULL,
  2368647251370188832ULL,
  4665518383679160384ULL,
  9259260648297103488ULL,
  18302911464433844481ULL,
  18231136449196065282ULL,
  18087586418720506884ULL,
  17800486357769390088ULL,
  17226286235867156496ULL,
  16077885992062689312ULL,
  13781085504453754944ULL,
  9187484529235886208ULL,
};



static const char piece_names[12] = {
    'P',
    'p',
    'N',
    'n',
    'B',
    'b',
    'R',
    'r',
    'Q',
    'q',
    'K',
    'k'
};
static const char PNAME[6] = {
    'P',
    'N',
    'B',
    'R',
    'Q',
    'K',
};

static const char file_names[8] = {
    'a',
    'b',
    'c',
    'd',
    'e',
    'f',
    'g',
    'h'
};

static const char rank_names[8] = {
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8'
};

static const char move_types[5][24] = {
    "MOVE",
    "CAPTURE",
    "CASTLE",
    "EN_PASSANT",
    "PROMOTION"
};
static const char promotion_types[5][24] = {
    "PROMOTION_NONE",
    "PROMOTION_KNIGHT",
    "PROMOTION_BISHOP",
    "PROMOTION_ROOK",
    "PROMOTION_QUEEN"
};


static const uint64_t one_and_two_file_mask = 0x303030303030303ULL;
static const uint64_t seven_and_eight_file_mask = 0xc0c0c0c0c0c0c0c0ULL;

static const uint64_t adjacent_file_masks[8] = {
    0x202020202020202ULL,
    0x505050505050505ULL,
    0xa0a0a0a0a0a0a0aULL,
    0x1414141414141414ULL,
    0x2828282828282828ULL,
    0x5050505050505050ULL,
    0xa0a0a0a0a0a0a0a0ULL,
    0x4040404040404040ULL
};
static const uint64_t file_masks[8] = {
    
    0x101010101010101ULL,
    0x202020202020202ULL,
    0x404040404040404ULL,
    0x808080808080808ULL,
    0x1010101010101010ULL,
    0x2020202020202020ULL,
    0x4040404040404040ULL,
    0x8080808080808080ULL,
    
};

static const uint64_t double_push_masks[2] = 
{
  
    0xff000000ULL,
    0xff00000000ULL,
};

static const uint64_t rank_masks[8] = {

    0xff00000000000000ULL,
    0xff000000000000ULL,
    0xff0000000000ULL,
    0xff00000000ULL,
    0xff000000ULL,
    0xff0000ULL,
    0xff00ULL,
    0xffULL,
    
};

// enemy territory 2 ranks
static const uint64_t ET2[COLOR_MAX] = {
  0xffff000000000000ULL,
  0xffffULL
};

// enemy territory 3 ranks
static const uint64_t ET3[COLOR_MAX] = {
  0xffffff0000000000ULL,
  0xffffffULL
};

static const uint64_t board_halves[COLOR_MAX] = {
  0xffffffffULL,
  0xffffffff00000000ULL
};

static const uint64_t promotion_ranks[COLOR_MAX] = {
    0xff00000000000000ULL, // black = rank 1
    0xffULL, // white = rank 8
};

static const uint64_t castle_masks[COLOR_MAX][CASTLESIDE_MAX] = {
    {
        0x1cULL, // black queenside
        0x70ULL // black kingside
    },
    {
        0x1c00000000000000ULL, // white queenside
        0x7000000000000000ULL  // white kingside
    },
};
static const uint64_t castle_occupation_masks[COLOR_MAX][CASTLESIDE_MAX] = {
    {
        0xeULL, // black queenside
        0x60ULL  // black kingside
    },
    {
        0xe00000000000000ULL, // white queenside
        0x6000000000000000ULL // white kingside
    },
};

// start and end locations for rooks during castling moves
static const int rook_castle_locations[COLOR_MAX][CASTLESIDE_MAX][2] = {
    
    // black
    {
        // queenside
        {
            0, 3 
        },
        // kingside
        {
            7, 5
        },
    
    },
    
    // white
    {

        // queenside
        {
            56, 59
        },
        // kingside
        {
            63, 61
        },
    
    }
    
};

static const int king_end_locations[COLOR_MAX][CASTLESIDE_MAX] =
{
  {2, 6}, {58, 62}
};
static const int king_castle_locations[COLOR_MAX][CASTLESIDE_MAX][2] = {
    
    // black
    {
        // queenside
        {
            4, 2 
        },
        // kingside
        {
            4, 6
        },
    
    },
    
    // white
    {

        // queenside
        {
            60, 58
        },
        // kingside
        {
            60, 62
        },
    
    }
    
};
static const int castle_attack_squares[COLOR_MAX][CASTLESIDE_MAX][3] = {
    
    // black
    {
        // queenside
        {
            4, 3, 2 
        },
        // kingside
        {
            4, 5, 6
        },
    
    },
    
    // white
    {

        // queenside
        {
            60, 59, 58
        },
        // kingside
        {
            60, 61, 62
        },
    
    }
    
};
static const uint8_t center_manhattan_distance[64] = { 

    6, 5, 4, 3, 3, 4, 5, 6,
    5, 4, 3, 2, 2, 3, 4, 5,
    4, 3, 2, 1, 1, 2, 3, 4,
    3, 2, 1, 0, 0, 1, 2, 3,
    3, 2, 1, 0, 0, 1, 2, 3,
    4, 3, 2, 1, 1, 2, 3, 4,
    5, 4, 3, 2, 2, 3, 4, 5,
    6, 5, 4, 3, 3, 4, 5, 6

};

static const int rook_starting_locations[COLOR_MAX][2] = 
{
    // black
    {0, 7},
    {56, 63}
};

static const int king_starting_locations[COLOR_MAX] = {4, 60};
static const int phase_values[PIECE_TYPES] = 
{
    0,
    1,
    1,
    2,
    4,
    0
};

static const int PVAL[PIECE_TYPES+1] = {100, 330, 350, 500, 900, 0, 0};
static const int piece_values_mg[PIECE_TYPES] = 
{
    // pawns
    84,
    // knights
    333,
    // bishops
    346,
    // rooks
    441,
    // queens
    921,
    // kings
    0
};
static const int piece_values_eg[PIECE_TYPES] = 
{
    // pawns
    106,
    // knights
    244,
    // bishops
    268,
    // rooks
    478,
    // queens
    886,
    // kings
    0
};

static const int mvv_lva[PIECE_TYPES][PIECE_TYPES] = 
{
    //  P    K    B    R    Q
    {
        105, 205, 305, 405, 505 // PAWN
    },
    {
        104, 204, 304, 404, 504 // KNIGHT
    },
    {
        103, 203, 303, 403, 503 // BISHOP
    },
    {
        102, 202, 302, 402, 502 // ROOK
    },
    {
        101, 201, 301, 401, 501 // QUEEN
    },
        
};



static const uint64_t center_squares_mask = 0x3c3c3c3c0000ULL;


extern int THREAT_SCORE[500];

extern int KING_DANGER[1000];

extern int KING_THREAT[1000];
extern int PASSER_SCORE[200];

extern int PSQT_SCALE[10000];

extern int LMR_PV[64][64];
extern int LMR_NON_PV[64][64];
extern int LMP[64];
extern int FP[64][64];
static const uint64_t STARTING_SQUARES[COLOR_MAX][PIECE_TYPES] = 
{
{
    
    0xff00ULL,
    0x24ULL,
    0x42ULL,
    0x81ULL,
    0x8ULL,
    0x10ULL
    
},
    
{
    0xff000000000000ULL,
    0x2400000000000000ULL,
    0x4200000000000000ULL,
    0x8100000000000000ULL,
    0x800000000000000ULL,
    0x1000000000000000ULL
}
};

static const uint64_t COLOR_SQUARES[COLOR_MAX] = 
{
  0x55aa55aa55aa55aaULL,
  0xaa55aa55aa55aa55ULL
};

extern int PSQT_MG[COLOR_MAX][PIECE_TYPES][64];
extern int PSQT_EG[COLOR_MAX][PIECE_TYPES][64];


extern int PAWN_STORM_PSQT_MG[COLOR_MAX][64];
extern int PAWN_STORM_PSQT_EG[COLOR_MAX][64];

extern uint64_t between_sq[64][64];

extern const uint8_t center_manhattan_distance[64];


extern StateInfo state_stack[1024];
extern int st_idx;

extern const MagicEntry ROOK_MAGICS[BOARD_MAX];
extern const MagicEntry BISHOP_MAGICS[BOARD_MAX];

extern uint64_t DIAGONAL_RAYS[4][64];
extern uint64_t LATERAL_RAYS[4][64];

extern uint64_t passed_pawn_masks[COLOR_MAX][64];

extern uint64_t in_front_ranks_masks[COLOR_MAX][64];

extern int SQ_TO_FILE[64];
extern int SQ_TO_RANK[64];


// a file in push direction for each color from a certain square
// used to check if pawns are actually passers or if they are doubled
extern uint64_t in_front_file_masks[COLOR_MAX][64];

// rank inversion
static inline int flip_square(int sq) {
    return sq ^ 56;
}


#endif
