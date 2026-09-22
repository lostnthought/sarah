#include "game.h"
#include "math.h"
#include "tuner.h"
#include "types.h"
#include "utils.h"
#include <stdio.h>

uint64_t castling_rights_masks[BOARD_MAX];
uint64_t between_sq[64][64];
Score PSQT[COLOR_MAX][PIECE_TYPES][BOARD_MAX] =
{
  // Black
{
  
    // Pawn
  {
    0, 0, 0, 0, 0, 0, 0, 0,
    -1048602, -786448, -1245191, -1048599, -983050, -1179640, -1638388, -1900561,
    -1310747, -1310750, -1638419, -2162698, -1769475, -1703944, -2228227, -2162706,
    -786455, -1114140, -1507342, -2293766, -2097157, -2424828, -1507344, -1703954,
    1507323, 851961, -65537, -1048578, -983024, -851945, -65533, 65539,
    7143460, 7733269, 4980782, 3014701, 2883633, 4063312, 6225966, 6422556,
    2097173, -655313, 1703959, -917465, -720883, -655344, 1834940, 3080088,
    0, 0, 0, 0, 0, 0, 0, 0
  
  },
    // Knight
  {
    786396, 458752, 327668, 327680, 458757, -196601, 917510, 1376249,
    720893, 851970, 458748, 327692, -65520, -131061, 262166, 1114130,
    458756, 393226, 196617, 1048585, 851983, -589809, -65513, 262159,
    1048594, 589844, 1114133, 720923, 983059, 393250, 327705, 524315,
    720910, 327700, 655391, 983083, 1048609, 786470, 786456, 65564,
    196606, 65549, 589836, 589855, 262182, -589782, 131071, -131063,
    458742, 720893, 262145, 458753, -131071, -655337, 458750, -589809,
    -1704023, -262217, 655288, 262101, 786399, -458818, -131143, -2883647
  },
    // Bishop
  {
    -131050, 196636, 262169, 196616, 393229, 655367, -262123, -1507299,
    196628, 131095, -196583, 65547, 262155, -458727, -196581, -851948,
    196620, 131096, 589848, 786449, 983063, 262166, -327657, -655343,
    9, 917514, 983056, 1114135, 655377, 589849, 196619, -524260,
    327683, 458761, 720913, 1310744, 1179672, 720910, 851992, 262140,
    327683, 327697, 458760, 327704, 393226, 1245202, 327686, 589835,
    -589835, -13, 131066, -13, 131065, -65547, 720866, -25,
    -196625, 589780, 327627, 1507225, 851890, 458690, 262090, -196654
  
  },
    // Rook
  {
    8, -65532, 131077, -720882, -786413, -655349, -917486, -720884,
    -131082, -10, -262143, -458756, -983037, -1114101, -1703919, -1114109,
    262131, 262136, -6, -4, -458753, -786425, -1507303, -1245177,
    655354, 983035, 786433, 720896, 655370, 327684, 131089, -65530,
    1179643, 851977, 1376263, 851974, 131086, -131049, 327701, 65553,
    1245174, 1310732, 1310726, 983046, 393241, 458765, 262188, 262162,
    1835003, 2424824, 2621442, 2097164, 2097149, 1703947, 1966086, 1114146,
    1835004, 2818029, 3473379, 3145688, 2752491, 2752499, 2621440, 1835029
  },
    // Queen
  {
    -262160, -327688, 65531, 524293, -393218, -589844, -1572866, -2097146,
    -196601, -131070, -65527, 917511, 720907, -1245174, -3473386, -4259803,
    -786418, 1376266, 1769469, 1507331, 1769472, 983059, -196587, -720869,
    327703, 1638410, 2031622, 2752515, 2359307, 2162701, 1769496, 1310744,
    1769476, 2031622, 1507334, 2883582, 4063230, 2359314, 3211284, 1703965,
    786443, 1638399, 3211261, 3342338, 4063243, 2686996, 1376279, 1638423,
    458759, 2228204, 4063212, 4718565, 5636068, 2949120, 2883587, 1572912,
    -917505, 1114094, 2359292, 2752512, 1900561, 1048602, -2162611, -1245147
  },
    // King
  {
    -7077818, -4063172, -2424798, -1835038, -2490352, -1441809, -3276757, -7208890,
    -3669953, -262113, 917508, 1441770, 1834985, 1245182, -131031, -3080142,
    -3276780, 262164, 2097125, 3080151, 2883550, 1966056, 458759, -1835009,
    -3014658, 1179626, 2752467, 3800998, 3997620, 2490346, 1834976, -1310771,
    -2621442, 1769447, 3538896, 4521872, 3932086, 4063194, 2490354, -655409,
    -1507376, 2293785, 3473367, 4194262, 4194287, 3407945, 3538984, -1114056,
    -2162743, 2228200, 3211195, 2031639, 3276791, 4063218, 3014709, -1048542,
    -9371573, -4325374, -3080167, -1048612, -2097135, -1769422, -1965991, -9961287
  },
},
{
    // Pawn
  {
    0, 0, 0, 0, 0, 0, 0, 0,
    2097173, -655313, 1703959, -917465, -720883, -655344, 1834940, 3080088,
    7143460, 7733269, 4980782, 3014701, 2883633, 4063312, 6225966, 6422556,
    1507323, 851961, -65537, -1048578, -983024, -851945, -65533, 65539,
    -786455, -1114140, -1507342, -2293766, -2097157, -2424828, -1507344, -1703954,
    -1310747, -1310750, -1638419, -2162698, -1769475, -1703944, -2228227, -2162706,
    -1048602, -786448, -1245191, -1048599, -983050, -1179640, -1638388, -1900561,
    0, 0, 0, 0, 0, 0, 0, 0
  },
    // Knight
  {
    -1704023, -262217, 655288, 262101, 786399, -458818, -131143, -2883647,
    458742, 720893, 262145, 458753, -131071, -655337, 458750, -589809,
    196606, 65549, 589836, 589855, 262182, -589782, 131071, -131063,
    720910, 327700, 655391, 983083, 1048609, 786470, 786456, 65564,
    1048594, 589844, 1114133, 720923, 983059, 393250, 327705, 524315,
    458756, 393226, 196617, 1048585, 851983, -589809, -65513, 262159,
    720893, 851970, 458748, 327692, -65520, -131061, 262166, 1114130,
    786396, 458752, 327668, 327680, 458757, -196601, 917510, 1376249
  },
    // Bishop
  {
    -196625, 589780, 327627, 1507225, 851890, 458690, 262090, -196654,
    -589835, -13, 131066, -13, 131065, -65547, 720866, -25,
    327683, 327697, 458760, 327704, 393226, 1245202, 327686, 589835,
    327683, 458761, 720913, 1310744, 1179672, 720910, 851992, 262140,
    9, 917514, 983056, 1114135, 655377, 589849, 196619, -524260,
    196620, 131096, 589848, 786449, 983063, 262166, -327657, -655343,
    196628, 131095, -196583, 65547, 262155, -458727, -196581, -851948,
    -131050, 196636, 262169, 196616, 393229, 655367, -262123, -1507299
  },
    // Rook
  {
    1835004, 2818029, 3473379, 3145688, 2752491, 2752499, 2621440, 1835029,
    1835003, 2424824, 2621442, 2097164, 2097149, 1703947, 1966086, 1114146,
    1245174, 1310732, 1310726, 983046, 393241, 458765, 262188, 262162,
    1179643, 851977, 1376263, 851974, 131086, -131049, 327701, 65553,
    655354, 983035, 786433, 720896, 655370, 327684, 131089, -65530,
    262131, 262136, -6, -4, -458753, -786425, -1507303, -1245177,
    -131082, -10, -262143, -458756, -983037, -1114101, -1703919, -1114109,
    8, -65532, 131077, -720882, -786413, -655349, -917486, -720884,
  },
    // Queen
  {
    -917505, 1114094, 2359292, 2752512, 1900561, 1048602, -2162611, -1245147,
    458759, 2228204, 4063212, 4718565, 5636068, 2949120, 2883587, 1572912,
    786443, 1638399, 3211261, 3342338, 4063243, 2686996, 1376279, 1638423,
    1769476, 2031622, 1507334, 2883582, 4063230, 2359314, 3211284, 1703965,
    327703, 1638410, 2031622, 2752515, 2359307, 2162701, 1769496, 1310744,
    -786418, 1376266, 1769469, 1507331, 1769472, 983059, -196587, -720869,
    -196601, -131070, -65527, 917511, 720907, -1245174, -3473386, -4259803,
    -262160, -327688, 65531, 524293, -393218, -589844, -1572866, -2097146
  },
    // King
  {
    -9371573, -4325374, -3080167, -1048612, -2097135, -1769422, -1965991, -9961287,
    -2162743, 2228200, 3211195, 2031639, 3276791, 4063218, 3014709, -1048542,
    -1507376, 2293785, 3473367, 4194262, 4194287, 3407945, 3538984, -1114056,
    -2621442, 1769447, 3538896, 4521872, 3932086, 4063194, 2490354, -655409,
    -3014658, 1179626, 2752467, 3800998, 3997620, 2490346, 1834976, -1310771,
    -3276780, 262164, 2097125, 3080151, 2883550, 1966056, 458759, -1835009,
    -3669953, -262113, 917508, 1441770, 1834985, 1245182, -131031, -3080142,
    -7077818, -4063172, -2424798, -1835038, -2490352, -1441809, -3276757, -7208890
  },
},
};
int PSQT_MG [COLOR_MAX][PIECE_TYPES][64] = 
{
// black -> filled on init
{

    
},




{
        // pawns
    {
        72, 72, 72, 72, 72, 72, 72, 72,
        105, 104, 106, 106, 108, 105, 102, 100,
        85, 101, 103, 102, 103, 105, 100, 85,
        84, 87, 88, 93, 93, 89, 87, 84,
        80, 83, 83, 83, 83, 83, 82, 81,
        78, 83, 79, 79, 80, 78, 83, 80,
        79, 81, 80, 78, 78, 79, 82, 81,
        72, 72, 72, 72, 72, 72, 72, 72,
     },
        // knights
    {
        326, 326, 326, 331, 331, 328, 324, 326,
        330, 333, 340, 346, 344, 343, 333, 327,
        335, 345, 348, 350, 352, 349, 345, 333,
        333, 338, 340, 343, 343, 342, 339, 333,
        334, 335, 336, 335, 336, 336, 335, 333,
        324, 330, 332, 332, 332, 332, 331, 326,
        326, 321, 328, 329, 330, 327, 324, 326,
        325, 326, 327, 325, 326, 326, 327, 324,
    },
        // bishops
    {
        343, 337, 337, 338, 338, 338, 338, 342,
        341, 348, 347, 346, 348, 348, 347, 342,
        350, 356, 359, 357, 358, 357, 354, 348,
        347, 351, 352, 358, 358, 352, 350, 347,
        346, 346, 351, 350, 351, 349, 347, 344,
        345, 347, 347, 348, 347, 348, 347, 347,
        347, 347, 345, 345, 345, 345, 347, 346,
        346, 342, 343, 342, 341, 343, 341, 345,
    },
    {
        446, 441, 438, 439, 439, 437, 442, 444,
        447, 447, 450, 453, 453, 452, 446, 446,
        444, 450, 450, 456, 456, 453, 452, 445,
        440, 444, 447, 451, 450, 446, 446, 443,
        433, 438, 437, 437, 436, 437, 439, 435,
        429, 438, 436, 433, 434, 434, 441, 433,
        416, 434, 436, 436, 436, 436, 437, 424,
        434, 440, 442, 441, 440, 440, 442, 437,
    },
    {
        927, 926, 928, 929, 928, 928, 925, 926,
        920, 918, 924, 923, 923, 927, 917, 917,
        924, 925, 927, 926, 926, 928, 926, 926,
        921, 923, 923, 921, 920, 922, 923, 922,
        920, 922, 919, 917, 918, 920, 922, 921,
        919, 921, 920, 920, 920, 920, 922, 920,
        916, 919, 922, 920, 921, 921, 919, 915,
        915, 919, 920, 920, 920, 920, 920, 916,
    },
    {
        0, 0, 0, 0, 0, 0, 0, 0,
        -1, 2, 1, 1, 1, 1, 1, -1,
        0, 3, 4, 2, 1, 4, 3, 0,
        -2, 2, 3, 1, 1, 3, 2, -3,
        -5, 1, 3, -1, 1, 4, 2, -4,
        -7, -2, -1, -5, -3, 0, 0, -5,
        5, 0, -3, -12, -10, -7, 3, 7,
        1, 4, -3, -2, -5, -3, 8, 7,
    },
},
};

