#ifndef UTILS_H
#define UTILS_H

#include "types.h"


static inline uint8_t move_from(Move m){return m & 0x3F;}
static inline uint8_t move_to(Move m){return (m >> 6) & 0x3F;}
static inline uint8_t move_flags(Move m){return (m >> 12) & 0x3F;}
static inline MoveType move_type(Move m){return (MoveType)(m & (3 << 14));}
static inline PieceType move_promotion_type(Move m) {
    return (PieceType)(((m >> 12) & 3) + 1); 
}
static inline Move create_move(uint8_t from, uint8_t to, MoveType type) {
    return (Move)(from | (to << 6) | type);
}
static inline Move create_promotion(int from, int to, PieceType promo_piece) {
    return (Move)(from | (to << 6) | ((promo_piece - 1) << 12) | PROMOTION);
}

static inline PieceType move_piece(Game * game, Move m){
    return game->piece_at[move_from(m)];
}
static inline PieceType move_cp(Game * game, Move m){
    return game->piece_at[move_to(m)];
}

static inline int from_to(Move m) {
    return m & 0xFFF;
}
static inline bool is_promo(Move m) {
    return move_type(m) == PROMOTION;
}

static inline bool is_ep(Move m) {
    return move_type(m) == ENPASSANT;
}

static inline bool is_castle(Move m) {
    return move_type(m) == CASTLE;
}
static inline bool is_cap(Game * game, Move m){

    if (game->piece_at[move_to(m)] != PIECE_NONE || move_type(m) == ENPASSANT) return true;

    return false;
}

static inline bool more_than_one(uint64_t b) {
    return b & (b - 1);
}
static inline bool is_aligned(uint8_t a, uint8_t b, uint8_t c){
    return between_sq[a][c] & bits[b];
}

#define MG(s) ((int16_t)(s))
#define EG(s) ((int16_t)((s) >> 16))
static inline void dump_score(int32_t s) {
    printf("MG=%d EG=%d\n", (int16_t)s, (int16_t)(s >> 16));
}

static inline Score make_score(int mg, int eg) {
    return (Score)((uint16_t)mg | ((uint32_t)(uint16_t)eg << 16));
}

// these are assorted utilities used when parsing fen strings or outputting info to uci etc. all are fairly self explanatory

Piece piece_type_and_color_to_piece(PieceType piece_type, Side side);
Side piece_to_piece_type_and_color(Piece piece, PieceType * piece_type);
int parse_piece(char piece);
int parse_rank(char rank);
int parse_file(char file);
int parse_move(Game * game, char * str, Move * move, PieceType * promo_piece);

void file_and_rank_to_str(File file, Rank rank, char str[]);
void raw_index_to_move(uint8_t index, char move[]);
Move find_move(Move move_list[256], uint8_t move_count, uint8_t start_pos, uint8_t end_pos, bool promotion, PieceType promotion_piece);
void print_move_algebraic(Side side, Move move);
void get_move_algebraic(Side side, Move move, char mv[24]);
void print_move_full(Move move);
void print_moves(Move move_list[256], uint8_t move_count);
void print_board(uint64_t board, Piece piece);

void print_game_board(Game * game);
void print_psqt(Side side, PieceType piece);
void print_flipped_psqts(Side side);
void print_move_boards(Game * game);

static inline int file_and_rank_to_index(File file, Rank rank){

    // our engine's convention starts black at 0, so we need to flip it since ranks start at 1 for white
    int flipped_rank = abs((int)rank - 7);
    return (flipped_rank * 8) + file;
    
}

static inline void index_to_file_and_rank(int index, File * file, Rank * rank){
    if (!file || !rank) return;
    
    *file = (File)SQ_TO_FILE[index];
    *rank = (Rank)SQ_TO_RANK[index];
}
static inline double now_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}
#endif
