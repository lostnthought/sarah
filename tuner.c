#include "eval.h"
#include "types.h"
#include "game.h"
#include "utils.h"



Score eval_params[2048];
ParamIndex ep_idx;

void dump_weights_text(const char *path, const ParamIndex * pi, const double *W_mg, const double *W_eg) {
    FILE *f = fopen(path, "w");
    if (!f) return;

    // #define PRINT(name) \
    //     fprintf(f, "%s = { mg = %.6f, eg = %.6f }\n", #name, \
    //             W_mg[pi->name[WHITE]], \
    //             W_eg[pi->name[WHITE]])

    // PRINT(isolated_pawn);
    // PRINT(doubled_pawn);
    // PRINT(passed_pawn);
    // PRINT(backward_pawn);
    // PRINT(candidate_passer);
    // PRINT(chained_pawn);
    // PRINT(bishop_pair);
    // PRINT(rook_semi_open);
    // PRINT(rook_open);
    // PRINT(connected_rook_bonus);
    // PRINT(open_file_king_penalty);
    // PRINT(semi_open_file_king_penalty);
    // PRINT(open_middle_file_king_penalty);
    // PRINT(semi_open_diag_king_penalty);
    // PRINT(open_diag_king_penalty);
    // PRINT(semi_open_diag_from_king_penalty);
    // PRINT(queen_rook_connected);
    // PRINT(queen_bishop_connected);
    // PRINT(king_zone_attackers);
    // PRINT(king_zone_defenders);
    // PRINT(king_zone_battery);
    // PRINT(pawn_blockers_penalty);
    // PRINT(open_file_near_king_with_attacker);
    // PRINT(open_file_from_king_with_attacker);
    // PRINT(open_diag_near_king_with_attacker);
    // PRINT(open_diag_from_king_with_attacker);
    // PRINT(attacked_by_pawn_penalty);

    #undef PRINT
    const char* pieceName[6] = {"P", "N", "B", "R", "Q", "K"};
    for (int p = 0; p < PIECE_TYPES; p++){
        fprintf(f,"%s: \n",pieceName[p]);
        for (int s = 0; s < 64; s++){
            if (s % 8 == 0){
                fprintf(f, "\n");
            }
            fprintf(f, "%d, ", (int)roundf(W_mg[pi->psqt[p][s]]));
        }
        fprintf(f,"\n");
    }
    for (int p = 0; p < PIECE_TYPES; p++){
        fprintf(f,"%s: \n",pieceName[p]);
        for (int s = 0; s < 64; s++){
            if (s % 8 == 0){
                fprintf(f, "\n");
            }
            fprintf(f, "%d, ", (int)roundf(W_eg[pi->psqt[p][s]]));
        }
        fprintf(f, "\n");
    }

    fclose(f);
}