int PSQT_EG [COLOR_MAX][PIECE_TYPES][64] = 
{
{

    
},
{
    {
        82, 82, 82, 82, 82, 82, 82, 82,
        130, 131, 127, 124, 124, 125, 129, 131,
        126, 123, 120, 118, 117, 118, 123, 125,
        113, 113, 110, 108, 107, 109, 112, 113,
        109, 109, 107, 106, 105, 106, 109, 109,
        108, 109, 107, 107, 107, 107, 108, 108,
        112, 113, 112, 111, 110, 112, 112, 112,
        82, 82, 82, 82, 82, 82, 82, 82,
    },
    {
        231, 237, 247, 246, 245, 246, 236, 229,
        235, 244, 250, 253, 254, 251, 243, 236,
        243, 250, 256, 257, 256, 256, 250, 245,
        244, 254, 259, 261, 261, 257, 255, 245,
        242, 250, 255, 257, 257, 255, 250, 242,
        235, 243, 247, 250, 250, 248, 244, 234,
        231, 238, 240, 244, 244, 240, 237, 230,
        222, 226, 231, 233, 234, 232, 227, 221,
    },
    {
        268, 267, 269, 273, 274, 268, 267, 266,
        263, 273, 274, 273, 273, 274, 273, 263,
        270, 274, 274, 274, 274, 275, 274, 270,
        270, 275, 275, 276, 277, 275, 275, 270,
        266, 271, 275, 275, 275, 275, 271, 267,
        264, 269, 272, 272, 272, 271, 267, 265,
        259, 263, 265, 267, 268, 265, 263, 257,
        261, 259, 260, 259, 261, 260, 260, 259,
    },
    {
        488, 490, 491, 490, 491, 492, 490, 489,
        486, 488, 488, 487, 487, 488, 489, 486,
        486, 484, 486, 483, 483, 485, 483, 486,
        483, 482, 483, 481, 482, 483, 482, 483,
        475, 478, 479, 478, 478, 480, 478, 475,
        467, 470, 470, 470, 470, 471, 470, 467,
        467, 464, 467, 467, 467, 466, 463, 464,
        469, 472, 473, 473, 473, 474, 472, 469,
    },
    {
        897, 898, 899, 906, 907, 899, 896, 897,
        896, 898, 896, 906, 909, 898, 896, 894,
        891, 892, 897, 904, 903, 900, 893, 891,
        892, 899, 898, 909, 911, 899, 899, 894,
        883, 889, 894, 902, 900, 893, 888, 884,
        871, 875, 881, 879, 879, 881, 874, 870,
        865, 866, 864, 871, 871, 863, 865, 863,
        866, 860, 859, 861, 861, 857, 860, 862,
    },
    {
        1, 4, 6, 6, 5, 6, 5, 1,
        -3, 9, 7, 4, 4, 7, 10, -5,
        2, 10, 10, 8, 8, 9, 10, 2,
        0, 6, 8, 9, 8, 8, 6, -1,
        -7, 0, 4, 6, 6, 3, 0, -7,
        -7, -5, -2, 2, 1, -2, -5, -8,
        -9, -6, -4, -3, -4, -3, -7, -9,
        -17, -11, -7, -16, -15, -12, -9, -18,
    },
} 
    
};


