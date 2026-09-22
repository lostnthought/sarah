#ifndef TUNER_H
#define TUNER_H

// #include "tuner.c"
#include "types.h"


void center_psqts(char * path);
void dump_weights_text(const char *path, const ParamIndex * pi, const double *W_mg, const double *W_eg);

void init_eval_params(const char * weights);
void convert_weights(const char * weights);
void new_tuner(Game * game, const char * path);
void init_texel_weights();
void initialize_mobility_tables();

void print_flipped_psqts_weights();
#endif