void print_weights(double * W, const ParamIndex * pi){
    
    #define PRINT(name) \
        printf("%s = %.2f\n", #name, \
                W[pi->name])
    #define LOOP_PRINT(name, cnt) \
        printf("%s = {", #name); \
        for (int i = 0; i < cnt; i++) { \
            printf("%.2f, ", W[pi->name[i]]); \
        } \
        printf("}\n")\

    LOOP_PRINT(piece_values, PIECE_TYPES);

    LOOP_PRINT(isolated_pawn, 8);
    LOOP_PRINT(passed_isolated_pawn, 8);
    PRINT(doubled_pawn);
    LOOP_PRINT(passed_pawn, 8);
    PRINT(connected_passer);
    LOOP_PRINT(backward_pawn, 8);
    LOOP_PRINT(candidate_passer, 8);
    PRINT(chained_pawn);

    for (int p = 0; p < PIECE_TYPES; p++){
        printf("MOBILITY %c: ", PNAME[p]);
        for (int i = 0; i < 64; i++){
            printf("%.2f, ", W[pi->mobility[p][i]]);
        }
        printf("\n");
    }
    printf("\n");
    

    PRINT(bishop_pair);
    LOOP_PRINT(defended_pc, 8);
    LOOP_PRINT(bishop_pawn_color, 8);
    LOOP_PRINT(bishop_pawn_color_w_attacker, 8);
    PRINT(outpost_occ);
    PRINT(outpost_control);
    PRINT(minor_cannot_enter_et);
    PRINT(slider_boxed_in);
    PRINT(unstoppable_attack);
    PRINT(rook_open);
    PRINT(rook_semi_open);
    PRINT(connected_rook);
    PRINT(qr_connected);
    PRINT(qb_connected);
    PRINT(weak_queen);
    LOOP_PRINT(tdp_weak_sq, 16);
    LOOP_PRINT(tdp_oed_pen, 16);
    LOOP_PRINT(unsafe, 16);
    LOOP_PRINT(awp, 8);
    LOOP_PRINT(safe_pawn_attacks, 16);
    LOOP_PRINT(safe_pawn_push_attacks, 16);
    LOOP_PRINT(threat_by_major, PIECE_TYPES);
    LOOP_PRINT(threat_by_minor, PIECE_TYPES);
    PRINT(weak_pieces);

    PRINT(passer_is_unstoppable);
    PRINT(passer_deficit);
    PRINT(passer_supported);
    PRINT(passer_blocked);
    PRINT(opponent_stuck_in_front_of_passer);
    PRINT(opponent_rook_stuck_on_promo_rank);
    PRINT(rook_on_passer_file);
    PRINT(rook_stuck_in_front_of_passer);
    PRINT(extra_passers_on_seven);

    LOOP_PRINT(ks_safe_checks, PIECE_TYPES);
    LOOP_PRINT(ks_unsafe_checks, PIECE_TYPES);
    LOOP_PRINT(ks_double_safe_king_attacks, 16);
    LOOP_PRINT(ks_safe_king_attacks, 16);
    LOOP_PRINT(ks_threat_correction, PIECE_TYPES);
    PRINT(ks_rook_contact_check);
    PRINT(ks_queen_contact_check);
    LOOP_PRINT(ks_akz, PIECE_TYPES);
    LOOP_PRINT(ks_aikz, 8);
    PRINT(ks_weak);
    LOOP_PRINT(ks_defended_squares_kz, 24);
    LOOP_PRINT(ks_def_minor, 8);
    LOOP_PRINT(ks_ek_defenders, 8);
    LOOP_PRINT(ks_pawn_shield, 3);
    LOOP_PRINT(ks_empty, 8);

    
    
}
void print_scores(int16_t * W_mg,  int16_t * W_eg, const ParamIndex * pi){
    
    #define PSCORE(name) \
        printf("%s = {%d, %d}\n", #name, \
                W_mg[pi->name], W_eg[pi->name])
    #define LSCORE(name, cnt) \
        printf("%s = ", #name); \
        for (int i = 0; i < cnt; i++) { \
            printf("{%d, %d}, ", W_mg[pi->name[i]], W_eg[pi->name[i]]); \
        } \
        printf("\n")\

    LSCORE(piece_values, PIECE_TYPES);

    for (int p = 0; p < PIECE_TYPES; p++){
        printf("PSQT %c = {\n", PNAME[p]);
        for (int i = 0; i < 64; i++){
            if (i % 8 == 0) {printf("\n");}
            printf("{%d, %d}, ", W_mg[pi->psqt[p][i]], W_eg[pi->psqt[p][i]]);
            
        }
        printf("}\n");
    }
    printf("\n");

    LSCORE(isolated_pawn, 8);
    LSCORE(passed_isolated_pawn, 8);
    PSCORE(doubled_pawn);
    LSCORE(passed_pawn, 8);
    PSCORE(connected_passer);
    LSCORE(backward_pawn, 8);
    LSCORE(candidate_passer, 8);
    PSCORE(chained_pawn);


    for (int p = 0; p < PIECE_TYPES; p++){
        printf("MOBILITY %c = {\n", PNAME[p]);
        for (int i = 0; i < 64; i++){
            printf("{%d, %d}, ", W_mg[pi->mobility[p][i]], W_eg[pi->mobility[p][i]]);
        }
        printf("}\n");
    }
    printf("\n");
    

    PSCORE(bishop_pair);
    LSCORE(defended_pc, 8);
    PSCORE(outpost_occ);
    PSCORE(outpost_control);
    PSCORE(minor_cannot_enter_et);
    PSCORE(slider_boxed_in);
    PSCORE(unstoppable_attack);
    PSCORE(rook_open);
    PSCORE(rook_semi_open);
    PSCORE(connected_rook);
    PSCORE(qr_connected);
    PSCORE(qb_connected);
    PSCORE(weak_queen);
    LSCORE(tdp_weak_sq, 16);
    LSCORE(tdp_oed_pen, 16);
    LSCORE(unsafe, 16);
    LSCORE(awp, 8);
    LSCORE(safe_pawn_attacks, 16);
    LSCORE(safe_pawn_push_attacks, 16);
    LSCORE(threat_by_major, PIECE_TYPES);
    LSCORE(threat_by_minor, PIECE_TYPES);

    PSCORE(passer_is_unstoppable);
    PSCORE(passer_deficit);
    PSCORE(passer_supported);
    PSCORE(passer_blocked);
    PSCORE(opponent_stuck_in_front_of_passer);
    PSCORE(opponent_rook_stuck_on_promo_rank);
    PSCORE(rook_on_passer_file);
    PSCORE(rook_stuck_in_front_of_passer);
    PSCORE(extra_passers_on_seven);

    LSCORE(ks_safe_checks, PIECE_TYPES);
    LSCORE(ks_unsafe_checks, PIECE_TYPES);
    LSCORE(ks_double_safe_king_attacks, 16);
    LSCORE(ks_safe_king_attacks, 16);
    LSCORE(ks_threat_correction, PIECE_TYPES);
    PSCORE(ks_rook_contact_check);
    PSCORE(ks_queen_contact_check);
    LSCORE(ks_akz, PIECE_TYPES);
    LSCORE(ks_aikz, 8);
    LSCORE(ks_defended_squares_kz, 24);
    LSCORE(ks_def_minor, 8);
    LSCORE(ks_ek_defenders, 8);
    LSCORE(ks_pawn_shield, 3);
    LSCORE(ks_empty, 8);

    
    
}
void print_packed_scores(int32_t * W, const ParamIndex * pi){
    
    #define PPSCORE(name) \
        printf("%s = %d\n", #name, \
                W[pi->name])
    #define LPSCORE(name, cnt) \
        printf("%s = ", #name); \
        for (int i = 0; i < cnt; i++) { \
            printf("%d, ", W[pi->name[i]]); \
        } \
        printf("\n")\

    LPSCORE(piece_values, PIECE_TYPES);

    for (int p = 0; p < PIECE_TYPES; p++){
        printf("PSQT %c = {\n", PNAME[p]);
        for (int i = 0; i < 64; i++){
            if (i % 8 == 0) {printf("\n");}
            printf("%d, ", W[pi->psqt[p][i]]);
            
        }
        printf("}\n");
    }
    printf("\n");

    LPSCORE(isolated_pawn, 8);
    LPSCORE(passed_isolated_pawn, 8);
    PPSCORE(doubled_pawn);
    LPSCORE(passed_pawn, 8);
    PPSCORE(connected_passer);
    LPSCORE(backward_pawn, 8);
    LPSCORE(candidate_passer, 8);
    PPSCORE(chained_pawn);


    for (int p = 0; p < PIECE_TYPES; p++){
        printf("MOBILITY %c = {\n", PNAME[p]);
        for (int i = 0; i < 64; i++){
            printf("%d, ", W[pi->mobility[p][i]]);
        }
        printf("}\n");
    }
    printf("\n");
    

    PPSCORE(bishop_pair);
    LPSCORE(bishop_pawn_color, 8);
    LPSCORE(bishop_pawn_color_w_attacker, 8);
    LPSCORE(defended_pc, 8);
    PPSCORE(outpost_occ);
    PPSCORE(outpost_control);
    PPSCORE(minor_cannot_enter_et);
    PPSCORE(slider_boxed_in);
    PPSCORE(unstoppable_attack);
    PPSCORE(rook_open);
    PPSCORE(rook_semi_open);
    PPSCORE(connected_rook);
    PPSCORE(qr_connected);
    PPSCORE(qb_connected);
    PPSCORE(weak_queen);
    LPSCORE(tdp_weak_sq, 16);
    LPSCORE(tdp_oed_pen, 16);
    LPSCORE(unsafe, 16);
    LPSCORE(awp, 8);
    LPSCORE(safe_pawn_attacks, 16);
    LPSCORE(safe_pawn_push_attacks, 16);
    LPSCORE(threat_by_major, PIECE_TYPES);
    LPSCORE(threat_by_minor, PIECE_TYPES);
    PPSCORE(weak_pieces);

    PPSCORE(passer_is_unstoppable);
    PPSCORE(passer_deficit);
    PPSCORE(passer_supported);
    PPSCORE(passer_blocked);
    PPSCORE(opponent_stuck_in_front_of_passer);
    PPSCORE(opponent_rook_stuck_on_promo_rank);
    PPSCORE(rook_on_passer_file);
    PPSCORE(rook_stuck_in_front_of_passer);
    PPSCORE(extra_passers_on_seven);

    LPSCORE(ks_safe_checks, PIECE_TYPES);
    LPSCORE(ks_unsafe_checks, PIECE_TYPES);
    LPSCORE(ks_double_safe_king_attacks, 16);
    LPSCORE(ks_safe_king_attacks, 16);
    LPSCORE(ks_threat_correction, PIECE_TYPES);
    PPSCORE(ks_rook_contact_check);
    PPSCORE(ks_queen_contact_check);
    LPSCORE(ks_akz, PIECE_TYPES);
    LPSCORE(ks_aikz, 8);
    PPSCORE(ks_weak);
    LPSCORE(ks_defended_squares_kz, 24);
    LPSCORE(ks_def_minor, 8);
    LPSCORE(ks_ek_defenders, 8);
    LPSCORE(ks_pawn_shield, 3);
    printf("\n");
    LPSCORE(ks_empty, 8);
    printf("\n");

    
    
}



int load_params(const char *filename, int expected_count) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open parameter file: %s\n", filename);
        return 0;
    }

    weights_mg = malloc(sizeof(double) * expected_count);
    if (!weights_mg) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(f);
        return 0;
    }
    weights_eg = malloc(sizeof(double) * expected_count);
    if (!weights_eg) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(f);
        return 0;
    }

    size_t mg = fread(weights_mg, sizeof(double), expected_count, f);
    size_t eg = fread(weights_eg, sizeof(double), expected_count, f);
    fclose(f);

    if ((int)mg != expected_count) {
        fprintf(stderr, "Parameter file mismatch: expected %d doubles, got %zu\n",
                expected_count, mg);
        free(weights_mg);
        free(weights_eg);
        weights_mg = NULL;
        weights_eg = NULL;
        return 0;
    }

    return 1;
}
void scale_texel_weights(float scale){
    for (int i = 0; i < params.total_params; i++){
        weights_mg[i] *= scale;
    }
    for (int i = 0; i < params.total_params; i++){
        weights_eg[i] *= scale;
    }
}

double t_tanh(double x, double k){
    return k * (x - 0.5);
}
double half_sig(double x, double k){
    return 1 - (exp(-k * x));
}
double shift_pow(double x, double t, double p) {
    if (x < t) return 0.0;
    double y = (x - t) / (1.0 - t);
    return pow(y, p);
}
double pow_exp(double x, double p){
    double d = x - 0.5;
    return copysign(pow(fabs(d), p), d);
}
double log_curve(double x, double d, double k){
    double y = copysign(log(1 + k * fabs(d)), d);
    return y / log(1 + k * 0.5);
}
double exp_div(double x, double k){
    return (exp(k * x) - 1.0) / (exp(k) - 1.0);
}


void init_t_tanh_table(int size, double scale, int table[size], double k) {
    
    for (int i = 0; i < size; i++){
        double v = t_tanh((double)i / size, k);
        int iv = roundf(0.5 * (v + 1.0) * scale * size);
        table[i] = iv;
    }
}
void init_t_tanh_table_neg(int size, double scale, int table[size], double k) {
    
    for (int i = 0; i < size; i++){
        double v = t_tanh((double)i / size, k);
        int iv = roundf(v * scale * size);
        table[i] = iv;
    }
}

