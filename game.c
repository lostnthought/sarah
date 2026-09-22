#include "game.h"
#include "math.h"
#include "move_generation.h"
#include "types.h"
#include "utils.h"
#include "magic.h"
#include "zobrist.h"
#include <string.h>
#include "search.h"
#include "eval.h"
#include "uci.h"






void output_game_to_fen(Game * game, char fen[MAX_FEN]){
    
    int current_char = 0;
    int empty_space_counter = 0;
    for (int i = 0; i < BOARD_MAX; i++){
        if (game->board_pieces[BLACK] & (1ULL << i) ||
        game->board_pieces[WHITE] & (1ULL << i)){
            bool piece_at_location = false;
            if (empty_space_counter > 0){
                fen[current_char] = '0' + empty_space_counter;
                current_char += 1;
            }
            empty_space_counter = 0;
            for (int c = 0; c < COLOR_MAX; c++){
                for (int p = 0; p < PIECE_TYPES; p++){
                    if ((1ULL << i) & game->pieces[c][p]){
                        fen[current_char] = piece_names[piece_type_and_color_to_piece(p, c)];
                        current_char += 1;
                        piece_at_location = true;
                    }
                }
            }
            if (!piece_at_location){
                // printf("ERROR: BOARDS UNSYNCED\n");
                // printf("PIECE BOARD WHITE:\n");
                // print_board(game->board_pieces[WHITE], WHITE_KING);
                // for (int i = 0; i < PIECE_TYPES; i++){
                //     printf("PIECE: %c", piece_names[piece_type_and_color_to_piece(i, WHITE)]);
                //     print_board(game->pieces[WHITE][i], piece_type_and_color_to_piece(i, WHITE));
                // }
                // printf("PIECE BOARD BLACK:\n");
                // print_board(game->board_pieces[BLACK], BLACK_KING);
                // for (int i = 0; i < PIECE_TYPES; i++){
                //     printf("PIECE: %c", piece_names[piece_type_and_color_to_piece(i, BLACK)]);
                //     print_board(game->pieces[BLACK][i], piece_type_and_color_to_piece(i, BLACK));
                // }
            }
        } else {
            empty_space_counter += 1;
        }
        if (i % 8 == 7) {
            if (empty_space_counter > 0){
                fen[current_char] = '0' + empty_space_counter;
                current_char += 1;
            }
            empty_space_counter = 0;
            if (i != 63){
                
                fen[current_char] = '/';
                current_char += 1;
            }
        }
    }

    fen[current_char] = ' ';
    current_char += 1;

    if (game->side_to_move == WHITE){
        
        fen[current_char] = 'w';
        current_char += 1;

    } else {
        
        fen[current_char] = 'b';
        current_char += 1;

    }

    fen[current_char] = ' ';
    current_char += 1;

    bool no_rights = true;
    // if(game->os.castle_flags & CWK) {
        
    //     fen[current_char] = 'K';
    //     current_char += 1;
    //     no_rights = false;
    // }
    // if(game->os.castle_flags & CWQ) {
        
    //     fen[current_char] = 'Q';
    //     current_char += 1;
    //     no_rights = false;
    // }
    // if(game->os.castle_flags & CBK) {
        
    //     fen[current_char] = 'k';
    //     current_char += 1;
    //     no_rights = false;
    // }
    // if(game->os.castle_flags & CBQ) {
        
    //     fen[current_char] = 'q';
    //     current_char += 1;
    //     no_rights = false;
    // }
    
    if (no_rights){
        
        fen[current_char] = '-';
        current_char += 1;
    }

    fen[current_char] = ' ';
    current_char += 1;


    if (game->os.en_passant_index != -1){

        File file;
        Rank rank;
        index_to_file_and_rank(game->os.en_passant_index, &file, &rank);

        fen[current_char] = file_names[file];
        current_char += 1;

        fen[current_char] = rank_names[rank];
        current_char += 1;
        
    } else {
        
        fen[current_char] = '-';
        current_char += 1;
    }

    fen[current_char] = ' ';
    current_char += 1;
    
    fen[current_char] = game->halfmove + '0';
    current_char += 1;

    fen[current_char] = ' ';
    current_char += 1;

    fen[current_char] = game->fullmove + '0';
    current_char += 1;

    fen[current_char] = '\0';
    current_char += 1;

    
}




