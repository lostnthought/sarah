#include "math.h"
#include "tuner.h"
#include "types.h"
#include "utils.h"
#include <stdio.h>






int double_pushed_pawn_squares[COLOR_MAX][64];
uint64_t passed_pawn_masks[COLOR_MAX][64];
uint64_t in_front_file_masks[COLOR_MAX][64];
uint64_t pawn_shield_masks[COLOR_MAX][64];
uint64_t in_front_ranks_masks[COLOR_MAX][64];
// basically pawn shield but with an extra row
uint64_t adjacent_in_front_masks[COLOR_MAX][64];
uint64_t king_zone_masks[64];
uint64_t DIAGONAL_RAYS[4][64];
uint64_t LATERAL_RAYS[4][64];
int SQ_TO_FILE[64];
int SQ_TO_RANK[64];

int push_direction[COLOR_MAX] = {1, -1};

double * weights_mg = NULL;
double * weights_eg = NULL;
ParamIndex params;


// EVAL CONSTANTS


// from - to
uint8_t manhattan_distance[64][64];


// these are the psqts that i based mine off of. mine are currently in types.h. these tables are lifted straight from lithander's minimal chess, which was very helpful in building my engine!
// https://github.com/lithander/MinimalChessEngine

int OLD_PSQT_MG [COLOR_MAX][PIECE_TYPES][64] = 
{
// black -> filled on init
{},
{
        // pawns
    {
      100,  100,  100,  100,  100,  100,  100,  100,
      176,  214,  147,  194,  189,  214,  132,   77,
       82,   88,  106,  113,  150,  146,  110,   73,
       67,   93,   83,   95,   97,   92,   99,   63,
       55,   74,   80,   89,   94,   86,   90,   55,
       55,   70,   68,   69,   76,   81,  101,   66,
       52,   84,   66,   60,   69,   99,  117,   60,
      100,  100,  100,  100,  100,  100,  100,  100,
     },
        // knights
    {
      116,  228,  271,  270,  338,  213,  278,  191,
      225,  247,  353,  331,  321,  360,  300,  281,
      258,  354,  343,  362,  389,  428,  375,  347,
      300,  332,  325,  360,  349,  379,  339,  333,
      298,  322,  325,  321,  337,  332,  332,  303,
      287,  297,  316,  319,  327,  320,  327,  294,
      276,  259,  300,  304,  308,  322,  296,  292,
      208,  286,  257,  274,  296,  284,  288,  284,
    },
        // bishops
    {
      292,  338,  254,  283,  299,  294,  337,  323,
      316,  342,  319,  319,  360,  385,  343,  295,
      342,  377,  373,  374,  368,  392,  385,  363,
      332,  338,  356,  384,  370,  380,  337,  341,
      327,  354,  353,  366,  373,  346,  345,  341,
      335,  350,  351,  347,  352,  361,  350,  344,
      333,  354,  354,  339,  344,  353,  367,  333,
      309,  341,  342,  325,  334,  332,  302,  313,
    },
    {
      493,  511,  487,  515,  514,  483,  485,  495,
      493,  498,  529,  534,  546,  544,  483,  508,
      465,  490,  499,  497,  483,  519,  531,  480,
      448,  464,  476,  495,  484,  506,  467,  455,
      442,  451,  468,  470,  476,  472,  498,  454,
      441,  461,  468,  465,  478,  481,  478,  452,
      443,  472,  467,  476,  483,  500,  487,  423,
      459,  463,  470,  479,  480,  480,  446,  458,
    },
    {
      865,  902,  922,  911,  964,  948,  933,  928,
      886,  865,  903,  921,  888,  951,  923,  940,
      902,  901,  907,  919,  936,  978,  965,  966,
      881,  885,  897,  894,  898,  929,  906,  915,
      907,  884,  899,  896,  904,  906,  912,  911,
      895,  916,  900,  902,  904,  912,  924,  917,
      874,  899,  918,  908,  915,  924,  911,  906,
      906,  899,  906,  922,  898,  890,  878,  858,
    },
    {
      -11,   70,   55,   31,  -37,  -16,   22,   22,
       37,   24,   25,   36,   16,    8,  -12,  -31,
       33,   26,   42,   11,   11,   40,   35,   -2,
        0,   -9,    1,  -21,  -20,  -22,  -15,  -60,
      -25,   16,  -27,  -67,  -81,  -58,  -40,  -62,
        7,   -2,  -37,  -77,  -79,  -60,  -23,  -26,
       12,   15,  -13,  -72,  -56,  -28,   15,   17,
       -6,   44,   29,  -58,    8,  -25,   34,   28,
    },
},
};