int double_pushed_pawn_squares[COLOR_MAX][64];
uint64_t passed_pawn_masks[COLOR_MAX][64];
uint64_t in_front_file_masks[COLOR_MAX][64];
uint64_t pawn_shield_masks[COLOR_MAX][64];
uint64_t in_front_ranks_masks[COLOR_MAX][64];
// basically pawn shield but with an extra row
uint64_t adjacent_in_front_masks[COLOR_MAX][64];
uint64_t king_zone_masks[COLOR_MAX][64];
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

// int start_and_end_pos_to_move(Game * game, int start_index, int end_index, Move * move){

//     if (start_index < 0 || start_index >= 64 || end_index < 0 || end_index >= 64){
//         printf("MOVE OOB\n");
//         return -1;
//     }
//     if (!game || !move){
//         printf("INVALID GAME / MOVE PTR\n");
//         return -1;
//     }
//     // PieceType piece = 0;
//     bool found_piece = false;
//     move->type = MOVE;

//     for (int c = 0; c < COLOR_MAX; c++){
//         for (int i = 0; i < PIECE_TYPES; i++){
//             if (check_bit(game->pieces[c][i], start_index)){

//                 found_piece = true;
//                 move->side = c;
//                 move->piece = i;

//             }
//             if (check_bit(game->pieces[c][i], end_index)){