void init_half_sig_table(int size, double scale, int table[size], double k) {
    
    for (int i = 0; i < size; i++){
        double v = half_sig((double)i / size, k);
        int iv = roundf(v * scale * size);
        table[i] = iv;
    }
}

void init_shift_pow_table(int size, double scale, int table[size], double k, double t) {
    
    for (int i = 0; i < size; i++){
        double v = shift_pow((double)i / size, t, k);
        int iv = roundf(v * scale * size);
        table[i] = iv;
    }
}



int THREAT_SCORE[500];
int KING_DANGER[1000];
int KING_THREAT[1000];
int PASSER_SCORE[200];
int PSQT_SCALE[10000];

void init_eval_tables(){

    init_t_tanh_table(100, 1, THREAT_SCORE, 1.6);
    for (int i = 100; i < 500; i++){ THREAT_SCORE[i] = THREAT_SCORE[99]; }

    
    init_shift_pow_table(1000, 1.7, KING_DANGER, 0.75, 0.01);

    init_shift_pow_table(1000, 2, KING_THREAT, 0.7, 0.01);

    // init_t_tanh_table_neg(200, 1, PASSER_SCORE, 0.7);

    // init_shift_pow_table(10000, 1, PSQT_SCALE, 0.75, 0.01);
    // for (int i = 2400; i < 10000; i++){ PSQT_SCALE[i] = PSQT_SCALE[2399]; }
    // for (int i = 0; i < 800; i++){
    //  printf("%d, ", KING_THREAT[i]);
    // }

}


void initialize_mobility_tables(){

    init_eval_tables();


    
}
void free_dataset(Dataset * dataset){
    // for (int i = 0; i < dataset->count; i++){
        // if (dataset->entries[i].Xi){
        //     free(dataset->entries[i].Xi);
        // }
    // }
    if (dataset->entries){
        free(dataset->entries);
    }
    dataset->entries = NULL;
    dataset->count = dataset->capacity = 0;
}

int parse_dataset(const char * path, Dataset * dataset){

    FILE * f = NULL;
    f = fopen(path, "r");
    if (!f){
        printf("NO FILE FOUND FOR DATASET\n");
        return -1;
    }
    if (dataset->entries == NULL){
        dataset->entries = malloc(sizeof(DatasetEntry) * 1000);
        dataset->capacity = 1000;
        dataset->count = 0;
        
    }
    
    char line[1024];
    while (fgets(line, 1023, f)){
        // if (dataset->count > 5000000) return 1;
        if (dataset->count + 1 >= dataset->capacity){
            dataset->capacity = (dataset->capacity + 2) * 2;
            dataset->entries = realloc(dataset->entries, sizeof(DatasetEntry) * dataset->capacity);
        }
        assert(strlen(line) > 0);

        // char fen[MAX_FEN];
        // our datasets have two dashes before results
        DatasetEntry * e = &dataset->entries[dataset->count];
        char * fen_str = strtok(line, "[");
        if (fen_str){
            strncpy(e->fen, fen_str, MAX_FEN);
            fen_str = strtok(NULL, "\n");
            if (fen_str){
                float res = strtod(fen_str, NULL);
                e->result = res;
                // printf("FEN: %s\n",  e->fen);

                // printf("RESULT FOUND: %f\n",res);
            }
        }
        // char * fen_str = strtok(line, "\"");
        // if (fen_str){
        //     strcpy(e->fen, fen_str);
        //     fen_str = strtok(NULL, "\"\n");
        //     if (fen_str){
        //         if (strcmp(fen_str, "0-1") == 0){
        //             e->result = 0;
        //             // printf("RESULT 0 FOR %s\n", fen_str);
        //         } else if (strcmp(fen_str, "1-0") == 0){
        //             e->result = 1;
        //             // printf("RESULT 1 FOR %s\n", fen_str);
        //         } else {
        //             e->result = 0.5;
        //             // printf("RESULT .5 FOR %s\n", fen_str);
        //         }
        //         // float res = strtod(fen_str, NULL);
        //         // e->result = res;
        //         // printf("FEN: %s\n",  e->fen);

        //         // printf("RESULT FOUND: %f\n",res);
        //     }
        // }
        dataset->count+=1;
    }

    return 1;
}

void print_tpsqt(double *psqt, ParamIndex * pi) {
    const char* colorName[2] = {"WHITE", "BLACK"};
    const char* pieceName[6] = {"P", "N", "B", "R", "Q", "K"};
        for(int p = 0; p < PIECE_TYPES; p++) {
            printf("Piece %s:\n", pieceName[p]);
            for (int i =0; i < 64; i++){
                if (i % 8 == 0){
                    printf("\n");
                }
                
                printf("%8.3f ", psqt[pi->psqt[p][i]]);
            }
            printf("\n");
        }
}
void load_raw_weights(double * W_raw_mg, double * W_raw_eg, int count, const char * path){
    if (count == 0) return;
    
    FILE *f = fopen(path, "rb");
    if (!f) return;
    size_t mg = fread(W_raw_mg, sizeof(double), count, f);
    size_t eg = fread(W_raw_eg, sizeof(double), count, f);
    fclose(f);
}

void shuffle_dataset(Dataset *ds) {
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned)time(NULL));
        seeded = 1;
    }

    for (int i = ds->count - 1; i > 0; i--) {
        int j = rand() % (i + 1);

        DatasetEntry tmp = ds->entries[i];
        ds->entries[i] = ds->entries[j];
        ds->entries[j] = tmp;
    }
}

void init_param_index(ParamIndex * pi){


    int k = 0;
    for (int i = 0; i < PIECE_TYPES; i++){
        pi->piece_values[i] = k++; 
    }
    for (int i = 0; i < PIECE_TYPES; i++){
        for (int c = 0; c < BOARD_MAX; c++){
            pi->psqt[i][c] = k++;
        }
    }

    for (int i = 0; i < 8; i++){
        pi->isolated_pawn[i] = k++; 
    }
    for (int i = 0; i < 8; i++){
        pi->passed_isolated_pawn[i] = k++; 
    }
    for (int i = 0; i < 8; i++){
        pi->passed_pawn[i] = k++; 
    }
    pi->connected_passer = k++; 
    for (int i = 0; i < 8; i++){
        pi->backward_pawn[i] = k++; 
    }
    for (int i = 0; i < 8; i++){
        pi->candidate_passer[i] = k++; 
    }
    pi->chained_pawn = k++; 
    pi->doubled_pawn = k++; 

    for (int p = 0; p < PIECE_TYPES; p++){
        for (int i = 0; i < 64; i++){
            pi->mobility[p][i] = k++;
        }
    }

    pi->bishop_pair = k++;
    for (int i = 0; i < 8; i++){
        pi->bishop_pawn_color[i] = k++; 
    }
    // with an attacker that can attack the blocking pawn's color
    for (int i = 0; i < 8; i++){
        pi->bishop_pawn_color_w_attacker[i] = k++; 
    }
    for (int i = 0; i < 8; i++){
        pi->defended_pc[i] = k++; 
    }
    pi->outpost_occ = k++;
    pi->outpost_control = k++;
    pi->minor_cannot_enter_et = k++;
    pi->slider_boxed_in = k++;
    pi->unstoppable_attack = k++; 
    pi->rook_open = k++;
    pi->rook_semi_open = k++;
    pi->connected_rook = k++;
    pi->qr_connected = k++;
    pi->qb_connected = k++;
    pi->weak_queen = k++;

    for (int i = 0; i < 16; i++){
        pi->tdp_oed_pen[i] = k++;
    }
    for (int i = 0; i < 16; i++){
        pi->tdp_weak_sq[i] = k++;
    }
    for (int i = 0; i < 16; i++){
        pi->unsafe[i] = k++;
    }
    for (int i = 0; i < 8; i++){
        pi->awp[i] = k++;
    }
    for (int i = 0; i < 16; i++){
        pi->safe_pawn_attacks[i] = k++;
    }
    for (int i = 0; i < 16; i++){
        pi->safe_pawn_push_attacks[i] = k++;
    }
    for (int p = 0; p < PIECE_TYPES; p++){
        pi->threat_by_major[p] = k++;
    }
    for (int p = 0; p < PIECE_TYPES; p++){
        pi->threat_by_minor[p] = k++;
    }
    pi->weak_pieces = k++;
    pi->passer_is_unstoppable = k++;
    pi->passer_deficit = k++;
    pi->passer_supported = k++;
    pi->passer_blocked = k++;
    pi->opponent_stuck_in_front_of_passer = k++;
    pi->opponent_rook_stuck_on_promo_rank = k++;
    pi->rook_on_passer_file = k++;
    pi->rook_stuck_in_front_of_passer = k++;
    pi->extra_passers_on_seven = k++;
    for (int p = 0; p < PIECE_TYPES; p++){
        pi->ks_safe_checks[p] = k++;
    }
    for (int p = 0; p < PIECE_TYPES; p++){
        pi->ks_unsafe_checks[p] = k++;
    }
    for (int p = 0; p < 16; p++){
        pi->ks_double_safe_king_attacks[p] = k++;
    }
    for (int p = 0; p < 16; p++){
        pi->ks_safe_king_attacks[p] = k++;
    }
    for (int p = 0; p < PIECE_TYPES; p++){
        pi->ks_threat_correction[p] = k++;
    }
    pi->ks_rook_contact_check = k++;
    pi->ks_queen_contact_check = k++;
    for (int p = 0; p < PIECE_TYPES; p++){
        pi->ks_akz[p] = k++;
    }
    for (int p = 0; p < 8; p++){
        pi->ks_aikz[p] = k++;
    }
    pi->ks_weak = k++;
    for (int p = 0; p < 24; p++){
        pi->ks_defended_squares_kz[p] = k++;
    }
    for (int p = 0; p < 8; p++){
        pi->ks_def_minor[p] = k++;
    }
    for (int p = 0; p < 8; p++){
        pi->ks_ek_defenders[p] = k++;
    }
    for (int p = 0; p < 3; p++){
        pi->ks_pawn_shield[p] = k++;
    }
    for (int p = 0; p < 8; p++){
        pi->ks_empty[p] = k++;
    }


    
    
    pi->total_params = k;
}

