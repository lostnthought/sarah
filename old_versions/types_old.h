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


/* this file, due to my atypical brain chemistry, is a dumping ground for many many things, not just types. feel free to peruse. */


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
#define DRAW_SCORE -1000000
#define MAX_PHASE 24
#define HISTORY_MAX 8192
#define HISTORY_ADDED 14336
// #define CAP_HIST_MAX 4000
#define CAP_HIST_MAX 8192
#define CORRHIST_SIZE 16384
#define CORRHIST_MASK 16383
#define MAX_THREADS 8
#define BUCKET_SIZE 4
#define MAX_PLY 128

#define SCORE_NONE -32000

// #define HISTORY_MAX 24000

// #define HISTORY_MAX 1024


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
  
  BLACK,
  WHITE,
  BOTH
  
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
  // PICK_MID_CAP,
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


typedef struct ParamIndex{
    int isolated_pawn[2];
    int doubled_pawn[2];
    int passed_pawn[2];
    int backward_pawn[2];
    int candidate_passer[2];
    int chained_pawn[2];
    // int additional_pawn_storm_bonus[2];

    int bishop_pair[2];
    int rook_semi_open[2];
    int rook_open[2];
    int connected_rook_bonus[2];


    // int pawn_shield[2];
    // int pawn_storm_bonus;

    // int dist_to_center[2];

    int king_threat_value[2][PIECE_TYPES];
    // int king_threat_base[2];
    int open_file_king_penalty[2];
    int semi_open_file_king_penalty[2];
    int open_middle_file_king_penalty[2];
    int semi_open_diag_king_penalty[2];
    int open_diag_king_penalty[2];
    int semi_open_diag_from_king_penalty[2];
    int open_diag_from_king_penalty[2];

    int mobility[2][PIECE_TYPES];
    // int mobility_eg[PIECE_TYPES];

    int piece_values[2][PIECE_TYPES];
    // int piece_values_eg[2][PIECE_TYPES];

    int psqt[2][PIECE_TYPES][64];
    // int psqt_eg[PIECE_TYPES][64];
    // int psqt_eg;

    // int pawn_storm_psqt[2][64];
    // int pawn_storm_psqt_eg;

    int queen_rook_connected[2];
    int queen_bishop_connected[2];
    
    int king_zone_attackers[2];
    int king_zone_defenders[2];
    int king_zone_battery[2];
    int pawn_blockers_penalty[2];
    // int attack_tempo[2];
    // int double_attack[2];
    int pin_penalty[2];
    // int pin_attacked_twice_penalty[2];
    int open_file_near_king_with_attacker[2];
    int open_file_from_king_with_attacker[2];
    int open_diag_near_king_with_attacker[2];
    int open_diag_from_king_with_attacker[2];

    int attacked_by_pawn_penalty[2];

    int total_params;
} ParamIndex;

typedef struct AttackUndo {

  uint8_t id;
  uint64_t attacks;
  uint8_t mob_count;
  
} AttackUndo;

typedef struct Undo {

  AttackUndo attack_undos[12];
  int attack_undo_count;
  uint64_t piece_key[PIECE_TYPES];
  uint64_t nonpawn_key[COLOR_MAX];
  uint64_t key;
  bool castle_flags[COLOR_MAX][CASTLESIDE_MAX];
  int psqt_eval_mg[COLOR_MAX];
  int psqt_eval_eg[COLOR_MAX];
  int ep_sq;
  uint8_t p_id;
  uint8_t cap_id;
  PieceType p;
  PieceType cp;
  uint64_t attack_undo_mask;
  bool was_capture;
  int k_sq[COLOR_MAX];
  int atk_kz[COLOR_MAX];

} Undo;

typedef uint16_t Move;

// typedef struct Move {

//   int start_index;
//   int end_index;
//   MoveType type;
//   Side side;
//   PieceType piece;
//   PieceType capture_piece;
//   CastleSide castle_side;
//   PieceType promotion_type;
//   bool promotion_capture;
//   bool double_push;
//   bool loses_rights;
//   CastleSide rights_toggle; // what rights you lose when we execute this move
//   bool is_checking;
//   bool good_quiet;
//   int see;
//   bool seed;
//   int score; // for move ordering
//   bool scored;
  
// } Move;


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

} SearchStack;

typedef struct {

  uint64_t blockers_for_king[COLOR_MAX];
  uint64_t check_sq[PIECE_TYPES];
  
} CheckInfo;