//                 move->capture_piece = i;
//                 move->type = CAPTURE;
                
//             }
            
//         }
//     }
//     if (!found_piece){
//         printf("NO PIECE FOUND AT START\n");
//         return -1;
//     }

//     if (move->piece == KING){

//         if (move->side == BLACK){
//             if (move->start_index == 4 && game->castle_flags[BLACK][QUEENSIDE]){
//                 if (move->end_index == 2){
//                     move->castle_side = QUEENSIDE;
//                     move->type = CASTLE;
//                 }
//             } else if (move->start_index == 4 && game->castle_flags[BLACK][KINGSIDE]){
//                 if (move->end_index == 6){
//                     move->castle_side = KINGSIDE;
//                     move->type = CASTLE;
//                 }
                
//             }
            
//         } else {
            
//             if (move->start_index == 60 && game->castle_flags[WHITE][QUEENSIDE]){
//                 if (move->end_index == 62){
//                     move->castle_side = QUEENSIDE;
//                     move->type = CASTLE;
//                 }
//             } else if (move->start_index == 60 && game->castle_flags[WHITE][KINGSIDE]){
//                 if (move->end_index == 58){
//                     move->castle_side = KINGSIDE;
//                     move->type = CASTLE;
//                 }
                
//             }
//         }
        