void clear_game(Game * game){

    game->board_pieces[BLACK] = 0;
    game->board_pieces[WHITE] = 0;
    game->board_pieces[BOTH] = 0;
    game->pieces[BLACK][PAWN] = 0;
    game->pieces[WHITE][PAWN] = 0;
    game->pieces[BLACK][KNIGHT] = 0;
    game->pieces[WHITE][KNIGHT] = 0;
    game->pieces[BLACK][BISHOP] = 0;
    game->pieces[WHITE][BISHOP] = 0;
    game->pieces[BLACK][ROOK] = 0;
    game->pieces[WHITE][ROOK] = 0;
    game->pieces[BLACK][QUEEN] = 0;
    game->pieces[WHITE][QUEEN] = 0;
    game->pieces[BLACK][KING] = 0;
    game->pieces[WHITE][KING] = 0;
    memset(&game->os, 0, sizeof(StateInfo));
    game->os.en_passant_index = -1;
    game->halfmove = 0;
    game->fullmove = 0;
    game->side_to_move = WHITE;
    game->history_count = 0;
    game->piece_uid = 0;
    // reset_tt(game);

    for (int i = 0; i < 64; i++){
        game->piece_at[i] = PIECE_NONE;
    }
    // for (int i = 0; i < 32; i++){
    //     game->piece_info[i].alive = false;
    // }
    for (int c = 0; c < COLOR_MAX; c++){
        for (int p = 0; p < PIECE_TYPES; p++){
            for (int i = 0; i < PIECE_MAX; i++){
                game->piece_list[c][p][i] = SQ_NONE;
            }
        }
    }
    for (int c = 0; c < COLOR_MAX; c++){
        for (int p = 0; p < PIECE_TYPES; p++){
            game->piece_count[c][p] = 0;
        }
    }

    for (int i = 0; i < PIECE_MAX; i++){
        game->piece_index[i] = 0;
    }
}



