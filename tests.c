

#include "types.h"
#include "game.h"
#include "move_generation.h"
#include "utils.h"
#include "search.h"

static void trim(char *s) {
    int len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' || s[len-1] == ' '))
        s[--len] = 0;
}

bool parse_test_line(const char *line_raw, TestPos *out) {
    char line[512];
    strncpy(line, line_raw, sizeof(line));
    trim(line);

    if (line[0] == 0) return false; // skip blanks

    // ---------- Step 1: split on " bm "
    char *bm_ptr = strstr(line, " bm ");
    if (!bm_ptr) return false;

    // Extract FEN part (everything before " bm ")
    size_t fen_len = bm_ptr - line;
    if (fen_len >= sizeof(out->fen)) fen_len = sizeof(out->fen)-1;
    strncpy(out->fen, line, fen_len);
    out->fen[fen_len] = 0;
    trim(out->fen);

    // ---------- Step 2: move token begins after " bm "
    char *move_start = bm_ptr + 4;

    // Move ends at first semicolon
    char *semi = strchr(move_start, ';');
    if (!semi) return false;

    size_t mv_len = semi - move_start;
    if (mv_len >= sizeof(out->bestmove)) mv_len = sizeof(out->bestmove)-1;

    strncpy(out->bestmove, move_start, mv_len);
    out->bestmove[mv_len] = 0;
    trim(out->bestmove);

    // ---------- Step 3: optional id
    char *id_ptr = strstr(semi, "id ");
    if (id_ptr) {
        id_ptr += 3;
        while (*id_ptr == ' ' || *id_ptr == '"') id_ptr++;
        char *end = strchr(id_ptr, '"');
        if (!end) end = strchr(id_ptr, ';');
        if (!end) end = id_ptr + strlen(id_ptr);

        size_t id_len = end - id_ptr;
        if (id_len >= sizeof(out->id)) id_len = sizeof(out->id)-1;
        strncpy(out->id, id_ptr, id_len);
        out->id[id_len] = 0;
    } else {
        out->id[0] = 0;
    }

    return true;
}
PieceType piece_letter_to_piece_type(char c) {
    switch (c) {
        case 'P': return PAWN;
        case 'N': return KNIGHT;
        case 'B': return BISHOP;
        case 'R': return ROOK;
        case 'Q': return QUEEN;
        case 'K': return KING;
    }
    return PAWN;
}

PieceType letter_to_piece_type(char c) {
    return piece_letter_to_piece_type(c);
}


bool parse_test_move(Game *g, const char *tok, Move *out)
{
    memset(out, 0, sizeof(*out));

    // ---- Handle castling
    // if (!strcmp(tok, "O-O") || !strcmp(tok, "0-0")) {
    //     return find_and_encode_move(g, out, KING, g->king_square[g->side_to_move], 
    //                                 g->side_to_move == WHITE ? 6 : 62);
    // }
    // if (!strcmp(tok, "O-O-O") || !strcmp(tok, "0-0-0")) {
    //     return find_and_encode_move(g, out, KING, g->king_square[g->side_to_move], 
    //                                 g->side_to_move == WHITE ? 2 : 58);
    // }

    int len = strlen(tok);
    char piece = 0;
    int idx = 0;

    // ---- Check if first char is a piece letter
    if (tok[0] >= 'A' && tok[0] <= 'Z' && tok[0] != 'O') {
        piece = tok[idx++];
    } else {
        piece = 'P'; // pawn
    }

    bool is_capture = false;

    // ---- Capture marker
    if (tok[idx] == 'x') {
        is_capture = true;
        idx++;
    }

    // ---- Destination square
    char file = tok[idx++];
    char rank = tok[idx++];
    int to = abs(7 - (rank - '1')) * 8 + (file - 'a');

    // ---- Promotion (optional)
    char promo = 0;
    if (idx < len) {
        promo = tok[idx];
    }

    // ---- Now find matching move in legal move list
    Move moves[200];
    int move_count = 0;
    generate_moves(g, g->side_to_move, moves, &move_count);

    // printf("TO INDEX: %d\n", to);
    for (int i = 0; i < move_count; i++) {
        if (moves[i].end_index != to) continue;

        PieceType pt = g->piece_at[moves[i].start_index];
        // printf("PIECE AT: %c\n", piece_names[pt]);
        if (pt != piece_letter_to_piece_type(piece)) continue;

        // capture consistency
        if (is_capture != (moves[i].type == CAPTURE)) continue;

        // promotion consistency
        if (promo) {
            if (moves[i].promotion_type != letter_to_piece_type(promo))
                continue;
        }

        *out = moves[i];
        return true;
    }

    printf("FAILED TO PARSE TEST MOVE: %s\n", tok);
    return false;
}

void run_test_file(Game * game, const char *filename) {
    FILE *f = fopen(filename, "r");
    char line[512];

    int total = 0, correct = 0;
    char pn[6] = {'p', 'n', 'b', 'r', 'q', 'k'};

    while (fgets(line, sizeof(line), f)) {
        TestPos tp;
        if (!parse_test_line(line, &tp)) continue;

        // Game g;
        set_board_to_fen(game, tp.fen);

        Move expected;
        if (!parse_test_move(game, tp.bestmove, &expected)) {
            printf("Could not parse best move for %s\n", tp.id);
            continue;
        }


        SearchFlags flags;
        int max_depth = 15;
        flags.max_depth = max_depth;
        flags.max_time = 22;
        flags.check_hash = false;
        Move actual = iterative_search(game, &flags);
        // Move actual = search_best_move(&g, 8); // or whatever depth

        if (move_is_equal(&expected, &actual)) {
            correct++;
        } else {
            printf("[X] %s: expected %s but engine played:",
                   tp.id, tp.bestmove);
            printf(" %c",pn[actual.piece]);
            print_move_algebraic(&actual);
            printf("\n");
        }

        total++;
    }

    printf("=== Score: %d / %d ===\n", correct, total);
}
