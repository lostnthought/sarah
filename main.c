#include "game.h"
#include "search.h"
// #include "tests.h"
#include "types.h"
#include "math.h"
#include "move_generation.h"
#include "utils.h"
#include "tuner.h"
#include "zobrist.h"
// #include "./tables/tbconfig.h"
// #include "./tables/tbprobe.h"

Game game;
EngineOptions eo = {
    .debug_info = true,
    .max_depth = 30,
    .time_per_move = 10,
    .threads = 1
};
int running = 0;

int init(){
    
    init_piece_boards(&game);
    init_tt(&game);
    init_pawn_hash(&game);
    init_eval_table(&game);
    init_search_tables();
    init_eval_params("./Sarah_1_0_data.bin");
    reset_countermove_and_refutation_tables(&game);
    reset_corrhist(&game);
    reset_piece_keys(&game);


    // if (tb_init("./tablebase.lichess.ovh/tables/standard/3-4-5-wdl:" "./tablebase.lichess.ovh/tables/standard/3-4-5-dtz")){
    //     printf("TABLEBASES SUCCESSFULLY SETUP\n");
    // } else {
    //     printf("EGTB FAILED\n");
    // }

    // init_opening_book(&game, "./komodo.bin");
    // print_flipped_psqts(WHITE);


    return 1;
}

int main(int argc, char *argv[]) {

    if (!init()){ return -1; } else { running = 1; }


    char fen[MAX_FEN];
    init_new_game(&game, WHITE);


    init_search_params();
    initialize_mobility_tables();
    // run_test_file(&game, "./tests/eigenmann.txt");
    // center_psqts("./tuner/PSQT.bin");
    // new_tuner(&game, "./lichess-big3-resolved.book");
    // new_tuner(&game, "./tuner/E12.46FRC-1250k-D12-1s-Resolved.book");
    // convert_weights("./tuner/Jan21-6.bin");
    // print_flipped_psqts_weights();
    while (running){
        
        handle_input(&game);
        
    }

    
    return 1;
}