int OLD_PSQT_EG [COLOR_MAX][PIECE_TYPES][64] = 
{
{},
{
    {
      100,  100,  100,  100,  100,  100,  100,  100,
      277,  270,  252,  229,  240,  233,  264,  285,
      190,  197,  182,  168,  155,  150,  180,  181,
      128,  117,  108,  102,   93,  100,  110,  110,
      107,  101,   89,   85,   86,   83,   92,   91,
       96,   96,   85,   92,   88,   83,   85,   82,
      107,   99,   97,   97,  100,   89,   89,   84,
      100,  100,  100,  100,  100,  100,  100,  100,
    },
    {
      229,  236,  269,  250,  257,  249,  219,  188,
      252,  274,  263,  281,  273,  258,  260,  229,
      253,  264,  290,  289,  278,  275,  263,  243,
      267,  280,  299,  301,  299,  293,  285,  264,
      263,  273,  293,  301,  296,  293,  284,  261,
      258,  276,  278,  290,  287,  274,  260,  255,
      241,  259,  270,  277,  276,  262,  260,  237,
      253,  233,  258,  264,  261,  260,  234,  215,
    },
    {
      288,  278,  287,  292,  293,  290,  287,  277,
      289,  294,  301,  288,  296,  289,  294,  281,
      292,  289,  296,  292,  296,  300,  296,  293,
      293,  302,  305,  305,  306,  302,  296,  297,
      289,  293,  304,  308,  298,  301,  291,  288,
      285,  294,  304,  303,  306,  294,  290,  280,
      285,  284,  291,  299,  300,  290,  284,  271,
      277,  292,  286,  295,  294,  288,  290,  285,
    },
    {
      506,  500,  508,  502,  504,  507,  505,  503,
      505,  506,  502,  502,  491,  497,  506,  501,
      504,  503,  499,  500,  500,  495,  496,  496,
      503,  502,  510,  500,  502,  504,  500,  505,
      505,  509,  509,  506,  504,  503,  496,  495,
      500,  503,  500,  505,  498,  498,  499,  489,
      496,  495,  502,  505,  498,  498,  491,  499,
      492,  497,  498,  496,  493,  493,  497,  480,
    },
    {
      918,  937,  943,  945,  934,  926,  924,  942,
      907,  945,  946,  951,  982,  933,  928,  912,
      896,  921,  926,  967,  963,  937,  924,  915,
      926,  944,  939,  962,  983,  957,  981,  950,
      893,  949,  942,  970,  952,  956,  953,  936,
      911,  892,  933,  928,  934,  942,  934,  924,
      907,  898,  883,  903,  903,  893,  886,  888,
      886,  887,  890,  872,  916,  890,  906,  879,
    },
    {
      -74,  -43,  -20,  -10,  -5,   10,    1,  -12,
      -18,    6,    4,    9,    7,   26,   14,    8,
       -3,    6,   10,    6,    8,   24,   27,    3,
      -16,    8,   13,   20,   14,   19,   10,   -3,
      -25,  -14,   13,   20,   24,   15,    1,  -15,
      -27,  -10,    9,   20,   23,   14,    2,  -12,
      -32,  -17,    4,   14,   15,    5,  -10,  -22,
      -55,  -40,  -23,   -6,  -20,   -8,  -28,  -47,
    }
} 
    
};







uint64_t castle_side_board_halves[CASTLESIDE_MAX] = 
{
    0xf0f0f0f0f0f0f0fULL,
    0xe0e0e0e0e0e0e0e0ULL
};

