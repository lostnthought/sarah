#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "types.h"
#include "utils.h"

/* this file contains all of our different hash tables. they are:
transposition table
pawn eval hash
full eval hash
refutation table
countermove table */



extern const uint64_t Random64[781];
extern uint64_t pawn_random[COLOR_MAX][64];
extern uint64_t king_location_random[COLOR_MAX][3];

extern int polyglot_piece_types[COLOR_MAX][PIECE_TYPES];
extern int polyglot_castle_offsets[COLOR_MAX][CASTLESIDE_MAX];

void init_tt(Game * game);
void reset_tt(Game * game);
void init_pawn_hash(Game * game);
void reset_pawn_hash(Game * game);
void init_eval_table(Game * game);
void reset_eval_table(Game * game);
void reset_countermove_and_refutation_tables(Game * game);

uint64_t create_pawn_hash_from_scratch(Game * game);

static inline size_t pawn_hash_index(uint64_t key){
    return (size_t)(key & PAWN_MASK);
}
static inline size_t tt_hash_index(uint64_t key){
    return (size_t)(key & TT_MASK);
}
static inline size_t eval_hash_index(uint64_t key){
    return (size_t)(key & EVAL_MASK);
}


/* @brief packs moves to 32 bit */
static inline uint32_t pack_move(Move *m){
    uint32_t mv = 0;
    mv |= (m->start_index & 0x3F) << 0; 
    mv |= (m->end_index & 0x3F) << 6; 
    mv |= (m->promotion_type & 0x7) << 12; 
    mv |= (m->type & 0x3) << 15; 
    return mv;
}
/* @brief unpack moves from 32 bit */
static inline void unpack_move(uint32_t mv, Move *out){
    out->start_index = (mv >> 0) & 0x3F;
    out->end_index = (mv >> 6) & 0x3F;
    out->promotion_type = (PieceType)((mv >> 12) & 0x7);
    out->type = (MoveType)((mv >> 15) & 0x3);
}


static inline TTEntry * search_for_tt_entry(Game * game, uint64_t key){
  size_t hashed_index = tt_hash_index(key);
  TTEntry * possible_entry = &game->tt[hashed_index];
  if (key == possible_entry->key){
    // possible_entry->age = 0;
    return possible_entry;
  } else {
    return NULL;
  }
}


static inline PawnHashEntry * search_for_pawn_hash_entry(Game * game, uint64_t key){
  size_t index = pawn_hash_index(key);
  if (key == game->pawn_hash_table[index].key){
    return &game->pawn_hash_table[index];
  } else {
    return NULL;
  }
}

static inline void create_new_pawn_hash_entry(Game * game, uint64_t key, float mg_score, float eg_score, float king_safety, uint64_t w_pawn_attacks, uint64_t b_pawn_attacks){
  
  size_t hashed_index = pawn_hash_index(key);
  PawnHashEntry* entry = &game->pawn_hash_table[hashed_index];
  // uint32_t key32 = (uint32_t)(key >> 32);

  game->pawn_hash_table[hashed_index] = (PawnHashEntry){
    .key = key,
    .mg_score = mg_score,
    .eg_score = eg_score,
    .w_pawn_attacks = w_pawn_attacks,
    .b_pawn_attacks = b_pawn_attacks,
  };

}


void init_opening_book(Game * game, const char * path);

static inline int adjust_mate_score_from_tt(int score, int stored_ply, int current_ply) {
    if (score >= MATE_SCORE - 200 || score <= -MATE_SCORE + 200) {
        if (score > 0) {
            return score - stored_ply + current_ply;
        } else {
            return score + stored_ply - current_ply;
        }
    }
    return score;
}
static inline void create_new_tt_entry(Game * game, uint64_t key, int score, TTType type, int depth, int16_t ply, Move * best_move){
  
  size_t hashed_index = tt_hash_index(key);
  TTEntry * entry = &game->tt[hashed_index];
  bool has_eval = false;
  if (entry->key == key){
    if (entry->depth >= depth) return;
  }
  game->tt[hashed_index] = (TTEntry){
    .key = key,
    .score = score,
    .depth = (int16_t)depth,
    .type = (uint8_t)type,
    .ply = ply,
    .is_opening_book = false,
    // .has_eval = has_eval,
  };
  if (best_move != NULL){
    game->tt[hashed_index].move32 = pack_move(best_move);
    game->tt[hashed_index].has_best_move = true;
  } else {
    game->tt[hashed_index].has_best_move = false;
    game->tt[hashed_index].move32 = 0;
  }

}
static inline EvalEntry * search_for_eval(Game * game, uint64_t key){
  
  size_t hashed_index = eval_hash_index(key);
  EvalEntry * e = &game->eval_table[hashed_index];
  if (e->key == key){
    return e;
  }
  return NULL;
}

