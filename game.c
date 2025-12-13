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
// #include "types.h"



// bool piece_is_attacking_square(Game * game, PieceType piece, Side side, int start_square, int end_square, uint64_t blockers){
    

//     switch (piece){
//         case PAWN:
//             return game->pawn_captures[side][start_square] & (1ULL << end_square);
//             break;
//         case KNIGHT:
//             return game->knight_moves[start_square] & (1ULL << end_square);
//             break;
//         case BISHOP:
//             return (fetch_bishop_moves(game, start_square, blockers)) & (1ULL << end_square);
//             break;
//         case ROOK:
//             return (fetch_rook_moves(game, start_square, blockers)) & (1ULL << end_square);
//             break;
//         case QUEEN:
//             return (fetch_bishop_moves(game, start_square, blockers) | fetch_rook_moves(game, start_square, blockers)) & (1ULL << end_square);
//             break;
//         case KING:
//             return game->king_moves[start_square] & (1ULL << end_square);
//             break;
//     }

// }





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
                printf("ERROR: BOARDS UNSYNCED\n");
                printf("PIECE BOARD WHITE:\n");
                print_board(game->board_pieces[WHITE], WHITE_KING);
                for (int i = 0; i < PIECE_TYPES; i++){
                    printf("PIECE: %c", piece_names[piece_type_and_color_to_piece(i, WHITE)]);
                    print_board(game->pieces[WHITE][i], piece_type_and_color_to_piece(i, WHITE));
                }
                printf("PIECE BOARD BLACK:\n");
                print_board(game->board_pieces[BLACK], BLACK_KING);
                for (int i = 0; i < PIECE_TYPES; i++){
                    printf("PIECE: %c", piece_names[piece_type_and_color_to_piece(i, BLACK)]);
                    print_board(game->pieces[BLACK][i], piece_type_and_color_to_piece(i, BLACK));
                }
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
    if(game->castle_flags[WHITE][KINGSIDE]) {
        
        fen[current_char] = 'K';
        current_char += 1;
        no_rights = false;
    }
    if(game->castle_flags[WHITE][QUEENSIDE]) {
        
        fen[current_char] = 'Q';
        current_char += 1;
        no_rights = false;
    }
    if(game->castle_flags[BLACK][KINGSIDE]) {
        
        fen[current_char] = 'k';
        current_char += 1;
        no_rights = false;
    }
    if(game->castle_flags[BLACK][QUEENSIDE]) {
        
        fen[current_char] = 'q';
        current_char += 1;
        no_rights = false;
    }
    
    if (no_rights){
        
        fen[current_char] = '-';
        current_char += 1;
    }

    fen[current_char] = ' ';
    current_char += 1;


    if (game->en_passant_index != -1){

        File file;
        Rank rank;
        index_to_file_and_rank(game->en_passant_index, &file, &rank);

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
    game->en_passant_index = -1;
    game->halfmove = 0;
    game->fullmove = 0;
    game->side_to_move = WHITE;
    game->history_count = 0;
    // reset_tt(game);

    for (int i = 0; i < 64; i++){
        game->piece_at[i] = 0;
    }
}