typedef struct StateInfo{

  CheckInfo ci;
  struct StateInfo * pst;
  uint64_t pinners[COLOR_MAX];
  
} StateInfo;

typedef struct EvalMasks {
  
  uint64_t am[COLOR_MAX];
  uint64_t am_nk[COLOR_MAX];
  uint64_t am_np[COLOR_MAX];
  uint64_t am_m[COLOR_MAX];
  uint64_t safe[COLOR_MAX];
  uint64_t am_empty;
  uint64_t am_p[COLOR_MAX][PIECE_TYPES];
  uint64_t am_mtm[COLOR_MAX];
  uint64_t am_mtr[COLOR_MAX];
  uint64_t see_pos[COLOR_MAX];
  uint64_t see_eq[COLOR_MAX];
  uint64_t kz_atk[COLOR_MAX];
  uint64_t datk[COLOR_MAX];
  uint64_t tatk[COLOR_MAX];
  uint64_t pa[COLOR_MAX];
  uint64_t wsq[COLOR_MAX];
  uint64_t out[COLOR_MAX];
  uint64_t tdp[COLOR_MAX];
  uint64_t passers[COLOR_MAX];
  uint64_t passers_front[COLOR_MAX];
  uint64_t passers_file[COLOR_MAX];

} EvalMasks;

typedef struct PawnHashEntry {

  uint64_t key;
  int mg_score;
  int eg_score;
  uint64_t w_atk;
  uint64_t b_atk;
  uint64_t w_passers;
  uint64_t b_passers;
  uint64_t w_passers_front;
  uint64_t b_passers_front;
  uint64_t w_passers_file;
  uint64_t b_passers_file;
  
} PawnHashEntry;

typedef struct EvalEntry {

  uint64_t key;
  int eval;
  int threats;
  int mkd;
  Side side;
  
} EvalEntry;


typedef struct SearchFlags {

  float max_time;
  bool check_hash;
  bool uci;
  bool mate;
  bool three_fold_repetition;
  bool use_opening_book;
  int threads;
  int max_depth;
  
} SearchFlags;

typedef struct SearchData {
  
  double start_time; 
  double max_time; // seconds
  bool has_extended;


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
  float extensions;
  float reductions;
  bool enable_time;

  // to avoid pv shortening in small aspiration windows
  bool disable_writes;

  double current_best_score;

  bool use_opening_book;
} SearchData;

typedef struct DatasetEntry {
  
  char fen[MAX_FEN];
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
  uint8_t ordering[256];
  uint8_t move_count;
  uint8_t current_index;
  PickType stage;
  Move tt_move;
  
} MovePicker;



typedef struct Game {
  
  int move_counter;
  Side side_to_move;
  uint64_t key;
  uint64_t nonpawn_key[COLOR_MAX];
  uint64_t piece_key[PIECE_TYPES];

  uint64_t board_pieces[COLOR_MAX + 1];

  bool castle_flags[COLOR_MAX][CASTLESIDE_MAX];
        
  int en_passant_index;
  int recap_square;
  int last_recap_square;

  int halfmove;
  int fullmove;

  uint64_t pieces[COLOR_MAX][PIECE_TYPES];
  PieceType piece_at[64];
  
  uint8_t sq_to_uid[64];

  PieceInfo piece_info[32];
  uint64_t piece_attacks[32];

  // attackers to enemy king zone in threat score
  int atk_kz[COLOR_MAX];

  // square the king is on. necessary for incrementalizing king attacks
  int k_sq[COLOR_MAX];

  char fen[MAX_MOVES][MAX_FEN];



  int psqt_evaluation_mg[COLOR_MAX];
  int psqt_evaluation_eg[COLOR_MAX];


  bool uci_mode;
  
  uint16_t gen;

  int phase;

  uint64_t key_history[2048];
  Move last_move;
  Move last_last_move;
  Move last_last_last_move;
  bool has_valid_last_move;
  int history_count;

  int fifty_move_clock;
  int fifty_move_clock_was;

  uint8_t piece_uid;

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

    Move pv[64];
    int pv_length;
    
    _Atomic bool stop;
    pthread_mutex_t pv_lock;

    pthread_mutex_t sd_lock;
    SearchData sd;
    
} RootData;

