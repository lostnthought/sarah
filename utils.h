#ifndef UTILS_H
#define UTILS_H

#include "types.h"


// these are assorted utilities used when parsing fen strings or outputting info to uci etc. all are fairly self explanatory

Piece piece_type_and_color_to_piece(PieceType piece_type, Side side);
Side piece_to_piece_type_and_color(Piece piece, PieceType * piece_type);
int parse_piece(char piece);
int parse_rank(char rank);
int parse_file(char file);
int parse_move(Game * game, char * str, Move * move, PieceType * promo_piece);
// int file_and_rank_to_index(File file, Rank rank);
// void index_to_file_and_rank(int index, File * file, Rank * rank);
void file_and_rank_to_str(File file, Rank rank, char str[]);
void raw_index_to_move(int index, char move[]);
Move * find_move(Move move_list[200], int move_count, int start_pos, int end_pos, bool promotion, PieceType promotion_piece);
void print_move_algebraic(Move * move);
void print_move_full(Move * move);
void print_moves(Move move_list[200], int move_count);
void print_board(uint64_t board, Piece piece);

void print_game_board(Game * game);
void print_psqt(Side side, PieceType piece);
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

#endif