int PAWN_STORM_PSQT_MG[COLOR_MAX][64] = 
{

      0,  0,  0,  0,  0,  0,  0,  0,
      12,  12,  12,  10,  10,  12,  12,  12,
      8,  8,  7,  5,  3,  6,  8,  8,
      7,  7,  8,  4,   2,  5,  6,  7,
      6,  6,   4,   4,   3,   5,   6,   6,
       3,   3,   4,  2,   2,   3,   3,   3,
      -5,   -5,   -5,   -5,  -5,  -5,  -5,   -5,
      0,  0,  0,  0,  0,  0,  0,  0,
    
};
int PAWN_STORM_PSQT_EG[COLOR_MAX][64] = 
{

      0,  0,  0,  0,  0,  0,  0,  0,
      12,  12,  12,  10,  10,  12,  12,  12,
      8,  8,  7,  5,  6,  6,  8,  8,
      7,  7,  6,  6,   6,  5,  6,  5,
      4,  4,   5,   2,   2,   3,   4,   3,
       3,   3,   4,  2,   2,   3,   3,   3,
      -5,   -5,   -5,   -5,  -5,  -5,  -5,   -5,
      0,  0,  0,  0,  0,  0,  0,  0,
    
};


// int KING_THREAT_VALUES[PIECE_TYPES] = 
// {
//     0.05,
//     0.075,
//     0.1,
//     0.2,
//     0.3
// };





int MOBILITY_BLOCKER_VALUES[2][PIECE_TYPES][PIECE_TYPES] =
{
    // friendly pieces
    {
        // pawns
        {
            10, // pxp
            1, // pxk
            12, // pxb
            2, // pxr
            2, // pxq
            1, 
        },
        // knight
        {
            6, //kxp
            -4, //kxk
            8, //kxb
            4, //kxr
            4, //kxq
            2
        },
        // bishop
        {
            2, //bxp
            -2, //bxk
            2, //bxb
            6, //bxr
            12, //bxq
            2
        },
        // rook
        {
            3, //rxp
            2, //rxk
            5, //rxb
            8, //rxr
            12, //rxq
            3
        },
        // queen
        {
            2, //qxp
            -2, //qxk
            2, //qxb
            5, //qxr
            4, //qxq
            3
        },
        // king
        {
            -5, //kxp
            3, //kxk
            -3, //kxb
            -1, //kxr
            -4, //kxq
            0
        },
    
    },
    // enemy pieces
    {
    
        // pawns
        {
            2, // pxp
            10, // pxk
            10, // pxb
            13, // pxr
            20, // pxq
            15, 
        },
        // knight
        {
            5, //kxp
            5, //kxk
            9, //kxb
            10, //kxr
            15, //kxq
            25
        },
        // bishop
        {
            4, //bxp
            5, //bxk
            3, //bxb
            10, //bxr
            10, //bxq
            20
        },
        // rook
        {
            3, //rxp
            3, //rxk
            5, //rxb
            5, //rxr
            15, //rxq
            25
        },
        // queen
        {
            2, //qxp
            2, //qxk
            2, //qxb
            5, //qxr
            5, //qxq
            20
        },
        // king
        {
            3, //kxp
            3, //kxk
            -3, //kxb
            -20, //kxr
            -50, //kxq
            0
        },
    
    }
};






Piece piece_type_and_color_to_piece(PieceType piece_type, Side side){
    switch(side){
        case BLACK:
            switch(piece_type){
                case PAWN:
                    return BLACK_PAWN;
                    break;
                case KNIGHT:
                    return BLACK_KNIGHT;
                    break;
                case BISHOP:
                    return BLACK_BISHOP;
                    break;
                case ROOK:
                    return BLACK_ROOK;
                    break;
                case QUEEN:
                    return BLACK_QUEEN;
                    break;
                case KING:
                    return BLACK_KING;
                    break;
            }
            break;
        case WHITE:
            switch(piece_type){
                case PAWN:
                    return WHITE_PAWN;
                    break;
                case KNIGHT:
                    return WHITE_KNIGHT;
                    break;
                case BISHOP:
                    return WHITE_BISHOP;
                    break;
                case ROOK:
                    return WHITE_ROOK;
                    break;
                case QUEEN:
                    return WHITE_QUEEN;
                    break;
                case KING:
                    return WHITE_KING;
                    break;
            }
            break;
        default:
            return 0;
            break;
    }
    
}