typedef struct ThreadData {

  Game game;
  SearchData search_data;
  RootData * root;

  int id;
  uint64_t seed;
  
} ThreadData;

extern uint64_t rook_table[ROOK_TABLE_SIZE];
extern uint64_t bishop_table[BISHOP_TABLE_SIZE];
extern int16_t conthist[COLOR_MAX * PIECE_TYPES * 64][COLOR_MAX * PIECE_TYPES * 64];
extern int16_t history[COLOR_MAX][PIECE_TYPES][64][64];
extern int16_t capture_history[PIECE_TYPES][BOARD_MAX][PIECE_TYPES];
extern Move killer_moves[64][2];

extern int corrhist_p[COLOR_MAX][CORRHIST_SIZE];
extern int corrhist_nonpawns_w[COLOR_MAX][CORRHIST_SIZE];
extern int corrhist_nonpawns_b[COLOR_MAX][CORRHIST_SIZE];
extern int corrhist_kbn[COLOR_MAX][CORRHIST_SIZE];
extern int corrhist_kqr[COLOR_MAX][CORRHIST_SIZE];
extern TTEntry tt[TT_SIZE];
extern PawnHashEntry pawn_hash_table[PAWN_SIZE];
extern EvalEntry eval_table[EVAL_SIZE];


extern int double_pushed_pawn_squares[COLOR_MAX][64];
extern uint64_t pawn_shield_masks[COLOR_MAX][64];
extern uint64_t adjacent_in_front_masks[COLOR_MAX][64];
extern uint64_t king_zone_masks[64];

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

static const int PVAL[PIECE_TYPES] = {100, 330, 350, 500, 900, 0};
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
static const int MOBILITY_BONUS_MG[PIECE_TYPES] =
{
    // pawns
    0,
    // knights
    4,
    // bishops  
    3,
    // rooks
    2,
    // queens
    1,
    0
};
static const int MOBILITY_BONUS_EG[PIECE_TYPES] =
{
    // pawns
    0,
    // knights
    3,
    // bishops  
    2,
    // rooks
    1,
    // queens
    1,
    0
};
static const int SAFE_MOBILITY_BONUS_MG[PIECE_TYPES] =
{
    // pawns
    0,
    // knights
    2,
    // bishops  
    2,
    // rooks
    1,
    // queens
    1,
    0
};
static const int SAFE_MOBILITY_BONUS_EG[PIECE_TYPES] =
{
    // pawns
    0,
    // knights
    1,
    // bishops  
    1,
    // rooks
    1,
    // queens
    1,
    0
};
static const int CENTRAL_MOBILITY_BONUS_MG[PIECE_TYPES] =
{
    // pawns
    0,
    // knights
    1,
    // bishops  
    1,
    // rooks
    1,
    // queens
    0,
    0
};
static const int CENTRAL_MOBILITY_BONUS_EG[PIECE_TYPES] =
{
    // pawns
    0,
    // knights
    1,
    // bishops  
    1,
    // rooks
    1,
    // queens
    0,
    0
};
static const int KING_ATTACK_BONUS_MG[PIECE_TYPES] =
{
    // pawns
    0,
    // knights
    6,
    // bishops  
    6,
    // rooks
    3,
    // queens
    2,
    0
};
static const int KING_ATTACK_BONUS_EG[PIECE_TYPES] =
{
    // pawns
    0,
    // knights
    4,
    // bishops  
    4,
    // rooks
    3,
    // queens
    2,
    0
};

static const int PAWN_SPACE_FILE_WEIGHTS[8] =
{
    1,
    1,
    1,
    2,
    2,
    1,
    1,
    1
};

static const int PASSED_PAWN_RANK_BONUS_MG[8] = {
  0,
  0,
  3,
  6,
  9,
  10,
  12,
  0
};
static const int PASSED_PAWN_RANK_BONUS_EG[8] = {
  0,
  0,
  5,
  9,
  12,
  15,
  19,
  0
};


