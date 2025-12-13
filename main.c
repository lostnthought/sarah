#include "game.h"
#include "tests.h"
#include "types.h"
#include "math.h"
#include "move_generation.h"
#include "utils.h"
#include "tuner.h"
#include "zobrist.h"

Game game;
int running = 0;

int init(){
    
    init_piece_boards(&game);
    init_tt(&game);
    init_pawn_hash(&game);
    init_eval_table(&game);
    reset_countermove_and_refutation_tables(&game);

    init_opening_book(&game, "./baron30.bin");


    return 1;
}

void cleanup(Game * game){
    
    if (game->bishop_table){
        free(game->bishop_table);
    }
    game->bishop_table = NULL;
    if (game->rook_table){
        free(game->rook_table);
    }
    game->rook_table = NULL;

    if (game->tt){
        free(game->tt);
    }
    game->tt = NULL;
    if (game->pawn_hash_table){
        free(game->pawn_hash_table);
    }
    game->pawn_hash_table = NULL;
    if (game->eval_table){
        free(game->eval_table);
    }
    game->eval_table = NULL;

}

int main(int argc, char *argv[]) {

    if (!init()){ return -1; } else { running = 1; }


    char fen[MAX_FEN];
    init_new_game(&game, WHITE);

    while(running){
        
        handle_input(&game);
        
    }
    
    cleanup(&game);
    return 1;
}