void extract_psqt_features(Game * game, int8_t * Xi, ParamIndex * pi){
    for (int c = 0; c < COLOR_MAX; c++){
        for (int p = PAWN; p < PIECE_TYPES; p++){
            uint8_t * pl = game->piece_list[c][p];
            int a = 0;
            for (uint8_t pos = *pl; pos != SQ_NONE; pos = *++pl){
                Xi[pi->piece_values[p]] += c ? 1 : -1;
                Xi[pi->psqt[p][c ? pos : flip_square(pos)]] += c ? 1 : -1;
                a++;
            }
            ASSERT(a == game->piece_count[c][p]);
        }
        
    }
}

void extract_material(Game * game, Side side, int8_t * Xi, ParamIndex * pi, EvalMasks * masks, uint64_t piece_attacks[COLOR_MAX][PIECE_TYPES][PIECE_MAX]){
    
    int sign = side ? 1 : -1;
    uint64_t fp = game->board_pieces[side];
    uint64_t ep = game->board_pieces[!side];
    int mg = 0, eg = 0;
    int t_mg = 0, t_eg = 0;
    int mob_mg = 0, mob_eg = 0;
    int phase = game->phase;
    uint64_t fpawns = game->pieces[side][PAWN];
    uint64_t epawns = game->pieces[!side][PAWN];
    int enemy_king_sq = __builtin_ctzll(game->pieces[!side][KING]);
    bool has_dark_square_bishop = false;
    bool has_light_square_bishop = false;
    bool enemy_dark_square_bishop = false;
    bool enemy_light_square_bishop = false;
    int bc = 0;
    if (game->pieces[side][BISHOP] & COLOR_SQUARES[BLACK]){
        has_dark_square_bishop = true;
        bc += 1;
    }
    if (game->pieces[side][BISHOP] & COLOR_SQUARES[WHITE]){
        has_light_square_bishop = true;
        bc += 1;
    }
    if (game->pieces[!side][BISHOP] & COLOR_SQUARES[BLACK]){
        enemy_dark_square_bishop = true;
    }
    if (game->pieces[!side][BISHOP] & COLOR_SQUARES[WHITE]){
        enemy_light_square_bishop = true;
    }

    int dark_square_pawns = __builtin_popcountll(game->pieces[side][PAWN] & COLOR_SQUARES[BLACK]);
    int light_square_pawns = __builtin_popcountll(game->pieces[side][PAWN] & COLOR_SQUARES[WHITE]);


    if (bc == 2){
        Xi[pi->bishop_pair] += 1 * sign;
    }

    uint64_t pfsq = game->pieces[side][PAWN] & (side ? game->board_pieces[BOTH] << 8 : game->board_pieces[BOTH] >> 8);
    



    uint64_t out = masks->wsq[!side] & center_squares_mask & ~masks->sppa[!side];
    for (int p = KNIGHT; p < KING; p++){
        uint8_t * pl = game->piece_list[side][p];
        for (uint8_t pos = *pl; pos != SQ_NONE; pos = *++pl){
         uint64_t m = piece_attacks[side][p][game->piece_index[pos]];


         uint64_t mob = m & ~masks->am_p[!side][PAWN] & ~pfsq;
         uint64_t smob = m & ~masks->am[!side] & ~masks->tdp[!side];
         int count = __builtin_popcountll(mob);
         uint64_t b = bits[pos];

         if (p == BISHOP || p == KNIGHT){
             int file = SQ_TO_FILE[pos];
             if (p == KNIGHT){
                 if (b & out){
                     Xi[pi->outpost_occ] += 1 * sign;
                 }

             }
             if (m & out){
                 Xi[pi->outpost_control] += 1 * sign;
             }
             if (!(m & ET3[side])){
                 Xi[pi->minor_cannot_enter_et] += 1 * sign;
             }
         }

         Xi[pi->mobility[p][count]] += sign * 1;

        if (smob == 0 && (masks->tdp[side] & b)){
            Xi[pi->unstoppable_attack] += 1 * sign;
        }


        bool piece_attacked = b & masks->am[!side];
        if (p == ROOK){

            int file = SQ_TO_FILE[pos];
            int rank = SQ_TO_RANK[pos];


            if (!(fpawns & file_masks[file])) {
                if (!(epawns & file_masks[file])) {
                    Xi[pi->rook_open] += 1 * sign;
                } else {
                    Xi[pi->rook_semi_open] += 1 * sign;
                }
            }
            if (m & game->pieces[side][ROOK]){
                Xi[pi->connected_rook] += 1 * sign;

            }
        } else if (p == QUEEN){

            if (rook_moves[pos] & m & game->pieces[side][ROOK]){
                Xi[pi->qr_connected] += 1 * sign;
            }
            if (bishop_moves[pos] & m & game->pieces[side][BISHOP]){
                Xi[pi->qb_connected] += 1 * sign;
            }

            uint64_t qp;
            if (slider_blockers(game, (Side)!side, game->pieces[!side][ROOK] | game->pieces[!side][BISHOP], pos, &qp)){
                Xi[pi->weak_queen] += 1 * sign;
            }
        }
    }

}
    
}