static const int DOUBLED_PAWN_PENALTY_MG = -15;
static const int DOUBLED_PAWN_PENALTY_EG = -12;
static const int ISOLATED_PAWN_PENALTY_MG = -13;
static const int ISOLATED_PAWN_PENALTY_EG = -11;
static const int PASSED_PAWN_BONUS_MG = 8;
static const int PASSED_PAWN_BONUS_EG = 15;
static const int BACKWARD_PAWN_PENALTY_MG = -12;
static const int BACKWARD_PAWN_PENALTY_EG = -9;
static const int CANDIDATE_PASSER_BONUS_MG = 5;
static const int CANDIDATE_PASSER_BONUS_EG = 9;
static const int CHAINED_PAWN_BONUS_MG = 8;
static const int CHAINED_PAWN_BONUS_EG = 8;
static const int BISHOP_PAIR_BONUS_MG = 15;
static const int BISHOP_PAIR_BONUS_EG = 20;
static const int PAWN_OBSTRUCTS_IMPORTANT_SQ = 10;
static const int PAWN_OBSTRUCTS_MOBILITY = 2;
static const int SAFE_PP_TDP_ATTACK = 4;
static const int SAFE_PP_CORRECTION = 2;
static const int PAWN_STORM_BONUS = 8;
static const int SUPPORTED_PAWN = 3;
static const int QUEEN_BISHOP_CONNECTED_BONUS_MG = 6;
static const int QUEEN_BISHOP_CONNECTED_BONUS_EG = 4;
static const int QUEEN_ROOK_CONNECTED_BONUS_MG = 6;
static const int QUEEN_ROOK_CONNECTED_BONUS_EG = 4;
static const int PIN_PT_PENALTY[PIECE_TYPES] =
  {-3, -5, -6, -8, -15};
static const int PIECE_OBSTRUCTS_CASTLE = -4;

static const int ROOK_SEMI_OPEN_FILE_BONUS = 5;
static const int ROOK_OPEN_FILE_BONUS = 7;
static const int CONNECTED_ROOK_BONUS = 6;
static const int SEMI_OPEN_FILE_NEAR_KING_PENALTY = -6;
static const int OPEN_FILE_NEAR_KING_PENALTY = -8;
static const int OPEN_MIDDLE_FILE_NEAR_KING_ADDITIONAL_PENALTY = -2;
static const int OPEN_DIAGONAL_NEAR_KING_PENALTY = -7;
static const int SEMI_OPEN_DIAGONAL_NEAR_KING_PENALTY = -5;
static const int OPEN_DIAGONAL_FROM_KING_PENALTY = -5;
static const int SEMI_OPEN_DIAGONAL_FROM_KING_PENALTY = -6;
static const int DIST_TO_CENTER_BONUS = 8;
static const int ATTACK_TEMPO_EVALUATION = 1;
static const int PAWN_BLOCKERS_PENALTY = -1;
static const int KING_ZONE_ATTACK_PENALTY = -7;
static const int KING_ZONE_DEFENSE_SCORE = 4;
static const int KING_ZONE_BATTERY_BONUS_MG = 8;
static const int KING_ZONE_BATTERY_BONUS_EG = 4;
static const int POTENTIAL_ATTACK_BONUS = 4;
static const int SI_BONUS = 4;
static const int PSI_BONUS = 3;
static const int ATTACKING_WEAK_PAWN = 5;
static const int TDP_PENALTY = -2;

static const int DOUBLE_ATTACK_BONUS_MG = 2;
static const int DOUBLE_ATTACK_BONUS_EG = 2;
static const int PIN_PENALTY = -18;
static const int PIN_ATTACKED_TWICE_PENALTY = -5;
static const int OPEN_FILE_NEAR_KING_WITH_ATTACKER = -8;
static const int OPEN_FILE_FROM_KING_WITH_ATTACKER = -12;
static const int OPEN_DIAGONAL_NEAR_KING_WITH_ATTACKER = -10;
static const int OPEN_DIAGONAL_FROM_KING_WITH_ATTACKER = -14;
static const int KING_SPACE_PENALTY_MG = -4;
static const int KING_SPACE_PENALTY_EG = -2;
static const int SAFE_KING_ATTACK_MG = 5;
static const int SAFE_KING_ATTACK_EG = 4;

static const int DOUBLE_SAFE_KING_ATTACKS = 8;
static const int UNSAFE_CHECK = 0;
static const int N_SAFE_CHECK = 2;
static const int B_SAFE_CHECK = 3;
static const int R_SAFE_CHECK = 6;
static const int Q_SAFE_CHECK = 7;
static const int QUEEN_CONTACT_CHECK = 12;
static const int ROOK_CONTACT_CHECK = 10;
static const int PAWN_SHIELD_BONUS = 5;

