#ifndef MAGIC_H
#define MAGIC_H

#include "types.h"

#define ROOK_TABLE_SIZE 102400

#define BISHOP_TABLE_SIZE 5248


static inline size_t magic_hash_index(const MagicEntry * entry, uint64_t blockers){
    
    return (((blockers & entry->mask) * entry->magic) >> entry->shift) + entry->offset;
    
};

static inline uint64_t fetch_bishop_moves(Game * game, int index, uint64_t blockers){
  return game->bishop_table[magic_hash_index(&BISHOP_MAGICS[index], blockers)];
}
static inline uint64_t fetch_rook_moves(Game * game, int index, uint64_t blockers){
  return game->rook_table[magic_hash_index(&ROOK_MAGICS[index], blockers)];
}
static inline uint64_t fetch_queen_moves(Game * game, int index, uint64_t blockers){
  return game->bishop_table[magic_hash_index(&BISHOP_MAGICS[index], blockers)] | game->rook_table[magic_hash_index(&ROOK_MAGICS[index], blockers)];
}


uint64_t generate_slider_moves(ivec2 slider_deltas[4], int index, uint64_t blockers);
void init_sliding_piece_tables(Game * game);

void init_directional_ray_tables();
#endif