void extract_pawn_structure(Game * game, Side side, int8_t * Xi, ParamIndex * pi, EvalMasks * masks){
    int sign = side ? 1 : -1;
    uint64_t pawns = game->pieces[side][PAWN];
    uint64_t friendly_pawns = game->pieces[side][PAWN];
    uint64_t enemy_pawns = game->pieces[!side][PAWN];
    for (int i = 0; i < 8; i++){
        int count = __builtin_popcountll(pawns & file_masks[i]);
        // doubled pawn
        if (count > 1) Xi[pi->doubled_pawn] += (count - 1) * sign;
    }
    uint64_t start_rank = 0;
    if (side == WHITE) {
        start_rank = rank_masks[1];
    } else {
        start_rank = rank_masks[6];
    }

    uint64_t ra = !side ? (pawns << 7 & ~file_masks[7]) : pawns >> 9 & ~file_masks[7];
    uint64_t la = !side ? (pawns << 9 & ~file_masks[0]) : pawns >> 7 & ~file_masks[0];
    uint64_t attacks = ra | la;
    masks->am_p[side][PAWN] = attacks;

    

    while (pawns){
        int pos = __builtin_ctzll(pawns);
        pawns = pawns & (pawns - 1);

        
        int file = SQ_TO_FILE[pos];
        int rank = side ? SQ_TO_RANK[pos] : SQ_TO_RANK[flip_square(pos)];
        bool enemy_pawn_on_file = enemy_pawns & in_front_file_masks[side][pos];
        bool enemy_pawn_in_front_adjacent = enemy_pawns & adjacent_in_front_masks[side][pos];
        bool is_passer = false;

        // passers
        if (!(game->pieces[!side][PAWN] & passed_pawn_masks[side][pos]) && !enemy_pawn_on_file) {
            Xi[pi->passed_pawn[rank]] += 1 * sign;
            is_passer = true;
            masks->passers[side] |= bits[pos];
        }
        // isolated
        if(!(game->pieces[side][PAWN] & adjacent_file_masks[file])) {
                Xi[pi->isolated_pawn[rank]] += 1 * sign;
                

        }
        // backward pawn
        bool has_supporting_adjacent = (friendly_pawns & (adjacent_file_masks[file] & in_front_ranks_masks[side][pos]));


        // check if a forward square is is controlled by an attacking enemy pawn
        int forward_sq = push_direction[side] * 8 + pos;
        bool enemy_controls_front = false;
        
        if (forward_sq >= 0 && forward_sq < 64){
            // from our side looking foward diagonally
            enemy_controls_front = pawn_captures[side][forward_sq] & enemy_pawns;
        }

        // is backward
        if (!has_supporting_adjacent && (enemy_controls_front || (bits[forward_sq] & game->pieces[!side][PAWN]))){
            Xi[pi->backward_pawn[rank]] += 1 * sign;
        }

        // candidate passer

        if (!enemy_pawn_on_file && !enemy_pawn_in_front_adjacent && !is_passer){
            Xi[pi->candidate_passer[rank]] += 1 * sign;
        }

        // chained pawn
        if (friendly_pawns & pawn_captures[side][pos]){
            
            Xi[pi->chained_pawn] += 1 * sign;
        }

        
    }
}

void extract_threats(Game * game, Side side, int8_t * Xi, ParamIndex * pi, EvalMasks * masks){
    
    int sign = side ? 1 : -1;
    int t_mg = 0, t_eg = 0;
    uint64_t ep_np = game->board_pieces[!side] & ~game->pieces[!side][PAWN];
    
    // uint64_t ews = masks->tdp[!side] & masks->am[side] & ET2[side];
    // int ews_count = __builtin_popcountll(ews);
    // // Xi[pi->tdp_weak_sq[0]] += ews_count * sign;
    // Xi[pi->tdp_weak_sq[MIN(ews_count, 15)]] += sign;

    // // undefended pieces in enemy territory
    // uint64_t oed = masks->tdp[side] & ET3[side] & game->board_pieces[side] & ~game->pieces[side][PAWN];
    // int oed_count = __builtin_popcountll(oed);
    // Xi[pi->tdp_oed_pen[MIN(oed_count, 15)]] += 1 * sign;
    // // Xi[pi->tdp_oed_pen[0]] += oed_count * sign;

    // uint64_t awp = masks->am[side] & game->pieces[!side][PAWN] & ~masks->am_p[!side][PAWN] & masks->tdp[!side];
    // int awp_count = __builtin_popcountll(awp);
    // Xi[pi->awp[MIN(awp_count, 7)]] += 1 * sign;
    // // Xi[pi->awp[0]] += awp_count * sign;


    uint64_t edef = ep_np & (masks->am_p[!side][PAWN] | masks->datk[!side]);
    uint64_t weak = ep_np & ~edef & masks->tdp[!side];
    
    uint64_t b = (ep_np) & (masks->am_p[side][KNIGHT] | masks->am_p[side][BISHOP]);
    while (b){
        uint8_t pos = __builtin_ctzll(b);
        b = b & (b - 1);

        Xi[pi->threat_by_minor[game->piece_at[pos]]] += 1 * sign;
    }

    b = (ep_np) & (masks->am_p[side][ROOK] | masks->am_p[side][QUEEN]);

    while (b){
        uint8_t pos = __builtin_ctzll(b);
        b = b & (b - 1);

        Xi[pi->threat_by_major[game->piece_at[pos]]] += 1 * sign;
    }

    Xi[pi->defended_pc[MIN(__builtin_popcountll(edef), 7)]] += sign;
    if (more_than_one(weak)){
        Xi[pi->weak_pieces] += sign;
        
    }

    uint8_t pp_c = __builtin_popcountll(masks->sppa[side] & ep_np);
    Xi[pi->safe_pawn_push_attacks[MIN(pp_c, 15)]] += 1 * sign;

    uint64_t sp = game->pieces[side][PAWN] & masks->safe[side];
    uint8_t sp_c = __builtin_popcountll(pawn_attacks(side, sp) & ep_np);
    Xi[pi->safe_pawn_attacks[MIN(sp_c, 15)]] += 1 * sign;
    

}