int set_board_to_fen(Game * game, char fen[MAX_FEN]){

    clear_game(game);
    int current_space = 0;
    int current_fen_index = 0;
    game->piece_uid = 0;

    // board state
    for (int i = 0; i < MAX_FEN; i++){

        if (fen[i] == ' ') break;

        int skip_spaces = 1;

        if (isdigit(fen[i])){
            skip_spaces = fen[i] - '0';
        } else {
            
            int piece = parse_piece(fen[current_fen_index]);
            if (piece == -1){
                switch(fen[current_fen_index]){
                    case '/':
                        skip_spaces = 0;
                        break;
                    case '\\':
                        skip_spaces = 0;
                        break;
                    default:

                        printf("BOARD STATE FEN ERROR DETECTED AT CHAR %c, RETURNING\n", fen[current_fen_index]);
                        return -1;
                }
                
            } else {
                PieceType converted_type = 0;
                Side side = piece_to_piece_type_and_color(piece, &converted_type);
                
                set_piece(game, &game->pieces[side][converted_type], side, converted_type, current_space);
                    
            }

        }
        
        current_space += skip_spaces;
        current_fen_index += 1;
        
    }

    //blank space
    current_fen_index += 1;

    // side to move
    if (fen[current_fen_index] == 'w' || fen[current_fen_index] == 'W'){

        game->side_to_move = WHITE;
        
    } else if (fen[current_fen_index] == 'b' || fen[current_fen_index] == 'B'){
        
        game->side_to_move = BLACK;

    } else {

        printf("INCORRECT FEN\n");
        return 0;

    }
    
    current_fen_index += 1;
    
    // blank space
    current_fen_index += 1;
    
    // castling rights
    int castling_rights_max = current_fen_index + 4;
    game->os.castle_flags = 0;
    // game->os.castle_flags[WHITE][QUEENSIDE] = false;
    // game->os.castle_flags[WHITE][KINGSIDE] = false;
    // game->os.castle_flags[BLACK][QUEENSIDE] = false;
    // game->os.castle_flags[BLACK][KINGSIDE] = false;
    for (int i = current_fen_index; i < castling_rights_max; i++) {
        
        if (fen[current_fen_index] == ' ') break;
        switch(fen[current_fen_index]){
            case 'K':
                // game->os.castle_flags[WHITE][KINGSIDE] = true;
                game->os.castle_flags |= CWK;
                break;
            case 'Q':
                // game->os.castle_flags[WHITE][QUEENSIDE] = true;
                game->os.castle_flags |= CWQ;
                break;
            case 'k':
                // game->os.castle_flags[BLACK][KINGSIDE] = true;
                game->os.castle_flags |= CBK;
                break;
            case 'q':
                // game->os.castle_flags[BLACK][QUEENSIDE] = true;
                game->os.castle_flags |= CBQ;                break;
            default:
                break;
        }
        current_fen_index += 1;
    }
    
    // blank space
    current_fen_index += 1;

    // en passant square
    
    if (fen[current_fen_index] != '-'){
        
        // parse file
        int ep_file = parse_file(fen[current_fen_index]);
        if (ep_file == -1){
            printf("INCORRECT EP FILE IN FEN, RETURNING\n");
            return 0;
        }
        current_fen_index += 1;

        // parse rank
        int ep_rank = parse_rank(fen[current_fen_index]);
        if (ep_rank == -1){
            printf("INCORRECT EP RANK IN FEN, RETURNING\n");
            return 0;
        }

        game->os.en_passant_index = file_and_rank_to_index(ep_file, ep_rank);
        
        current_fen_index += 1;

    } else {

        game->os.en_passant_index = -1;
        
        current_fen_index += 1;

    }

    // blank space
    current_fen_index += 1;

    char halfmove_str[64];
    int halfmove_index = 0;

    for (int i = current_fen_index; i < MAX_FEN; i++){
        if (fen[current_fen_index] == ' ' || halfmove_index >= 62) break;
        halfmove_str[halfmove_index] = fen[current_fen_index];
        current_fen_index += 1;
        halfmove_index += 1;
    }

    halfmove_str[halfmove_index] = '\0';
    game->halfmove = strtol(halfmove_str, NULL, 10);

    // blank space
    current_fen_index += 1;

    
    char fullmove_str[64];
    int fullmove_index = 0;

    for (int i = current_fen_index; i < MAX_FEN; i++){
        if (fen[current_fen_index] == ' ' || fullmove_index >= 62) break;
        fullmove_str[fullmove_index] = fen[current_fen_index];
        current_fen_index += 1;
        fullmove_index += 1;
    }

    fullmove_str[fullmove_index] = '\0';
    game->fullmove = strtol(fullmove_str, NULL, 10);


    update_blocker_masks(game);

    game->os.pst = NULL;
    game->os.rule50 = 0;
    game->st = &game->os;
    game->os.key = create_zobrist_from_scratch(game);
    game->os.nonpawn_key[WHITE] = create_nonpawn_hash_from_scratch(game, WHITE);
    game->os.nonpawn_key[BLACK] = create_nonpawn_hash_from_scratch(game, BLACK);
    game->os.material_key = create_material_hash_from_scratch(game);
    for (int i = 0; i < PIECE_TYPES; i++){
        game->os.piece_key[i] = create_piece_hash_from_scratch(game, i);
        
    }
    // game->os.pawn_key = create_piece_hash_from_scratch(game, PAWN);
    game->st->rule50 = 0;
    
    compute_check_info(game, &game->os);
    init_evaluate(game);
    return 1;

}