// unrolls a piece enum to it's piece type and color separately
// returns side
Side piece_to_piece_type_and_color(Piece piece, PieceType * piece_type){
    

    switch(piece){
        
        case BLACK_PAWN:
            if (!piece_type){
                return BLACK;
            }
            *piece_type = PAWN;
            return BLACK;
            break;
        case WHITE_PAWN:
            if (!piece_type){
                return WHITE;
            }
            *piece_type = PAWN;
            return WHITE;
            break;
        case BLACK_KNIGHT:
            if (!piece_type){
                return BLACK;
            }
            *piece_type = KNIGHT;
            return BLACK;
            break;
        case WHITE_KNIGHT:
            if (!piece_type){
                return WHITE;
            }
            *piece_type = KNIGHT;
            return WHITE;
            break;
        case BLACK_BISHOP:
            if (!piece_type){
                return BLACK;
            }
            *piece_type = BISHOP;
            return BLACK;
            break;
        case WHITE_BISHOP:
            if (!piece_type){
                return WHITE;
            }
            *piece_type = BISHOP;
            return WHITE;
            break;
        case BLACK_ROOK:
            if (!piece_type){
                return BLACK;
            }
            *piece_type = ROOK;
            return BLACK;
            break;
        case WHITE_ROOK:
            if (!piece_type){
                return WHITE;
            }
            *piece_type = ROOK;
            return WHITE;
            break;
        case BLACK_QUEEN:
            if (!piece_type){
                return BLACK;
            }
            *piece_type = QUEEN;
            return BLACK;
            break;
        case WHITE_QUEEN:
            if (!piece_type){
                return WHITE;
            }
            *piece_type = QUEEN;
            return WHITE;
            break;
        case BLACK_KING:
            if (!piece_type){
                return BLACK;
            }
            *piece_type = KING;
            return BLACK;
            break;
        case WHITE_KING:
            if (!piece_type){
                return WHITE;
            }
            *piece_type = KING;
            return WHITE;
            break;
    }
}


int parse_piece(char piece){

        
    switch(piece){
        case 'p':
            return BLACK_PAWN;
            break;
        case 'P':
            return WHITE_PAWN;
            break;
        case 'n':
            return BLACK_KNIGHT;
            break;
        case 'N':
            return WHITE_KNIGHT;
            break;
        case 'b':
            return BLACK_BISHOP;
            break;
        case 'B':
            return WHITE_BISHOP;
            break;
        case 'r':
            return BLACK_ROOK;
            break;
        case 'R':
            return WHITE_ROOK;
            break;
        case 'q':
            return BLACK_QUEEN;
            break;
        case 'Q':
            return WHITE_QUEEN;
            break;
        case 'k':
            return BLACK_KING;
            break;
        case 'K':
            return WHITE_KING;
            break;
        default:
            return -1;
            break;
    }

}

int parse_rank(char rank){
    if (!isdigit(rank)) return -1;
    return rank - '1';
}

int parse_file(char file){
    
    switch(file){
        case 'a':
            return FILE_A;
            break;
        case 'A':
            return FILE_A;
            break;
        case 'b':
            return FILE_B;
            break;
        case 'B':
            return FILE_B;
            break;
        case 'c':
            return FILE_C;
            break;
        case 'C':
            return FILE_C;
            break;
        case 'd':
            return FILE_D;
            break;
        case 'D':
            return FILE_D;
            break;
        case 'e':
            return FILE_E;
            break;
        case 'E':
            return FILE_E;
            break;
        case 'f':
            return FILE_F;
            break;
        case 'F':
            return FILE_F;
            break;
        case 'g':
            return FILE_G;
            break;
        case 'G':
            return FILE_G;
            break;
        case 'h':
            return FILE_H;
            break;
        case 'H':
            return FILE_H;
            break;
        default:
            printf("INCORRECT FILE CHARACTER DETECTED, RETURNING -1\n");
            return -1;
            break;
    }
}