void extract_king_safety(Game * game, Side side, int8_t * Xi, ParamIndex * pi, EvalMasks * masks, uint64_t piece_attacks[COLOR_MAX][PIECE_TYPES][PIECE_MAX]){
    
    int sign = side ? 1 : -1;
    int mr_mg = 0, mr_eg = 0;
    int enemy_king_sq = game->st->k_sq[!side];
    uint64_t ek = game->pieces[!side][KING];
    uint64_t enemy_king_zone = king_zone_masks[!side][enemy_king_sq];
    int empty = __builtin_popcountll(king_moves[enemy_king_sq] & ~game->board_pieces[BOTH] & ~masks->am[side]);
    int enemy_king_defended_squares = __builtin_popcountll((enemy_king_zone & masks->am_nk[!side]) | (king_moves[enemy_king_sq] & ~masks->am[side]));

    // pawn shield only relevant early
    uint64_t pawn_shield =  game->pieces[!side][PAWN] & pawn_shield_masks[!side][enemy_king_sq];    
    int shield = 0;
    shield = __builtin_popcountll(pawn_shield);


    // TODO use state->check info here, we already have it computed
    uint64_t checks[PIECE_TYPES];
    checks[PAWN] = pawn_captures[!side][enemy_king_sq];
    checks[KNIGHT] = knight_moves[enemy_king_sq];
    uint64_t bm = fetch_bishop_moves(enemy_king_sq, game->board_pieces[BOTH]);
    uint64_t rm = fetch_rook_moves(enemy_king_sq, game->board_pieces[BOTH]);
    checks[BISHOP] = bm;
    checks[ROOK] = rm;
    checks[QUEEN] = bm | rm;
    checks[KING] = 0;
    bool is_stm = side == game->side_to_move;

    uint64_t weak_sq = masks->am[side] & ~masks->datk[!side] & (~masks->am[!side] | masks->am_p[!side][QUEEN] | masks->am_p[!side][KING]);
    uint64_t safe = masks->tdp[!side];
    
    int def_score = 0;

    int attacks = 0;
    uint64_t safe_attacks = weak_sq & enemy_king_zone;
    int sa = __builtin_popcountll(safe_attacks);
    uint64_t double_safe = masks->datk[side] & weak_sq & enemy_king_zone;
    int dsafe = __builtin_popcountll(double_safe);
    sa -= dsafe;

    // safe attacks are defined as on enemy tdp, which is attacked but not defended or double attacked but not double defended
    Xi[pi->ks_safe_king_attacks[MIN(sa, 15)]] += 1 * sign;
    Xi[pi->ks_double_safe_king_attacks[MIN(__builtin_popcountll(double_safe), 15)]] += 1 * sign;
    
    // checks and contact checks
    uint64_t pcheck = checks[PAWN] & masks->am_p[side][PAWN];
    if (pcheck){
        if (pcheck & safe){
            Xi[pi->ks_safe_checks[PAWN]] += 1 * sign;
        } else {
            
            Xi[pi->ks_unsafe_checks[PAWN]] += 1 * sign;
        }
    }
    uint64_t ncheck = checks[KNIGHT] & masks->am_p[side][KNIGHT];
    if (ncheck){
        if (ncheck & safe){
            Xi[pi->ks_safe_checks[KNIGHT]] += 1 * sign;
        } else {
            
            Xi[pi->ks_unsafe_checks[KNIGHT]] += 1 * sign;
        }
    }

    uint64_t bcheck = checks[BISHOP] & masks->am_p[side][BISHOP];
    if (bcheck){
        if (bcheck & safe){
            Xi[pi->ks_safe_checks[BISHOP]] += 1 * sign;
        } else {
            
            Xi[pi->ks_unsafe_checks[BISHOP]] += 1 * sign;
        }
    }

    uint64_t rcheck = checks[ROOK] & masks->am_p[side][ROOK];
    if (rcheck){
        if (rcheck & safe){
            Xi[pi->ks_safe_checks[ROOK]] += 1 * sign;
        } else {
            
            Xi[pi->ks_unsafe_checks[ROOK]] += 1 * sign;
        }
        if (rcheck & king_moves[enemy_king_sq] & (~masks->am_nk[!side] & masks->am[side])){
            Xi[pi->ks_rook_contact_check] += 1 * sign;
        }
    }
    uint64_t qcheck = checks[QUEEN] & masks->am_p[side][QUEEN];
    if (qcheck){
        if (qcheck & safe){
            Xi[pi->ks_safe_checks[QUEEN]] += 1 * sign;
        } else {
           
            Xi[pi->ks_unsafe_checks[QUEEN]] += 1 * sign;
        }
        uint64_t qm = qcheck & king_moves[enemy_king_sq];
        if (qm & ((~masks->am_nk[!side] & masks->am[side]) | (qm & masks->am_xr_nq[side]))){
            Xi[pi->ks_queen_contact_check] += 1 * sign;
        }
    }

    // correction for hanging pieces
    // extra correction for hanging pieces in the king zone
    // this handles throwing the queen in front of the king, it is conservative to make sure we dont overshoot, since it isn't infallible
    // historically this uncounted the piece's attack contribution, however this was too slow
    for (int p = KNIGHT; p < KING; p++){
        uint8_t * pl = game->piece_list[side][p];
        for (uint8_t pos = *pl; pos != SQ_NONE; pos = *++pl){
            uint64_t mz = piece_attacks[side][p][game->piece_index[pos]] & enemy_king_zone;
            if (mz){
                if ((bits[pos] & masks->tdp[side] & king_moves[enemy_king_sq]) && side != game->side_to_move){
                    Xi[pi->ks_threat_correction[p]] += 1 * sign;
                } else if (mz & king_moves[enemy_king_sq]){
                    Xi[pi->ks_akz[p]] += 1 * sign;
                }
            }
        }
    }
    int def_minor = __builtin_popcountll((masks->am_p[!side][KNIGHT] | masks->am_p[!side][BISHOP]) & king_moves[enemy_king_sq]);
    uint64_t wksq = masks->datk[side] & ~masks->am_nk[!side] & king_moves[enemy_king_sq];
    int aikz = __builtin_popcountll(game->board_pieces[side] & enemy_king_zone & ~game->pieces[side][PAWN]);


    Xi[pi->ks_aikz[MIN(aikz, 7)]] += 1 * sign;
    Xi[pi->ks_defended_squares_kz[MIN(enemy_king_defended_squares, 23)]] += 1 * sign;
    Xi[pi->ks_def_minor[MIN(def_minor, 7)]] += 1 * sign;
    if (more_than_one(wksq)){
        
        Xi[pi->ks_weak] += 1 * sign;
    }
    Xi[pi->ks_pawn_shield[MIN(shield, 2)]] += 1 * sign;
    Xi[pi->ks_empty[MIN(empty, 7)]] += 1 * sign;
}

void extract_passers(Game * game, Side side, int8_t * Xi, ParamIndex * pi, EvalMasks * masks){
    int sign = side ? 1 : -1;
    uint64_t passers = masks->passers[side];
    int passers_on_seven = 0;
    uint8_t pr_cnt = 0;
    int sc = 0;
    while (passers){
        int pos = __builtin_ctzll(passers);
        passers = passers & (passers - 1);
        
            int rank = SQ_TO_RANK[pos];
            if (rank == (side ? 6 : 1) || (passers_on_seven && side ? rank >= 5 : rank <= 2)){
                passers_on_seven++;
            }
            int file = SQ_TO_FILE[pos];
            int push = rank + push_direction[side];
            uint64_t f_sq = rank_masks[push] & file_masks[file];
            if (f_sq & masks->tdp[side]){
                Xi[pi->passer_deficit] += sign;
            } else if (f_sq & masks->am[side]){
                Xi[pi->passer_supported] += sign;
            }
            if (in_front_file_masks[side][pos] & game->board_pieces[!side]){
                Xi[pi->passer_blocked] += sign;
            }

            // if we are defended and about to promote, we are most likely locking enemy pieces into defending our promotion square. this bonus reflects that
            if (bits[pos] & ~masks->tdp[side] && (rank == 6 || rank == 1)){
                if (promotion_ranks[side] & game->pieces[!side][ROOK]){
                    Xi[pi->opponent_rook_stuck_on_promo_rank] += sign;
                }
                
            }
            uint64_t rf = file_masks[file] & game->pieces[side][ROOK];
            
            if (rf){
                if (rf & ~in_front_file_masks[side][pos]){
                    Xi[pi->rook_on_passer_file] += sign;
                }
                uint8_t rpos = __builtin_ctzll(rf);

                // defer this until after to check for other passers about to promote, since that makes this irrelevant
                if ((f_sq & bits[rpos]) && (rank == 6 || rank == 1)){
                    pr_cnt += 1;
                }
                
            }
    }

    if (passers_on_seven > 1){
        
        Xi[pi->extra_passers_on_seven] += sign;
    }
    if (passers_on_seven <= 1 && pr_cnt) {
        Xi[pi->rook_stuck_in_front_of_passer] += sign;
    }

    
}

void extract_features(Game * game, int8_t * Xi, ParamIndex * pi){
    EvalMasks masks = {0};

    extract_psqt_features(game, Xi, pi);
    
    extract_pawn_structure(game, WHITE, Xi, pi, &masks);
    extract_pawn_structure(game, BLACK, Xi, pi, &masks);
    
    uint64_t piece_attacks[COLOR_MAX][PIECE_TYPES][PIECE_MAX];
    generate_attack_mask_and_eval_mobility(game, WHITE, &masks, piece_attacks);
    generate_attack_mask_and_eval_mobility(game, BLACK, &masks, piece_attacks);

    extract_material(game, WHITE, Xi, pi, &masks, piece_attacks);
    extract_material(game, BLACK, Xi, pi, &masks, piece_attacks);
    extract_threats(game, WHITE, Xi, pi, &masks);
    extract_threats(game, BLACK, Xi, pi, &masks);
    extract_king_safety(game, WHITE, Xi, pi, &masks, piece_attacks);
    extract_king_safety(game, BLACK, Xi, pi, &masks, piece_attacks);
    extract_passers(game, WHITE, Xi, pi, &masks);
    extract_passers(game, BLACK, Xi, pi, &masks);
}

void extract_dataset_to_binary(Game * game, Dataset * dataset, ParamIndex * pi, const char * extraction_path){

    int feat_count = pi->total_params;
    int8_t *Xi= calloc(feat_count, sizeof(int8_t));
    FILE *out = fopen(extraction_path, "wb");

    for (int i = 0; i < dataset->count; i++){
        memset(Xi, 0, feat_count * sizeof(int8_t));
        
        DatasetEntry * e = &dataset->entries[i];
        set_board_to_fen(game, e->fen);
        extract_features(game, Xi, pi);
        e->offset = ftell(out);
        fwrite(Xi, sizeof(int8_t), feat_count, out);

    }
    fclose(out);
    free(Xi);
    
}


