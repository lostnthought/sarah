#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"


static inline void decay_history_table(Game * game){
    
    for (int i = 0; i < COLOR_MAX; i++){
        for (int f = 0; f < 64; f++){
            for (int t = 0; t < 64; t++){
                game->history_table[i][f][t] >>= 1;
            }
        }
    }
}

/* @brief main iterative deepening search
@param search flags, see types.h */

Move iterative_search(Game * game, SearchFlags * flags);

/* @brief perft recursion
@return node count */

uint64_t perft(Game * game, int depth);

/* @brief perft, benches at 17m nps with hash updates, more without */

void perft_root(Game * game, int depth);

#endif