static inline void hash_eval(Game * game, uint64_t key, int eval){
  size_t hashed_index = eval_hash_index(key);
  EvalEntry * e = &game->eval_table[hashed_index];

  e->key = key;
  e->eval = eval;
  
}



static inline int sq_from_file_rank(int file, int rank) {
    return rank * 8 + file; 
}

/* @brief polyglot parsing, see bluefever software's tutorial */

static inline Move polyglot_decode(uint16_t m)
{
    int to_file   =  m & 0x7;
    int to_rank   = (m >> 3) & 0x7;
    int from_file = (m >> 6) & 0x7;
    int from_rank = (m >> 9) & 0x7;
    int promo     = (m >> 12) & 0x7;

    Move move;
    move.start_index = file_and_rank_to_index((File)from_file, (Rank)from_rank);
    move.end_index = file_and_rank_to_index((File)to_file, (Rank)to_rank);
    if (promo != 0){
      move.promotion_type = (PieceType)promo;
      move.type = PROMOTION;
    }


    // due to polyglot convention, you MUST change castling moves to king moves
    if (move.start_index == 60 && move.end_index == 63){
      move.type = CASTLE;
      move.castle_side = KINGSIDE;
      move.start_index = 60;
      move.end_index = 62;
    } 
    if (move.start_index == 60 && move.end_index == 56){
      move.type = CASTLE;
      move.castle_side = QUEENSIDE;
      move.start_index = 60;
      move.end_index = 58;
      
    }
    if (move.start_index == 4 && move.end_index == 0){
      move.type = CASTLE;
      move.castle_side = QUEENSIDE;
      move.start_index = 4;
      move.end_index = 2;
      
    }
    if (move.start_index == 4 && move.end_index == 7){
      move.type = CASTLE;
      move.castle_side = QUEENSIDE;
      move.start_index = 4;
      move.end_index = 6;
    }
    return move;
}
static inline void create_new_polyglot_entry(Game * game, uint64_t key, uint16_t weight, Move move){

  size_t hashed_index = tt_hash_index(key);
  TTEntry * entry = &game->tt[hashed_index];


  game->tt[hashed_index] = (TTEntry){
    .key = key,
    .move32 = pack_move(&move),
    .depth = -20,
    .weight = weight,
    .is_opening_book = true,
  };
  
}


static inline uint64_t get_piece_random(PieceType piece, Side side, int index){
  
  // see polyglot format for what this means
  return Random64[(64 * polyglot_piece_types[side][piece]) + (8 * SQ_TO_RANK[index]) + SQ_TO_FILE[index]];
}

static inline uint64_t get_castling_random(Side side, CastleSide castle_side){
  return Random64[CASTLING_OFFSET + polyglot_castle_offsets[side][castle_side]];
}

static inline uint64_t get_en_passant_random(int index){
  return Random64[EN_PASSANT_OFFSET + SQ_TO_FILE[index]];
}
static inline uint64_t get_turn_random(){
  return Random64[TURN_OFFSET];
}

uint64_t create_zobrist_from_scratch(Game * game);
static inline bool check_hash(Game * game){
    uint64_t zob = create_zobrist_from_scratch(game);
    return (zob == game->key);
}

static inline uint32_t mix32(uint32_t x){
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    return x;
}
static inline uint32_t get_countermove(Game * game, Move * m) {
    uint32_t idx = (size_t)(mix32(pack_move(m))) & (COUNTERMOVE_TABLE_SIZE - 1);
    return game->countermove_table[idx];
}

static inline void store_countermove(Game * game, Move * p, Move * m) {
    uint32_t idx = (size_t)(mix32(pack_move(p))) & (COUNTERMOVE_TABLE_SIZE - 1);
    game->countermove_table[idx] = pack_move(m);
}
static inline uint32_t get_refutation(Game * game, Move * m) {
    uint32_t idx = (size_t)(mix32(pack_move(m))) & (REFUTATION_TABLE_SIZE - 1);
    return game->refutation_table[idx];
}

static inline void store_refutation(Game * game, Move * p, Move * m) {
    uint32_t idx = (size_t)(mix32(pack_move(p))) & (REFUTATION_TABLE_SIZE - 1);
    game->refutation_table[idx] = pack_move(m);
}
// to generate randoms for our hash tables
static inline uint64_t splitmix64(uint64_t *state) {
    uint64_t z = (*state += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

#endif