void adam(Game *game, Dataset *ds, ParamIndex *pi,
                 double *W_mg, double *W_eg,
                 int epochs, double lr, double beta1, double beta2, double eps, int batch_size, double lambda, const char * name)
{

    const int RESULT_SCALE = 300;
    int feat_count = pi->total_params;
    double *grad_mg = calloc(feat_count, sizeof(double));
    double *grad_eg = calloc(feat_count, sizeof(double));
    int8_t *Xi = calloc(feat_count, sizeof(int8_t));
    double *W_mg_prev = calloc(feat_count, sizeof(double));
    double *W_eg_prev = calloc(feat_count, sizeof(double));

    double *m_mg = calloc(feat_count, sizeof(double));
    double *v_mg = calloc(feat_count, sizeof(double));
    double *m_eg = calloc(feat_count, sizeof(double));
    double *v_eg = calloc(feat_count, sizeof(double));
     clock_t start_time = clock();   
     int dset_cnt = ds->count;
     int t = 0;
     double bias = 0.0;
     int T = 20;
     double lr_max = 0.001;
     double lr_min = 0.0001;
     int tc = 0;

    for (int epoch = 0; epoch < epochs; epoch++) {
        if (tc == T) {
            T *= 2;
            tc = 0;
            lr_max *= 0.8;
            lr_min *= 0.8;

        }
        double local_lr = lr_min + 0.5 * (lr_max - lr_min) * (1 + cos(M_PI * tc / T));
        tc++;

        shuffle_dataset(ds);

        memset(grad_mg, 0, feat_count * sizeof(double));
        memset(grad_eg, 0, feat_count * sizeof(double));

        int batch_pos = 0;

        double ge_mean_mg = 0.0;
        double ge_mean_eg = 0.0;

        for (int k = 0; k < dset_cnt; k++) {
            memset(Xi, 0, feat_count * sizeof(int8_t));

            DatasetEntry *e = &ds->entries[k];

            set_board_to_fen(game, e->fen);
            extract_features(game, Xi, pi);
            double dot_mg = 0.0, dot_eg = 0.0;

            double phase = (float)game->phase / MAX_PHASE;

            for (int i = 0; i < feat_count; i++) {
                dot_mg += W_mg[i] * Xi[i];
                dot_eg += W_eg[i] * Xi[i];
            }

            double eval = phase * dot_mg + (1.0 - phase) * dot_eg;
            double p = 1.0 / (1.0 + exp(-eval));  
            // assumes result is 0 - 1
            double diff = (p - e->result); 
            
            for (int i = 0; i < feat_count; i++) {
                grad_mg[i] += diff * phase * Xi[i];
                grad_eg[i] += diff * (1.0 - phase) * Xi[i];
            }

            batch_pos++;

            if (batch_pos == batch_size || k == dset_cnt - 1) {

                t += 1;
                double bcorr1 = pow(beta1, t);
                double bcorr2 = pow(beta2, t);
                double inv_bs = 1.0 / (double)batch_pos;
            
                for (int i = 0; i < feat_count; i++) {
                    double gmg = grad_mg[i] * inv_bs;
                    double geg = grad_eg[i] * inv_bs;

                    m_mg[i] = beta1 * m_mg[i] + (1.0 - beta1) * gmg;
                    v_mg[i] = beta2 * v_mg[i] + (1.0 - beta2) * (gmg * gmg);

                    double mcorr_mg = m_mg[i] / (1.0 - bcorr1);
                    double vcorr_mg = v_mg[i] / (1.0 - bcorr2);

                    // double lt = local_lr * sqrt(1 - bcorr2) / (1 - bcorr1);
                    // W_mg[i] -= lt * (m_mg[i] / (sqrt(v_mg[i]) + eps) + lambda * W_mg[i]);

                    W_mg[i] -= local_lr * (mcorr_mg / (sqrt(vcorr_mg) + eps) + lambda * W_mg[i]);

                    m_eg[i] = beta1 * m_eg[i] + (1.0 - beta1) * geg;
                    v_eg[i] = beta2 * v_eg[i] + (1.0 - beta2) * (geg * geg);

                    double mcorr_eg = m_eg[i] / (1.0 - bcorr1);
                    double vcorr_eg = v_eg[i] / (1.0 - bcorr2);

                    // W_eg[i] -= lt * (m_eg[i] / (sqrt(v_eg[i]) + eps) + lambda * W_eg[i]);
                    W_eg[i] -= local_lr * (mcorr_eg / (sqrt(vcorr_eg) + eps) + lambda * W_eg[i]);
                    

                }

                memset(grad_mg, 0, feat_count * sizeof(double));
                memset(grad_eg, 0, feat_count * sizeof(double));
                batch_pos = 0;
            }
        }

        double delta_sum = 0.0;
        double magnitude = 0.0;
        for (int i = 0; i < feat_count; i++) {
            double dmg = fabs(W_mg[i] - W_mg_prev[i]);
            double deg = fabs(W_eg[i] - W_eg_prev[i]);
            delta_sum += dmg + deg;
            magnitude += fabs(W_mg[i]) + fabs(W_eg[i]);
        }
        double dmean = delta_sum / feat_count;
        printf("Delta sum epoch %d: %f Delta Mean: %f Magnitude: %f Mean %f\n", epoch, delta_sum, dmean, magnitude, magnitude / feat_count);
        if (epoch % 10 == 0){
            clock_t time = clock();
            int elapsed_time = (double)(time - start_time) / CLOCKS_PER_SEC; 
            char str[1024];
            sprintf(str, "%s_CHECKPOINT %d.bin", name, rand());
            FILE *out = fopen(str, "wb");
            fwrite(W_mg, sizeof(double), feat_count, out);
            fwrite(W_eg, sizeof(double), feat_count, out);
            fclose(out);
            
        }

        print_tpsqt(W_mg, pi);
        print_tpsqt(W_eg, pi);
        print_weights(W_mg, pi);
        print_weights(W_eg, pi);
        printf("Delta sum epoch %d: %f\n", epoch, delta_sum);
        memcpy(W_mg_prev, W_mg, sizeof(double) * feat_count);
        memcpy(W_eg_prev, W_eg, sizeof(double) * feat_count);
    }
    

    free(grad_mg);
    free(grad_eg);
    free(W_mg_prev);
    free(W_eg_prev);
    free(Xi);
    free(m_mg);
    free(m_eg);
    free(v_mg);
    free(v_eg);

    

}



void sgd(Game *game, Dataset *ds, ParamIndex *pi,
                 double *W_mg, double *W_eg,
                 int epochs, double lr,
                 int batch_size, double lambda, const char * name)
{
    const int RESULT_SCALE = 650;
    int feat_count = pi->total_params;
    double *grad_mg = calloc(feat_count, sizeof(double));
    double *grad_eg = calloc(feat_count, sizeof(double));
    int8_t *Xi = calloc(feat_count, sizeof(int8_t));
    double *W_mg_prev = calloc(feat_count, sizeof(double));
    double *W_eg_prev = calloc(feat_count, sizeof(double));
    double *M_mg = calloc(feat_count, sizeof(double));
    double *M_eg = calloc(feat_count, sizeof(double));

    double bias = 0.0;
     clock_t start_time = clock();   
     int M = ds->count;

     int T = 10;
     const double mu = 0.9;
     double lr_max = 0.01;
     double lr_min = 0.001;
     int tc = 0;


    for (int epoch = 0; epoch < epochs; epoch++) {
        if (tc == T) {
            T *= 2;
            tc = 0;
            lr_max *= 0.95;
            lr_min *= 0.95;

            memset(M_mg, 0, feat_count * sizeof(double));
            memset(M_eg, 0, feat_count * sizeof(double));
        }
        double local_lr = lr_min + 0.5 * (lr_max - lr_min) * (1 + cos(M_PI * tc / T));
        tc++;

        shuffle_dataset(ds);

        memset(grad_mg, 0, feat_count * sizeof(double));
        memset(grad_eg, 0, feat_count * sizeof(double));
        double grad_bias = 0.0;

        int batch_pos = 0;

        for (int k = 0; k < M; k++) {
            memset(Xi, 0, feat_count * sizeof(int8_t));

            DatasetEntry *e = &ds->entries[k];

            set_board_to_fen(game, e->fen);
            extract_features(game, Xi, pi);
            double dot_mg = 0.0, dot_eg = 0.0;
            double phase = (float)game->phase / MAX_PHASE;

            for (int i = 0; i < feat_count; i++) {
                dot_mg += W_mg[i] * Xi[i];
                dot_eg += W_eg[i] * Xi[i];
            }

            double eval = phase * dot_mg + (1.0 - phase) * dot_eg;
            double p = 1.0 / (1.0 + exp(-eval));  
            double diff = (p - e->result); 

            for (int i = 0; i < feat_count; i++) {
                grad_mg[i] += diff * phase * Xi[i];
                grad_eg[i] += diff * (1.0 - phase) * Xi[i];
            }

            batch_pos++;

            if (batch_pos == batch_size || k == M - 1) {

                for (int i = 0; i < feat_count; i++) {
                    double gmg = grad_mg[i] / batch_pos + lambda * W_mg[i];
                    double geg = grad_eg[i] / batch_pos + lambda * W_eg[i];
                    M_mg[i] = mu * M_mg[i] + gmg;
                    M_eg[i] = mu * M_eg[i] + geg;
                    W_mg[i] -= M_mg[i] * local_lr;
                    W_eg[i] -= M_eg[i] * local_lr;
                }
                memset(grad_mg, 0, feat_count * sizeof(double));
                memset(grad_eg, 0, feat_count * sizeof(double));
                batch_pos = 0;
            }
        }

        double delta_sum = 0.0;
        double magnitude = 0.0;
        for (int i = 0; i < feat_count; i++) {
            double dmg = fabs(W_mg[i] - W_mg_prev[i]);
            double deg = fabs(W_eg[i] - W_eg_prev[i]);
            delta_sum += dmg + deg;
            magnitude += W_mg[i] + W_eg[i];
        }
        double dmean = delta_sum / feat_count;
        printf("Delta sum epoch %d: %f Delta Mean: %f Magnitude: %f Mean %f\n", epoch, delta_sum, dmean, magnitude, magnitude / feat_count);

        memcpy(W_mg_prev, W_mg, sizeof(double) * feat_count);
        memcpy(W_eg_prev, W_eg, sizeof(double) * feat_count);
        print_tpsqt(W_mg, pi);
        print_tpsqt(W_eg, pi);
        print_weights(W_mg, pi);
        print_weights(W_eg, pi);
        if (epoch % 10 == 0){
            clock_t time = clock();
            int elapsed_time = (double)(time - start_time) / CLOCKS_PER_SEC; 
            char str[1024];
            sprintf(str, "./tuner/%s_CHECKPOINT %d.bin", name, rand());
            FILE *out = fopen(str, "wb");
            fwrite(W_mg, sizeof(double), pi->total_params, out);
            fwrite(W_eg, sizeof(double), pi->total_params, out);
            fclose(out);
            
        }
    }

    free(grad_mg);
    free(grad_eg);
    free(M_mg);
    free(M_eg);
    free(W_mg_prev);
    free(W_eg_prev);
    free(Xi);
}