static const int KR_DEF_MINOR = 2;
static const int SAFE_CHECK_MG = 3;
static const int SAFE_CHECK_EG = 2;
static const int KNIGHT_CHECK_FORK = 5;
static const int CHECK_MG = 1;
static const int CHECK_EG = 1;
// these are applied twice so they can be relatively low
static const int DOUBLE_KING_ATTACK_BONUS_MG = 4;
static const int DOUBLE_KING_ATTACK_BONUS_EG = 3;
static const int DOUBLE_SAFE_KING_ATTACK_BONUS_MG = 9;
static const int DOUBLE_SAFE_KING_ATTACK_BONUS_EG = 6;
static const int DIAGONAL_KING_ATTACK_THREAT_MG = 3;
static const int DIAGONAL_KING_ATTACK_THREAT_EG = 2;
static const int LATERAL_KING_ATTACK_THREAT_MG = 3;
static const int LATERAL_KING_ATTACK_THREAT_EG = 2;
static const int PAWN_COLOR_PENALTY = 0;
static const int SLIDER_BOXED_IN_PENALTY = -4;
static const int THREAT_DEFICIT_PENALTY = -9;
static const int UNSTOPPABLE_ATTACK_PENALTY = -12;
static const int DEFENDED_PIECE_BONUS = 5;
static const int COLOR_SQUARE_BASE_MG = 3;
static const int COLOR_DEFENDER_ABSENT = 7;
static const int COLOR_SQUARE_END_EG = 5;
static const int PAWN_THREAT_VALUE = 3;

static const int COLOR_SQUARE_BASE_EG = 0;
static const int REFUTED_BY_PAWN_PUSH = -8;

static const int PASSER_DEFICIT_PENALTY_MG = -7;
static const int PASSER_DEFICIT_PENALTY_EG = -11;
static const int ROOK_ON_PASSER_FILE_BONUS_MG = 6;
static const int ROOK_ON_PASSER_FILE_BONUS_EG = 10;
static const int PASSER_SUPPORTED_BONUS_MG = 6;
static const int PASSER_SUPPORTED_BONUS_EG = 9;
static const int ROOK_STUCK_IN_FRONT_OF_PASSER_PENALTY = -12;
static const int PASSER_BLOCKED_MG = -6;
static const int PASSER_BLOCKED_EG = -9;

static const int OPPONENT_PIECE_STUCK_IN_FRONT_OF_PASSER = 12;
static const int OPPONENT_ROOK_STUCK_ON_PROMOTION_RANK = 6;

static const int TDP_WEAK_SQUARE_BONUS = 8;
static const int TDP_OED_PENALTY = -8;

static const int MINOR_CANNOT_ENTER_ENEMY_MG = -5;
static const int MINOR_CANNOT_ENTER_ENEMY_EG = -3;

static const int ATTACKED_BY_PAWN_PENALTY = -8;
static const int OVEREXTENDED_PENALTY_MG = -2;
static const int OVEREXTENDED_PENALTY_EG = -1;

static const int OUTPOST_CONTROLLED_BONUS = 5;

static const uint64_t center_squares_mask = 0x3c3c3c3c0000ULL;

static const int TDP_WEAK_SQ[16] =
{
  8, 16, 24, 31, 39, 46, 53, 59, 65, 71, 76, 81, 86, 90, 94, 97,
}; 
static const int TDP_OED_PEN[16] =
{
  -8, -16, -24, -31, -39, -46, -53, -59, -65, -71, -76, -81, -86, -90, -94, -97,
}; 

static const int PAWN_SHIELD[3] = {5, 11, 18};

extern int THREAT_SCORE[500];

extern int KING_DANGER[1000];

extern int KING_THREAT[1000];
extern int PASSER_SCORE[200];

extern int PSQT_SCALE[10000];