int start_and_end_pos_to_move(Game * game, int start_index, int end_index, Move * move){

    if (start_index < 0 || start_index >= 64 || end_index < 0 || end_index >= 64){
        printf("MOVE OOB\n");
        return -1;
    }
    if (!game || !move){
        printf("INVALID GAME / MOVE PTR\n");
        return -1;
    }
    // PieceType piece = 0;
    bool found_piece = false;
    move->type = MOVE;

    for (int c = 0; c < COLOR_MAX; c++){
        for (int i = 0; i < PIECE_TYPES; i++){
            if (check_bit(game->pieces[c][i], start_index)){

                found_piece = true;
                move->side = c;
                move->piece = i;

            }
            if (check_bit(game->pieces[c][i], end_index)){

                move->capture_piece = i;
                move->type = CAPTURE;
                
            }
            
        }
    }
    if (!found_piece){
        printf("NO PIECE FOUND AT START\n");
        return -1;
    }

    if (move->piece == KING){

        if (move->side == BLACK){
            if (move->start_index == 4 && game->castle_flags[BLACK][QUEENSIDE]){
                if (move->end_index == 2){
                    move->castle_side = QUEENSIDE;
                    move->type = CASTLE;
                }
            } else if (move->start_index == 4 && game->castle_flags[BLACK][KINGSIDE]){
                if (move->end_index == 6){
                    move->castle_side = KINGSIDE;
                    move->type = CASTLE;
                }
                
            }
            
        } else {
            
            if (move->start_index == 60 && game->castle_flags[WHITE][QUEENSIDE]){
                if (move->end_index == 62){
                    move->castle_side = QUEENSIDE;
                    move->type = CASTLE;
                }
            } else if (move->start_index == 60 && game->castle_flags[WHITE][KINGSIDE]){
                if (move->end_index == 58){
                    move->castle_side = KINGSIDE;
                    move->type = CASTLE;
                }
                
            }
        }
        
    }

    if (move->piece == PAWN){
        if (game->en_passant_index == end_index){
            move->type = EN_PASSANT;
            move->capture_piece = PAWN;
        }
    }

    
    return 1;
}
void print_moves(Move move_list[200], int move_count){

    printf("MOVE COUNT: %d\n", move_count);
    for (int i = 0; i < move_count; i++){
        // print out moves
        Move * move = &move_list[i];
        print_move_full(move);
    }
    
}

// returns a pointer to a found move using start and end pos, takes an additional argument for promotion piece

Move * find_move(Move move_list[200], int move_count, int start_pos, int end_pos, bool promotion, PieceType promotion_piece){

    for (int i = 0; i < move_count; i++){
        if (move_list[i].start_index == start_pos && move_list[i].end_index == end_pos){
            if (promotion && move_list[i].type == PROMOTION){
                if (move_list[i].promotion_type == promotion_piece){
                    return &move_list[i];
                }
            } else {
                return &move_list[i];
            }
        }
    }
    return NULL;
}
    

int parse_move(Game * game, char * str, Move * move, PieceType * promo_piece){
    
    if (strlen(str) < 4) {
        return -1;
    };
    
    File start_file = parse_file(str[0]);
    // printf("PARSING FILE %c to FILE %c\n", str[0], file_names[start_file]);
    Rank start_rank = parse_rank(str[1]);
    File end_file = parse_file(str[2]);
    Rank end_rank = parse_rank(str[3]);
    Piece promotion_piece = 0;
    bool promotion = false;
    if (strlen(str) >= 5){
        promotion_piece = parse_piece(str[4]);
        promotion = true;
        if (promo_piece){
            piece_to_piece_type_and_color(promotion_piece, promo_piece);
        }
    }

    int start_index = file_and_rank_to_index(start_file, start_rank);
    int end_index = file_and_rank_to_index(end_file, end_rank);
    move->start_index = start_index;
    move->end_index = end_index;

    if (promotion){
        return 2;
    }
    
    return 1;
    
}


void file_and_rank_to_str(File file, Rank rank, char str[]){
    str[0] = file_names[file];
    str[1] = rank_names[rank];
    str[2] = '\0';
}

void raw_index_to_move(int index, char move[]){

    File file = 0;
    Rank rank = 0;
    index_to_file_and_rank(index, &file, &rank);
    file_and_rank_to_str(file, rank, move);
    
}