void handle_input(Game * game){

    // printf("HANDLING INPUT\n");
    char input_line[100];
    char * a = fgets(input_line, sizeof(input_line), stdin);
    
    char * l = strtok(input_line, " \n");
    CommandType command = 0;
    int additional_argument_count = 0;
    if (l){
        
        if (strcmp(l, "move") == 0){
            command = COMMAND_MOVE;
            
        } else if (strcmp(l, "perft") == 0){

            command = COMMAND_PERFT;
            
        } else if (strcmp(l, "fen") == 0){

            command = COMMAND_FEN;
            
        } else if (strcmp(l, "display") == 0){
            command = COMMAND_DISPLAY_BOARD;
        } else if (strcmp(l, "generate") == 0){
            command = COMMAND_GENERATE_MOVES;
        } else if (strcmp(l, "bestmove") == 0){
            command = COMMAND_BESTMOVE;
        } else if (strcmp(l, "key") == 0){
            command = COMMAND_GET_KEY;
        } else if (strcmp(l, "auto") == 0){
            command = COMMAND_AUTOMATE;
        } else if (strcmp(l, "debug") == 0){
            command = COMMAND_DEBUG;
        } else if (strcmp(l, "threads") == 0){
            command = COMMAND_THREADS;
        } else if (strcmp(l, "displayoptions") == 0){
            command = COMMAND_DISPLAY_OPTIONS;
        } else if (strcmp(l, "checkhash") == 0){
            command = COMMAND_DEBUG_CHECK_HASH;
        } else if (strcmp(l, "eval") == 0){
            command = COMMAND_DEBUG_EVALUATE;
        } else if (strcmp(l, "uci") == 0){
            command = COMMAND_UCI;
        } else if (strcmp(l, "help") == 0){
            command = COMMAND_HELP;
        } else if (strcmp(l, "quit") == 0){
            command = COMMAND_QUIT;
        }
    }
    bool command_end = false;
    while (l){
        switch(command){
            case COMMAND_MOVE:
                {
                    l = strtok(NULL, " \n");
                    if (l){
                        Move move;
                        PieceType promotion_piece;
                        int code = parse_move(game, l, &move, &promotion_piece);
                        bool promotion = false;
                        if (code == 2){
                            promotion = true;
                        }
                        Move move_list[256];
                        uint8_t move_count = 0;
                        generate_moves(game, game->side_to_move, move_list, &move_count);
                        Move found_move = find_move(move_list, move_count, move_from(move), move_to(move), promotion, promotion_piece);
                        SearchStack stack[1];
                        StateInfo st;
                        if (!found_move){
                            printf("NO MOVE FOUND\n");
                        } else {
                            print_move_full(found_move);
                            Undo undo;
                            if (!make_move(game, found_move, &st, stack)){
                                printf("INVALID MOVE\n");
                                undo_move(game, found_move, stack);
                            }
                            
                        }
                        
                    }
                    command_end = true;
                }
                break;
            case COMMAND_PERFT:
                
                l = strtok(NULL, " \n");
                if (l){
                    int depth = strtol(l, NULL, 10);

                    perft_root(game, depth);
                }
                command_end = true;
                break;
            case COMMAND_FEN:
                l = strtok(NULL, "\n");
                if (l){
                    
                    set_board_to_fen(game, l);
                }
                command_end = true;
                break;
            case COMMAND_DISPLAY_BOARD:
                {
                    print_game_board(game);
                    char fen[MAX_FEN];
                    output_game_to_fen(game, fen);
                    printf("FEN: %s\n", fen);
                    command_end = true;
                }
                break;
            case COMMAND_GENERATE_MOVES:
                {
                    Move move_list[256];
                    uint8_t move_count = 0;
                    generate_moves(game, game->side_to_move, move_list, &move_count);
                    print_moves(move_list, move_count);
                    command_end = true;
                }
                break;
            case COMMAND_BESTMOVE:
                {
                    SearchFlags flags;
                    int max_depth = 0;
                    l = strtok(NULL, "\n");
                    if (l){
                        max_depth = strtol(l, NULL, 10);
                    }
                    // default depth
                    if (max_depth == 0){
                        max_depth = 40;
                    }
                    flags.draw = false;
                    flags.mate = false;
                    flags.three_fold_repetition = false;
                    flags.uci = false;
                    flags.nodes = 0;
                    flags.max_depth = max_depth;
                    flags.max_time = 20;
                    flags.check_hash = false;
                    iterative_search(game, &flags);
                    command_end = true;
                }
                break;
            case COMMAND_GET_KEY:
                {
                    // printf("KEY: %lx\n",create_zobrist_from_scratch(game));
                    printf("KEY: %lx\n",game->os.key);

                    command_end = true;
                }
                break;
            case COMMAND_AUTOMATE:
                {
                    SearchFlags flags;
                    float max_time = 7;
                    l = strtok(NULL, "\n");
                    if (l){
                        max_time = strtod(l, NULL);
                    }

                    game->os.pst = NULL;
                    game->st = &game->os;
                    st_idx = 0;
                    while (true){
                        
                        flags.max_time = max_time;
                        flags.check_hash = false;
                        flags.three_fold_repetition = false;
                        flags.mate = false;
                        flags.nodes = 0;
                        Move move = iterative_search(game, &flags);
                        if (flags.mate || flags.three_fold_repetition){
                            printf("Game end detected.\n");
                            break;
                            
                        }
                        Undo undo;
                        SearchStack stack[1];
                        // StateInfo st;
                        // game->st = &game->os;
                        make_move(game, move, &state_stack[st_idx++], stack);
                        // memcpy(&game->os, &st, sizeof(StateInfo));
                        
                        print_game_board(game);
                        char fen[MAX_FEN];
                        output_game_to_fen(game, fen);
                        printf("FEN: %s\n", fen);
                    }
                    command_end = true;
                }
                break;
            case COMMAND_DEBUG:
                {
                    eo.debug_info = !eo.debug_info;
                    if (eo.debug_info){
                        printf("Debug info enabled.\n");
                    } else {
                        
                        printf("Debug info disabled.\n");
                    }
                }
                break;
            case COMMAND_THREADS:
                {
                    int t = 1;
                    l = strtok(NULL, "\n");
                    if (l){
                        t = strtod(l, NULL);
                        eo.threads = t;
                    }
                    printf("Threads set to %d\n", t);
                }
                break;
            case COMMAND_DISPLAY_OPTIONS:
                {
                    printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                    "Here are the available UCI options (syntax: setoption name <name> value <value>):\n",
                    "- debug\n",
                    "- threads\n",
                    "- eval_scale\n",
                    "- aspiration_base\n",
                    "- aspiration_mul\n",
                    "- qdelta_margin\n",
                    "- qsee_margin\n",
                    "- check_prune_margin\n",
                    "- rfp_mul\n",
                    "- rfp_base\n",
                    "- rfp_improving\n",
                    "- rfp_depth\n",
                    "- razor_depth\n",
                    "- razor_base\n",
                    "- razor_mul\n",
                    "- razor_improving\n",
                    "- nmp_mul\n",
                    "- nmp_base\n",
                    "- nmp_slope\n",
                    "- probcut_depth\n",
                    "- probcut_base\n",
                    "- probcut_improving\n",
                    "- iid_depth\n",
                    "- lmr_depth\n",
                    "- lmr_move_start\n",
                    "- lmr_hd\n",
                    "- lmr_cap_mul\n",
                    "- lmr_cap_base\n",
                    "- lmr_quiet_mul\n",
                    "- lmr_quiet_base\n",
                    "- lmp_depth\n",
                    "- lmp_base\n",
                    "- lmp_improving\n",
                    "- lmp_depth_pow\n",
                    "- futility_depth\n",
                    "- futility_base\n",
                    "- futility_mul\n",
                    "- futility_hist_mul\n",
                    "- futility_improving\n",
                    "- chist_depth\n",
                    "- chist1_margin\n",
                    "- chist2_margin\n",
                    "- mp_goodcap_margin\n",
                    "- chist1_scale\n",
                    "- chist2_scale\n",
                    "- chist4_scale\n",
                    "- chist6_scale\n",
                    "- see_depth\n",
                    "- see_quiet_margin\n",
                    "- see_nonquiet_margin\n",
                    "- se_depth\n",
                    "- se_depth_margin\n",
                    "- qhistory_base\n",
                    "- qhistory_mul\n",
                    "- qhpen_base\n",
                    "- qhpen_mul\n",
                    "- chistory_base\n",
                    "- chistory_mul\n",
                    "- chpen_base\n",
                    "- chpen_mul\n",
                    "- beta_bonus\n",
                    "- corr_depth_base\n",
                    "- corrhist_grain\n",
                    "- corrhist_weight\n",
                    "- corrhist_max\n",
                    "- corr_pawn_weight\n",
                    "- corr_np_weight\n",
                    "- corr_mat_weight\n",
                    "- corr_kbn_weight\n",
                    "- corr_kqr_weight\n",
                    "- corr_ch_weight\n",
                    "- l1\n",
                    "- l2\n",
                    "- l3\n");
                }
                break;
            case COMMAND_DEBUG_CHECK_HASH:
                {
                    SearchFlags flags;
                    int max_time = 10;
                    l = strtok(NULL, "\n");
                    if (l){
                        max_time = strtod(l, NULL);
                    }
                    flags.max_time = max_time;
                    if(!check_hash(game)){
                        printf("HASH WRONG: %lx\n", game->st->key);
                        printf("CORRECT: %lx\n", create_zobrist_from_scratch(game));
                    }
                    // flags.check_hash = true;
                    // iterative_search(game, &flags);
                    command_end = true;
                }
                break;
            case COMMAND_DEBUG_EVALUATE:
                {
                    // TODO make a new debug eval command. it's a pain
                    // debug_evaluate(game, game->side_to_move);
                    SearchData d;
                    bool lazy = false;
                    // evaluate(game, game->side_to_move, &d,  1, -INT_MAX, INT_MAX, &lazy, true, true);
                }
                break;
            case COMMAND_UCI:
                {
                    game->uci_mode = true;
                    printf("id name Sarah 1.0\n");
                    printf("uciok\n");
                    fflush(stdout);

                    while (true){
                        // if (!handle_uci_input(game, NULL)) break;
                        handle_uci_input(game, NULL);
                        if (!running) return;
                    }
                    command_end = true;
                }
                break;
            case COMMAND_HELP:
                {
                    printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                    "Here are the available commands:\n",
                    "- fen [fen]: Sets board to given fen.\n",
                    "- perft [depth]: performs a perft (move generation verification) to given depth on the current position\n",
                    "- display: displays the current position and fen\n",
                    "- generate: lists all pseudolegal moves for the given position\n",
                    "- bestmove [depth]: performs a search to given depth to find the best move\n",
                    "- move [move]: plays a move given in algebraic notation\n",
                    "- auto: plays the engine against itself from the current position\n",
                    "- debug: toggles debug information for the current (default = true)\n",
                    "- displayoptions: displays all uci options\n",
                    "- threads [count]: sets engine thread amount\n",
                    "- help: displays this dialogue\n",
                    "- quit: exits the program\n",
                    "- uci: enter uci mode\n");
                }
                break;
            case COMMAND_QUIT:
                {
                    running = 0;
                    command_end = true;
                }
                break;
        }

        additional_argument_count += 1;

        l = strtok(NULL, " ");
        if (command_end) break;
    }
    
    
}



int init_new_game(Game * game, Side color){

    sprintf(game->fen[0], "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    set_board_to_fen(game, game->fen[0]);
    
    return 1;
}

void init_game_from_fen(Game * game, Side color, char fen[MAX_FEN]){
    
    strcpy(game->fen[0], fen);
    printf("INITIALIZING GAME TO FEN:\n%s\n", game->fen[0]);
    set_board_to_fen(game, game->fen[0]);
    
    
}