extern int LMR_PV[64][64];
extern int LMR_NON_PV[64][64];
extern int LMP[64];
extern int FP[64][64];
static const int MOBILITY_MG[PIECE_TYPES][32] =
{
  {},
    // knight
  {
    -9,
    -5,
    0,
    5,
    9,
    12,
    15,
    16,
    16,
    16,
    16,
    16,
    16,
    16,
  },
    // bishop
  {
    // -15,
    // -12,
    -9,
    -6,
    -3,
    0,
    3,
    6,
    9,
    12,
    15,
    17,
    20,
    22,
    23,
    25,
    26,
    27,
    27,
    27,
    27,
    27,
    27,
    27,
    27,
    27,
    27,
    27,
    27,
    27,
    27,
    27,
    27,
    27,
  },
    // rook
  {
    -6,
    -5,
    -4,
    -3,
    -1,
    0,
    1,
    3,
    4,
    5,
    6,
    8,
    9,
    10,
    11,
    12,
    13,
    13,
    13,
    13,
    13,
    13,
    13,
    13,
    13,
    13,
    13,
    13,
    13,
    13,
    13,
    13,
  },
    // queen
  {
    -4,
    -4,
    -3,
    -2,
    -1,
    -1,
    0,
    1,
    1,
    2,
    3,
    4,
    4,
    5,
    6,
    6,
    7,
    8,
    8,
    9,
    9,
    10,
    10,
    11,
    11,
    11,
    11,
    11,
    11,
    11,
    11,
    11,
  },
  {}
};
static const int MOBILITY_EG[PIECE_TYPES][32] =
{
  {},
    //knight
  {
    -6,
    -3,
    0,
    3,
    6,
    8,
    10,
    11,
    11,
    11,
    11,
  },
    // bishop
  {
    // -11,
    // -9,
    -7,
    -4,
    -2,
    0,
    2,
    4,
    7,
    9,
    11,
    12,
    14,
    16,
    17,
    18,
    20,
    21,
    21,
    21,
    21,
    21,
    21,
    21,
    21,
    21,
    21,
    21,
    21,
    21,
    21,
    21,
    21,
    21,
  },
    // rook
  {
    -3,
    -2,
    -2,
    -1,
    -1,
    0,
    1,
    1,
    2,
    2,
    3,
    3,
    4,
    4,
    5,
    5,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
  },
    // queen
  {
    -3,
    -2,
    -2,
    -1,
    -1,
    0,
    0,
    0,
    1,
    1,
    2,
    2,
    3,
    3,
    4,
    4,
    5,
    5,
    6,
    6,
    7,
    7,
    7,
    7,
    7,
    7,
    7,
    7,
    7,
    7,
    7,
    7,
  },
  {}
    
};

static const int CONTROLS_OUTPOST_SQUARE_BONUS[PIECE_TYPES] = 
{
  
};
static const int DENIES_OUTPOST_SQUARE_BONUS[PIECE_TYPES] = 
{
  
};
static const int OCCUPIES_OUTPOST_SQUARE_BONUS[PIECE_TYPES] = 
{
  0, 8, 3, 1, 1, 0
};


static const int HANGING_PIECE_BONUS[PIECE_TYPES] =
{
    5, 8, 10, 10, 20, 0
};
static const int STARTING_SQUARE_VALUES[PIECE_TYPES] =
{
    0, -3, -3, 0, 6, 0
};
static const int KING_THREAT_MG[PIECE_TYPES] =
{
    1, 2, 3, 4, 6, 0
};
static const int KING_THREAT_EG[PIECE_TYPES] =
{
    1, 2, 3, 4, 6, 0
};
static const int KING_DEF[PIECE_TYPES] =
{
    0, 1, 1, 2, 3, 0
};
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

// [ATTACKER][VICTIM]
static const int THREAT_VALUES[PIECE_TYPES][PIECE_TYPES] =
{
    
    // pawns
    {
        1, 
        1, 
        2, 
        2, 
        3, 
        4, 
    },
    // knight
    {
        1, 
        1, 
        1, 
        3, 
        5, 
        6
    },
    // bishop
    {
        1, 
        2, 
        2, 
        5, 
        5, 
        6
    },
    // rook
    {
        1, 
        1, 
        1, 
        2, 
        5, 
        6
    },
    // queen
    {
        1, 
        1, 
        1, 
        1, 
        3, 
        7
    },
    // king
    {
        1, 
        1, 
        1, 
        1, 
        1, 
        1
    },
    
};
extern int PSQT_MG[COLOR_MAX][PIECE_TYPES][64];
extern int PSQT_EG[COLOR_MAX][PIECE_TYPES][64];


extern int PAWN_STORM_PSQT_MG[COLOR_MAX][64];
extern int PAWN_STORM_PSQT_EG[COLOR_MAX][64];

extern uint64_t between_sq[64][64];

extern const uint8_t center_manhattan_distance[64];



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
inline int flip_square(int sq) {
    return sq ^ 56;
}


#endif