int set_board_to_fen(Game * game, char fen[MAX_FEN]){

    clear_game(game);
    int current_space = 0;
    int current_fen_index = 0;

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
                
                set_piece(game, &game->pieces[side][converted_type], converted_type, current_space);
                    
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
    game->castle_flags[WHITE][KINGSIDE] = false;
    game->castle_flags[BLACK][KINGSIDE] = false;
    game->castle_flags[WHITE][QUEENSIDE] = false;
    game->castle_flags[BLACK][QUEENSIDE] = false;
    for (int i = current_fen_index; i < castling_rights_max; i++) {
        
        if (fen[current_fen_index] == ' ') break;
        switch(fen[current_fen_index]){
            case 'K':
                game->castle_flags[WHITE][KINGSIDE] = true;
                break;
            case 'Q':
                game->castle_flags[WHITE][QUEENSIDE] = true;
                break;
            case 'k':
                game->castle_flags[BLACK][KINGSIDE] = true;
                break;
            case 'q':
                game->castle_flags[BLACK][QUEENSIDE] = true;
                break;
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

        game->en_passant_index = file_and_rank_to_index(ep_file, ep_rank);
        
        current_fen_index += 1;

    } else {

        game->en_passant_index = -1;
        
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

    game->key = create_zobrist_from_scratch(game);
    game->pawn_key = create_pawn_hash_from_scratch(game);
    init_evaluate(game);
    return 1;

}





void handle_input(Game * game){

    // printf("HANDLING INPUT\n");
    char input_line[100];
    fgets(input_line, sizeof(input_line), stdin);
    
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
        } else if (strcmp(l, "checkhash") == 0){
            command = COMMAND_DEBUG_CHECK_HASH;
        } else if (strcmp(l, "eval") == 0){
            command = COMMAND_DEBUG_EVALUATE;
        } else if (strcmp(l, "uci") == 0){
            command = COMMAND_UCI;
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
                        Move move_list[400];
                        int move_count = 0;
                        generate_moves(game, game->side_to_move, move_list, &move_count);
                        Move * found_move = find_move(move_list, move_count, move.start_index, move.end_index, promotion, promotion_piece);
                        if (!found_move){
                            printf("NO MOVE FOUND\n");
                        } else {
                            print_move_full(found_move);
                            if (!make_move(game, found_move)){
                                printf("INVALID MOVE\n");
                                undo_move(game, found_move);
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
                    Move move_list[400];
                    int move_count = 0;
                    generate_moves(game, game->side_to_move, move_list, &move_count);
                    print_moves(move_list, move_count);
                    command_end = true;
                }
                break;
            case COMMAND_BESTMOVE:
                {
                    SearchFlags flags;
                    int max_depth = -1;
                    l = strtok(NULL, "\n");
                    if (l){
                        max_depth = strtol(l, NULL, 10);
                    }
                    flags.max_depth = max_depth;
                    flags.max_time = 50;
                    flags.check_hash = false;
                    iterative_search(game, &flags);
                    command_end = true;
                }
                break;
            case COMMAND_GET_KEY:
                {
                    // printf("KEY: %lx\n",create_zobrist_from_scratch(game));
                    printf("KEY: %lx\n",game->key);

                    command_end = true;
                }
                break;
            case COMMAND_AUTOMATE:
                {
                    SearchFlags flags;
                    int max_time = 10;
                    l = strtok(NULL, "\n");
                    if (l){
                        max_time = strtod(l, NULL);
                    }
                    while (true){
                        
                        flags.max_time = max_time;
                        flags.check_hash = false;
                        flags.three_fold_repetition = false;
                        flags.mate = false;
                        Move move = iterative_search(game, &flags);
                        if (flags.mate || flags.three_fold_repetition){
                            printf("Game end detected.\n");
                            break;
                            
                        }
                        make_move(game, &move);
                        
                        
                        print_game_board(game);
                        char fen[MAX_FEN];
                        output_game_to_fen(game, fen);
                        printf("FEN: %s\n", fen);
                    }
                    command_end = true;
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
                    flags.check_hash = true;
                    iterative_search(game, &flags);
                    command_end = true;
                }
                break;
            case COMMAND_DEBUG_EVALUATE:
                {
                    // TODO make a new debug eval command. it's a pain
                    // debug_evaluate(game, game->side_to_move);
                }
                break;
            case COMMAND_UCI:
                {
                    game->uci_mode = true;
                    printf("id name Vixen 2.0\n");
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

    // clear history
    // for (int i = 0; i < 64; i++){
    //     for (int c = 0; c < 64; c++){
    //         game->history_table[WHITE][i][c] = 0;
    //         game->history_table[BLACK][i][c] = 0;
    //     }
    // }

    
    return 1;
}

void init_game_from_fen(Game * game, Side color, char fen[MAX_FEN]){
    
    strcpy(game->fen[0], fen);
    printf("INITIALIZING GAME TO FEN:\n%s\n", game->fen[0]);
    set_board_to_fen(game, game->fen[0]);
    
    
}