//     }

//     if (move->piece == PAWN){
//         if (game->en_passant_index == end_index){
//             move->type = EN_PASSANT;
//             move->capture_piece = PAWN;
//         }
//     }

    
//     return 1;
// }
void print_moves(Move move_list[256], uint8_t move_count){

    printf("MOVE COUNT: %d\n", move_count);
    for (int i = 0; i < move_count; i++){
        Move move = move_list[i];
        print_move_full(move);
    }
    
}

// returns a pointer to a found move using start and end pos, takes an additional argument for promotion piece

Move find_move(Move move_list[256], uint8_t move_count, uint8_t start_pos, uint8_t end_pos, bool promotion, PieceType promotion_piece){

    for (int i = 0; i < move_count; i++){
        if (move_from(move_list[i]) == start_pos && move_to(move_list[i]) == end_pos){
            if (promotion && move_type(move_list[i]) == PROMOTION){
                if (move_promotion_type(move_list[i]) == promotion_piece){
                    return move_list[i];
                }
            } else {
                return move_list[i];
            }
        }
    }
    return 0;
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
    *move = create_move(start_index, end_index, NORMAL);

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

void raw_index_to_move(uint8_t index, char move[]){

    File file = 0;
    Rank rank = 0;
    index_to_file_and_rank(index, &file, &rank);
    file_and_rank_to_str(file, rank, move);
    
}

void print_move_algebraic(Side side, Move move){
    
    char move_start[24];
    char move_end[24];
    raw_index_to_move(move_from(move), move_start);
    raw_index_to_move(move_to(move), move_end);
    printf("%s%s", move_start, move_end);
    if (move_type(move) == PROMOTION){
        printf("%c", piece_names[piece_type_and_color_to_piece(move_promotion_type(move), side)]);
    }
}
void get_move_algebraic(Side side, Move move, char mv[24]){
    
    char move_start[24];
    char move_end[24];
    raw_index_to_move(move_from(move), move_start);
    raw_index_to_move(move_to(move), move_end);
    sprintf(mv, "%s%s", move_start, move_end);
    if (move_type(move) == PROMOTION){
        int l = strlen(mv);
        mv[l] = piece_names[piece_type_and_color_to_piece(move_promotion_type(move), side)];
        mv[l+1] = '\0';
    }
}

void print_move_full(Move move){

    char move_start[24];
    char move_end[24];
    raw_index_to_move(move_from(move), move_start);
    raw_index_to_move(move_to(move), move_end);
    // printf("MOVE %d %d %s\n", move->start_index, move->end_index, move_types[move->type]);
    // printf("MOVE %c %s %s %s %c %d\n", piece_names[piece_type_and_color_to_piece(move->piece, move->side)], move_start, move_end, move_types[move->type], piece_names[move->promotion_type], move->score);
    printf("%s%s %s\n", move_start, move_end, move_types[move_type(move)]);

    
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

void print_flipped_psqt(Side side, PieceType piece){
    
    for (int r = 0; r < 8; r++){
        for (int f = 0; f < 8; f++){
            printf("%d, ", PSQT_MG[side][piece][abs(r - 7) * 8 + f]);
        }
        printf("\n");
    }
    for (int r = 0; r < 8; r++){
        for (int f = 0; f < 8; f++){
            printf("%d, ", PSQT_EG[side][piece][abs(r - 7) * 8 + f]);
        }
        printf("\n");
    }
}
void print_flipped_psqts(Side side){

    print_flipped_psqt(side, PAWN);
    print_flipped_psqt(side, KNIGHT);
    print_flipped_psqt(side, BISHOP);
    print_flipped_psqt(side, ROOK);
    print_flipped_psqt(side, QUEEN);
    print_flipped_psqt(side, KING);
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