void print_move_algebraic(Move * move){
    
    char move_start[24];
    char move_end[24];
    raw_index_to_move(move->start_index, move_start);
    raw_index_to_move(move->end_index, move_end);
    printf("%s%s", move_start, move_end);
    if (move->type == PROMOTION){
        printf("%c", piece_names[piece_type_and_color_to_piece(move->promotion_type, move->side)]);
    }
}

void print_move_full(Move * move){

    char move_start[24];
    char move_end[24];
    raw_index_to_move(move->start_index, move_start);
    raw_index_to_move(move->end_index, move_end);
    // printf("MOVE %d %d %s\n", move->start_index, move->end_index, move_types[move->type]);
    // printf("MOVE %c %s %s %s %c %d\n", piece_names[piece_type_and_color_to_piece(move->piece, move->side)], move_start, move_end, move_types[move->type], piece_names[move->promotion_type], move->score);
    printf("%s%s %s %d\n", move_start, move_end, move_types[move->type], move->score);

    
}



// void print_move_boards(Game * game){

//     printf("PAWN MOVES\n");
//     for (int i = 0; i < BOARD_MAX; i++){
//         printf("%luULL,\n",game->pawn_moves[BLACK][i]);
//     }
//     printf("\n");
//     for (int i = 0; i < BOARD_MAX; i++){
//         printf("%luULL,\n",game->pawn_moves[WHITE][i]);
//     }
//     printf("\n");
    
//     printf("PAWN CAP\n");
//     for (int i = 0; i < BOARD_MAX; i++){
//         printf("%luULL,\n",game->pawn_captures[BLACK][i]);
//     }
//     printf("\n");
//     for (int i = 0; i < BOARD_MAX; i++){
//         printf("%luULL,\n",game->pawn_captures[WHITE][i]);
//     }
//     printf("\n");

//     printf("KNIGHT MOVES\n");
//     for (int i = 0; i < BOARD_MAX; i++){
//         printf("%luULL,\n",game->knight_moves[i]);
//     }
//     printf("\n");

//     printf("KING MOVES\n");
//     for (int i = 0; i < BOARD_MAX; i++){
//         printf("%luULL,\n",game->king_moves[i]);
//     }
//     printf("\n");

//     printf("BISHOP MOVES\n");
//     for (int i = 0; i < BOARD_MAX; i++){
//         printf("%luULL,\n",game->bishop_moves[i]);
//     }
//     printf("\n");
//     printf("ROOK MOVES\n");
//     for (int i = 0; i < BOARD_MAX; i++){
//         printf("%luULL,\n",game->rook_moves[i]);
//     }
//     printf("\n");
    
// }


void print_psqt(Side side, PieceType piece){

    for (int i = 0; i < BOARD_MAX; i++){
        if (i % 8 == 0){
            printf("\n");
        }
            printf("%d, ", PSQT_MG[side][piece][i]);
    }
    printf("\n");
    for (int i = 0; i < BOARD_MAX; i++){
        if (i % 8 == 0){
            printf("\n");
        }
            printf("%d, ", PSQT_EG[side][piece][i]);
    }
    printf("\n");
}


void print_board(uint64_t board, Piece piece){

    for (int i = 0; i < BOARD_MAX; i++){
        if (i % 8 == 0){
            printf("\n");
        }
        if (check_bit(board, i)){
            
            printf("%c ", piece_names[piece]);
        } else {
            printf("- ");
        }
    }
    printf("\n");
}

void print_game_board(Game * game){

    printf("%c ", rank_names[7]);
    for (int i = 0; i < BOARD_MAX; i++){
        if (i % 8 == 0 && i != 0){
            printf("\n%c ", rank_names[abs((i / 8) - 7)]);
        }

        bool no_piece = true;
        for (int c = 0; c < COLOR_MAX; c++){
            for (int p = 0; p < PIECE_TYPES; p++){
                if(check_bit(game->pieces[c][p], i)){
                    printf("%c ", piece_names[piece_type_and_color_to_piece(p, c)]);

                    no_piece = false;
                }
            }
        }
        if (no_piece){
            printf("- ");
        }
    }

    printf("\n  ");

    for (int i = 0; i < 8; i++){
        printf("%c ", file_names[i]);
    }
    printf("\n");
}