void center_psqt(int * psqt, const char * name, int offset){
    printf("CENTERED %s: \n", name);
    int sum = 0;
    for (int i = 0; i < 64; i++){
        sum += psqt[i];
    }
    int mean = sum / 64;
    for (int i = 0; i < 64; i++){
        psqt[i] -= mean;
    }
    for (int i = 0; i < 64; i++){
        if (i % 8 == 0) printf("\n");
        printf("%d, ", psqt[i] + offset);
    }
    printf("\n");
}

void center_psqts(char * path){
    ParamIndex pi;
    init_param_index(&pi);
    double * W_mg = calloc(pi.total_params, sizeof(double));
    double * W_eg = calloc(pi.total_params, sizeof(double));
    load_raw_weights(W_mg, W_eg, pi.total_params, path);
    int * psqt_mg = calloc(pi.total_params, sizeof(int));
    int * psqt_eg = calloc(pi.total_params, sizeof(int));
    for (int i = 0; i < pi.total_params; i++){
        psqt_mg[i] = (int)roundf(W_mg[i]);
        psqt_eg[i] = (int)roundf(W_eg[i]);
    }

    center_psqt(&psqt_mg[pi.psqt[PAWN][0]], "PAWN MG", piece_values_mg[PAWN]);
    center_psqt(&psqt_eg[pi.psqt[PAWN][0]], "PAWN EG", piece_values_eg[PAWN]);
    center_psqt(&psqt_mg[pi.psqt[KNIGHT][0]], "KNIGHT MG", piece_values_mg[KNIGHT]);
    center_psqt(&psqt_eg[pi.psqt[KNIGHT][0]], "KNIGHT EG", piece_values_eg[KNIGHT]);
    center_psqt(&psqt_mg[pi.psqt[BISHOP][0]], "BISHOP MG", piece_values_mg[BISHOP]);
    center_psqt(&psqt_eg[pi.psqt[BISHOP][0]], "BISHOP EG", piece_values_eg[BISHOP]);
    center_psqt(&psqt_mg[pi.psqt[ROOK ][0]], "ROOK MG", piece_values_mg[ROOK ]);
    center_psqt(&psqt_eg[pi.psqt[ROOK ][0]], "ROOK EG", piece_values_eg[ROOK ]);
    center_psqt(&psqt_mg[pi.psqt[QUEEN][0]], "QUEEN MG", piece_values_mg[QUEEN]);
    center_psqt(&psqt_eg[pi.psqt[QUEEN][0]], "QUEEN EG", piece_values_eg[QUEEN]);
    center_psqt(&psqt_mg[pi.psqt[KING][0]], "KING MG", 0);
    center_psqt(&psqt_eg[pi.psqt[KING][0]], "KING EG", 0);
    free(W_mg);
    free(W_eg);
    free(psqt_mg);
    free(psqt_eg);
}

void convert_weights(const char * weights){
    
    const int scale = 200;
    ParamIndex pi;
    init_param_index(&pi);
    double * W_mg = calloc(pi.total_params, sizeof(double));
    double * W_eg = calloc(pi.total_params, sizeof(double));
    load_raw_weights(W_mg, W_eg, pi.total_params, weights);

    int32_t * out = calloc(pi.total_params, sizeof(int32_t));

    for (int i = 0; i < pi.total_params; i++){
        out[i] = make_score((int16_t)round(W_mg[i] * scale), (int16_t)round(W_eg[i]* scale));

        
        dump_score(out[i]);
    }

    print_packed_scores(out, &pi);
    free(W_mg);
    free(W_eg);
    free(out);
    
}

void init_eval_params(const char * weights){
    
    const int scale = 200;
    init_param_index(&ep_idx);
    double * W_mg = calloc(ep_idx.total_params, sizeof(double));
    double * W_eg = calloc(ep_idx.total_params, sizeof(double));
    load_raw_weights(W_mg, W_eg, ep_idx.total_params, weights);

    for (int i = 0; i < ep_idx.total_params; i++){
        eval_params[i] = make_score((int16_t)round(W_mg[i] * scale), (int16_t)round(W_eg[i]* scale));
    }

    for (int p = 0; p < PIECE_TYPES; p++){
        for (int i = 0; i < 64; i++){
            PSQT[WHITE][p][i] = eval_params[ep_idx.psqt[p][i]];
            PSQT[BLACK][p][i] = eval_params[ep_idx.psqt[p][flip_square(i)]];
        }
    }

    free(W_mg);
    free(W_eg);
}


void print_flipped_psqts_weights(){
    
    for (int p = 0; p < PIECE_TYPES; p++){
        printf("P: %c\n", PNAME[p]);
        for (int i = 0; i < 64; i++){
            if (i %8 == 0) printf("\n");
            printf("%d, ", PSQT[WHITE][p][flip_square(i)]);
        }
        printf("\n");
    }
}


void new_tuner(Game * game, const char * path){

    Dataset dataset; 
    dataset.entries = NULL;
    parse_dataset(path, &dataset);
    ParamIndex pi;
    init_param_index(&pi);

    printf("DSET CNT: %d PARAMS: %d\n", dataset.count, pi.total_params);
    double * W_mg = calloc(pi.total_params, sizeof(double));
    double * W_eg = calloc(pi.total_params, sizeof(double));
    // sgd(game, &dataset, &pi, W_mg, W_eg, 500, 0.1, 32, 1e-4, "Jan30-1");
    adam(game, &dataset, &pi, W_mg, W_eg, 500, 0.001, 0.9, 0.999, 1e-8, 120000, 0.001, "Jan31-1");

    FILE *out = fopen("./tuner/Jan31-1.bin", "wb");
    fwrite(W_mg, sizeof(double), pi.total_params, out);
    fwrite(W_eg, sizeof(double), pi.total_params, out);
    fclose(out);
    free_dataset(&dataset);

    free(W_mg);
    free(W_eg);

}









