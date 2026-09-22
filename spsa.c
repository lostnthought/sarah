#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <sys/wait.h>


// this is the spsa tuner. it is not multithreaded and does not have shared theta. it is a very simple implementation based on joona's perl script. most values are hardcoded and therefore you'll probably have to recompile it if you intend to use it. however, it is very simple and with basically no dependencies, just -lm -ldl

#define PARAM_MAX 500

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef enum Type{
    UINT8,
    INT16,
    DOUBLE
} Type;


typedef struct Param{
    char name[128];
    double min, max, start, value, a, c, r, ak, rk, ck;
    double e1, e2;
    int delta;
    Type t;

} Param;

Param params[PARAM_MAX];

static inline double now() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}



const double alpha = 0.602;
const double y = 0.101;

int param_count = 0;

// this long function is where we hardcode params in. we also parse txt files for starting values in the same format as we output them so we can reload and start from where we left off. of course we have to just set the iteration number to where we stopped. it's possible to make this a more convenient csv parser but prob would have not saved much time in the end. rk and ck are just estimated, working on eval terms you want them to probably be lower than this, but an rk of 0.02 - 0.05 seems to work good for search since you want the spsa to explore a bit more. read joona's perl script page for more info, the implementation here basically just translates their math to c.
void init_params(){

    param_count = 0;
    params[param_count++] = (Param)
    {
        .name = "eval_scale",
        .min = 0.5,
        .max = 1.5,
        .start = 0.868,
        .ak = 0,
        .ck = 0.075,
        .r = 0,
        .rk = 0.02,
        .t = DOUBLE,
    };
    params[param_count++] = (Param)
    {
        .name = "aspiration_base",
        .min = 20,
        .max = 150,
        .start = 80,
        .ak = 0,
        .ck = 13,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "aspiration_mul",
        .min = 1,
        .max = 50,
        .start = 8,
        .ak = 0,
        .ck = 3,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "qdelta_margin",
        .min = 100,
        .max = 300,
        .start = 263.18,
        .ak = 0,
        .ck = 40,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "qsee_margin",
        .min = -300,
        .max = 0,
        .start = -108.69,
       .ak = 0,
        .ck = 40,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    // params[param_count++] = (Param)
    // {
    //     .name = "check_prune_margin",
    //     .min = 1,
    //     .max = 200,
    //     .start = 49,
    //     .ak = 0,
    //     .ck = 30,
    //     .r = 0,
    //     .rk = 0.02,
    //     .t = INT16,
    // };
    params[param_count++] = (Param)
    {
        .name = "rfp_depth",
        .min = 3,
        .max = 7,
        .start = 4.29,
        .ak = 0,
        .ck = 1,
        .r = 0,
        .rk = 0.05,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "rfp_base",
        .min = 0,
        .max = 80,
        .start = 13.7,
        .ak = 0,
        .ck = 7,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "rfp_mul",
        .min = 10,
        .max = 100,
        .start = 53.88,
        .ak = 0,
        .ck = 10,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "rfp_improving",
        .min = 20,
        .max = 200,
        .start = 84.50,
        .ak = 0,
        .ck = 10,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "lmr_depth",
        .min = 1,
        .max = 7,
        .start = 3,
        .ak = 0,
        .ck = 1,
        .r = 0,
        .rk = 0.05,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "lmr_move_start",
        .min = 0,
        .max = 7,
        .start = 4,
        .ak = 0,
        .ck = 0.5,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "lmr_hd",
        .min = 6000,
        .max = 24000,
        .start = 16834,
        .ak = 0,
        .ck = 100,
        .r = 0,
        .rk = 0.025,
        .t = INT16,
    };
    // params[param_count++] = (Param)
    // {
    //     .name = "lmr_cap_mul",
    //     .min = 0.07,
    //     .max = 0.51,
    //     .start = 0.219,
    //     .ak = 0,
    //     .ck = 0.1,
    //     .r = 0,
    //     .rk = 0.02,
    //     .t = DOUBLE,
    // };
    // params[param_count++] = (Param)
    // {
    //     .name = "lmr_cap_base",
    //     .min = 0.01,
    //     .max = 0.3,
    //     .start = 0.05,
    //     .ak = 0,
    //     .ck = 0.1,
    //     .r = 0,
    //     .rk = 0.02,
    //     .t = DOUBLE,
    // };
    params[param_count++] = (Param)
    {
        .name = "lmr_quiet_mul",
        .min = 0.25,
        .max = 0.7,
        .start = 0.5326,
        .ak = 0,
        .ck = 0.1,
        .r = 0,
        .rk = 0.02,
        .t = DOUBLE,
    };
    params[param_count++] = (Param)
    {
        .name = "lmr_quiet_base",
        .min = 0.6,
        .max = 1.5,
        .start = 0.879,
        .ak = 0,
        .ck = 0.1,
        .r = 0,
        .rk = 0.02,
        .t = DOUBLE,
    };
    params[param_count++] = (Param)
    {
        .name = "lmp_depth",
        .min = 3,
        .max = 8,
        .start = 5,
        .ak = 0,
        .ck = 1,
        .r = 0,
        .rk = 0.05,
        .t = UINT8,
    };
    params[param_count++] = (Param)
    {
        .name = "lmp_base",
        .min = 5,
        .max = 10,
        .start = 7.51,
        .ak = 0,
        .ck = 0.5,
        .r = 0,
        .rk = 0.02,
        .t = UINT8,
    };
    params[param_count++] = (Param)
    {
        .name = "lmp_improving",
        .min = 0.5,
        .max = 2,
        .start = 0.97,
        .ak = 0,
        .ck = 0.1,
        .r = 0,
        .rk = 0.02,
        .t = DOUBLE,
    };
    params[param_count++] = (Param)
    {
        .name = "lmp_depth_pow",
        .min = 1.3,
        .max = 2.5,
        .start = 1.79,
        .ak = 0,
        .ck = 0.1,
        .r = 0,
        .rk = 0.05,
        .t = DOUBLE,
    };
    params[param_count++] = (Param)
    {
        .name = "futility_depth",
        .min = 2,
        .max = 6,
        .start = 3.96,
        .ak = 0,
        .ck = 1,
        .r = 0,
        .rk = 0.05,
        .t = UINT8,
    };
    params[param_count++] = (Param)
    {
        .name = "futility_base",
        .min = 125,
        .max = 375,
        .start = 197.4,
        .ak = 0,
        .ck = 30,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "futility_mul",
        .min = 125,
        .max = 325,
        .start = 217.01,
        .ak = 0,
        .ck = 30,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "futility_hist_mul",
        .min = 20,
        .max = 150,
        .start = 42.77,
        .ak = 0,
        .ck = 25,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "futility_improving",
        .min = 20,
        .max = 150,
        .start = 120.31,
        .ak = 0,
        .ck = 25,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "razor_depth",
        .min = 1,
        .max = 5,
        .start = 2.75,
        .ak = 0,
        .ck = 1,
        .r = 0,
        .rk = 0.05,
        .t = UINT8,
    };
    params[param_count++] = (Param)
    {
        .name = "razor_base",
        .min = 50,
        .max = 300,
        .start = 177.95,
        .ak = 0,
        .ck = 30,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "razor_mul",
        .min = 50,
        .max = 200,
        .start = 151.49,
        .ak = 0,
        .ck = 25,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "razor_improving",
        .min = 50,
        .max = 200,
        .start = 74.31,
        .ak = 0,
        .ck = 25,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "nmp_base",
        .min = 1,
        .max = 250,
        .start = 1.38,
        .ak = 0,
        .ck = 20,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "nmp_mul",
        .min = 600,
        .max = 1000,
        .start = 770,
        .ak = 0,
        .ck = 50,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "nmp_slope",
        .min = 150,
        .max = 400,
        .start = 327,
        .ak = 0,
        .ck = 20,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "probcut_depth",
        .min = 4,
        .max = 8,
        .start = 4.98,
        .ak = 0,
        .ck = 1,
        .r = 0,
        .rk = 0.05,
        .t = UINT8,
    };
    params[param_count++] = (Param)
    {
        .name = "probcut_improving",
        .min = 1,
        .max = 200,
        .start = 7.91,
        .ak = 0,
        .ck = 20,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "probcut_base",
        .min = 100,
        .max = 350,
        .start = 211.81,
        .ak = 0,
        .ck = 30,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "iid_depth",
        .min = 3,
        .max = 10,
        .start = 5,
        .ak = 0,
        .ck = 1,
        .r = 0,
        .rk = 0.05,
        .t = UINT8,
    };
    // params[param_count++] = (Param)
    // {
    //     .name = "chist_depth",
    //     .min = 1,
    //     .max = 5,
    //     .start = 1.37,
    //     .ak = 0,
    //     .ck = 0.5,
    //     .r = 0,
    //     .rk = 0.02,
    //     .t = UINT8,
    // };
    // params[param_count++] = (Param)
    // {
    //     .name = "chist1_margin",
    //     .min = -24000,
    //     .max = 0,
    //     .start = -475,
    //     .ak = 0,
    //     .ck = 30,
    //     .r = 0,
    //     .rk = 0.02,
    //     .t = INT16,
    // };
    // params[param_count++] = (Param)
    // {
    //     .name = "chist2_margin",
    //     .min = -24000,
    //     .max = 0,
    //     .start = -501,
    //     .ak = 0,
    //     .ck = 30,
    //     .r = 0,
    //     .rk = 0.02,
    //     .t = INT16,
    // };
    params[param_count++] = (Param)
    {
        .name = "mp_goodcap_margin",
        .min = -150,
        .max = 0,
        .start = -39.16,
        .ak = 0,
        .ck = 15,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "chist1_scale",
        .min = 0.5,
        .max = 5,
        .start = 1,
        .ak = 0,
        .ck = 0.5,
        .r = 0,
        .rk = 0.02,
        .t = DOUBLE,
    };
    params[param_count++] = (Param)
    {
        .name = "chist2_scale",
        .min = 0.5,
        .max = 5,
        .start = 1.5,
        .ak = 0,
        .ck = 0.5,
        .r = 0,
        .rk = 0.02,
        .t = DOUBLE,
    };
    params[param_count++] = (Param)
    {
        .name = "chist4_scale",
        .min = 0.5,
        .max = 6,
        .start = 2.2,
        .ak = 0,
        .ck = 0.5,
        .r = 0,
        .rk = 0.02,
        .t = DOUBLE,
    };
    params[param_count++] = (Param)
    {
        .name = "chist6_scale",
        .min = 0.5,
        .max = 6,
        .start = 3,
        .ak = 0,
        .ck = 0.5,
        .r = 0,
        .rk = 0.02,
        .t = DOUBLE,
    };
    params[param_count++] = (Param)
    {
        .name = "see_depth",
        .min = 2,
        .max = 6,
        .start = 2.74,
        .ak = 0,
        .ck = 1,
        .r = 0,
        .rk = 0.05,
        .t = UINT8,
    };
    params[param_count++] = (Param)
    {
        .name = "see_quiet_margin",
        .min = -120,
        .max = 0,
        .start = -51.76,
        .ak = 0,
        .ck = 15,
        .r = 0,
        .rk = 0.03,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "see_nonquiet_margin",
        .min = -220,
        .max = -35,
        .start = -79.82,
        .ak = 0,
        .ck = 15,
        .r = 0,
        .rk = 0.03,
        .t = INT16,
    };
 params[param_count++] = (Param)
 {
     .name = "se_depth",
     .min = 5,
     .max = 10,
     .start = 7.51,
     .ak = 0,
     .ck = 1,
     .r = 0,
     .rk = 0.05,
     .t = UINT8,
 };
 params[param_count++] = (Param)
 {
     .name = "se_depth_margin",
     .min = 1,
     .max = 4,
     .start = 1.35,
     .ak = 0,
     .ck = 1,
     .r = 0,
     .rk = 0.02,
     .t = DOUBLE,
 };
    params[param_count++] = (Param)
    {
        .name = "qhistory_mul",
        .min = 80,
        .max = 250,
        .start = 157.46,
        .ak = 0,
        .ck = 20,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "qhistory_base",
        .min = 80,
        .max = 250,
        .start = 94.98,
        .ak = 0,
        .ck = 40,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "qhpen_mul",
        .min = 80,
        .max = 250,
        .start = 173.45,
        .ak = 0,
        .ck = 40,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "qhpen_base",
        .min = 80,
        .max = 250,
        .start = 100,
        .ak = 0,
        .ck = 40,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "chistory_mul",
        .min = 80,
        .max = 250,
        .start = 129.59,
        .ak = 0,
        .ck = 40,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "chistory_base",
        .min = 80,
        .max = 250,
        .start = 166.18,
        .ak = 0,
        .ck = 40,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "chpen_mul",
        .min = 80,
        .max = 250,
        .start = 138.34,
        .ak = 0,
        .ck = 40,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "chpen_base",
        .min = 80,
        .max = 250,
        .start = 82.67,
        .ak = 0,
        .ck = 40,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "beta_bonus",
        .min = 1,
        .max = 150,
        .start = 28.05,
        .ak = 0,
        .ck = 20,
        .r = 0,
        .rk = 0.02,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "corrhist_grain",
        .min = 1,
        .max = 1024,
        .start = 296.79,
        .ak = 0,
        .ck = 30,
        .r = 0,
        .rk = 0.025,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "corrhist_weight",
        .min = 1,
        .max = 1024,
        .start = 243.60,
        .ak = 0,
        .ck = 30,
        .r = 0,
        .rk = 0.025,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "corrhist_max",
        .min = 1,
        .max = 16384,
        .start = 8130.03,
        .ak = 0,
        .ck = 128,
        .r = 0,
        .rk = 0.025,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "corr_depth_base",
        .min = 0,
        .max = 5,
        .start = 0.752,
        .ak = 0,
        .ck = 0.5,
        .r = 0,
        .rk = 0.025,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "corr_ch_weight",
        .min = 1,
        .max = 1024,
        .start = 206.585,
        .ak = 0,
        .ck = 50,
        .r = 0,
        .rk = 0.025,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "corr_mat_weight",
        .min = 1,
        .max = 1024,
        .start = 107.94,
        .ak = 0,
        .ck = 50,
        .r = 0,
        .rk = 0.025,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "corr_np_weight",
        .min = 1,
        .max = 1024,
        .start = 208.44,
        .ak = 0,
        .ck = 50,
        .r = 0,
        .rk = 0.025,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "corr_pawn_weight",
        .min = 1,
        .max = 1024,
        .start = 84.79,
        .ak = 0,
        .ck = 50,
        .r = 0,
        .rk = 0.025,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "corr_kbn_weight",
        .min = 1,
        .max = 1024,
        .start = 319.54,
        .ak = 0,
        .ck = 50,
        .r = 0,
        .rk = 0.025,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "corr_kqr_weight",
        .min = 1,
        .max = 1024,
        .start = 274.92,
        .ak = 0,
        .ck = 50,
        .r = 0,
        .rk = 0.025,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "l1",
        .min = 1000,
        .max = 2000,
        .start = 1500,
        .ak = 0,
        .ck = 55,
        .r = 0,
        .rk = 0.025,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "l2",
        .min = 400,
        .max = 1000,
        .start = 700,
        .ak = 0,
        .ck = 40,
        .r = 0,
        .rk = 0.025,
        .t = INT16,
    };
    params[param_count++] = (Param)
    {
        .name = "l3",
        .min = 100,
        .max = 400,
        .start = 250,
        .ak = 0,
        .ck = 30,
        .r = 0,
        .rk = 0.025,
        .t = INT16,
    };
}


int first_in[2];
int first_out[2];
int second_in[2];
int second_out[2];
pid_t pid_first;
pid_t pid_second;

char fens[10000][87];


// i have really no clue how piping works so not really sure if this is 100% safe but I haven't run into any problems with long long tuning runs for more than 40 hours on my very bad laptop
void start_engine(FILE ** fin, FILE ** fout, FILE ** sin, FILE ** sout){
    
    pipe(first_in);
    pipe(first_out);
    pid_first = fork();
    if (pid_first == 0){
        dup2(first_in[0], STDIN_FILENO);
        dup2(first_out[1], STDOUT_FILENO);

        close(first_in[1]);
        close(first_out[0]);
        execl("./sarah_1_1", "./sarah_1_1", NULL);
        perror("execl");
        exit(1);
    }
    close(first_in[0]);
    close(first_out[1]);
    *fin = fdopen(first_in[1], "w");
    *fout = fdopen(first_out[0], "r");
    setvbuf(*fin,  NULL, _IONBF, 0);
    setvbuf(*fout, NULL, _IONBF, 0);
    if (!fin || !fout) {
        perror("fdopen");
        printf("HERE\n");
        exit(1);
    }


    pipe(second_in);
    pipe(second_out);
    pid_second = fork();
    if (pid_second == 0){
        printf("DUP\n");
        dup2(second_in[0], STDIN_FILENO);
        dup2(second_out[1], STDOUT_FILENO);

        close(second_in[1]);
        close(second_out[0]);

        execl("./sarah_1_1", "./sarah_1_1", NULL);
        perror("execl");
        exit(1);
    }
    close(second_in[0]);
    close(second_out[1]);
    
    *sin = fdopen(second_in[1], "w");
    *sout = fdopen(second_out[0], "r");

    setvbuf(*sin,  NULL, _IONBF, 0);
    setvbuf(*sout, NULL, _IONBF, 0);

    if (!sin || !sout) {
        perror("fdopen");
        printf("HERE\n");
        exit(1);
    }
}

void close_engine(FILE * fin, FILE * fout, FILE * sin, FILE * sout){
    

    fprintf(fin, "quit\n");
    fflush(fin);
    fprintf(sin, "quit\n");
    fflush(sin);

    fclose(fin);
    fclose(fout);
    fclose(sin);
    fclose(sout);

    for (int i = 0; i < 10; i++){
        int status;
        int p1 = kill(pid_first, SIGKILL);
        int p2 = kill(pid_second, SIGKILL);
        if (!p1 && !p2) break;
        waitpid(pid_first, &status, WNOHANG);
        waitpid(pid_second, &status, WNOHANG);
        sleep(10);
    }
}

const int nodes = 10000;
char moves[5000000];
bool fwhite = false;
int fen_cnt = 0;
int draws = 0;
int t_plus_wins = 0;
int t_minus_wins = 0;
int res_acc = 0;
int time_loss = 0;
int w_wins = 0;
int b_wins = 0;
int illegal_positions = 0;
double current_time = 0.0;

int init_fens(const char * epd){
    FILE * f = fopen(epd, "r");
    if (!f) return 0;
    char line[20000];

    while (fgets(line, sizeof(line), f)){
        strcpy(fens[fen_cnt], line);
        fen_cnt++;
    }
    return 1;
}

void init_params_from_file(const char * path){
    FILE * f = fopen(path, "r");

    if (!f) { printf("FILE NOT FOUND\n"); return; }

    char line[50000];
    while (fgets(line, sizeof(line), f)){
        char * n = strtok(line, " =");
        if (!n) continue;
        for (int i = 0; i < param_count; i++){
            if (strcmp(&n[1], params[i].name) == 0){
                n = strtok(NULL, " \n");
                if (!n) continue;
                n = strtok(NULL, " \n");
                if (!n) continue;
                printf("%s SET TO %f\n", params[i].name, strtod(n, NULL));
                params[i].start = strtod(n, NULL);
            }
        }
    }

    fclose(f);
}



int play_game(FILE * fin, FILE * fout, FILE * sin, FILE * sout, int s){

    fprintf(fin, "uci\n");
    fflush(fin);
    char line[512];
    while (fgets(line, sizeof(line), fout)) {
        if (strstr(line, "uciok")) break;
    }
    fprintf(sin, "uci\n");
    fflush(sin);

    while (fgets(line, sizeof(line), sout)) {
        if (strstr(line, "uciok")) break;
    }

    fprintf(fin, "isready\n");
    fflush(fin);
    while (fgets(line, sizeof(line), fout)) {
        if (strstr(line, "readyok")) break;
    }
    fprintf(sin, "isready\n");
    fflush(sin);
    while (fgets(line, sizeof(line), sout)) {
        if (strstr(line, "readyok")) break;
    }

    // this sets our uci options. my engine uses this syntax parsing. see uci.c for the macro that parses these options in. the type specifier is here so that we can round. without rounding for integer values (especially for depth parameters) spsa simply does not work, since jumping down when moving down by only 0.1 for instance may cause big changes in elo.
    for (int i = 0; i < param_count; i++){

        if (params[i].t == INT16 || params[i].t == UINT8){

            float r1 = ceil(params[i].e1);
            float d1 = r1 - params[i].e1;
            int rg1 = round(d1 * 10000.0f);
            int rnd1 = rand() % 10000;
            double rn1 = 0.0;
            if (rnd1 > rg1){
                rn1 = r1;
            } else {
                rn1 = r1 - 1;
            }

            float r2 = ceil(params[i].e2);
            float d2 = r2 - params[i].e2;
            int rg2 = round(d2 * 10000.0f);
            int rnd2 = rand() % 10000;
            double rn2 = 0.0;
            if (rnd2 > rg2){
                rn2 = r2;
            } else {
                rn2 = r2 - 1;
            }

            
            fprintf(fin, "setoption name %s value %f\n", params[i].name, rn1);
            fflush(fin);

            fprintf(sin, "setoption name %s value %f\n", params[i].name, rn2);
            fflush(sin);
            
        } else {
            
            fprintf(fin, "setoption name %s value %f\n", params[i].name, params[i].e1);
            fflush(fin);

            fprintf(sin, "setoption name %s value %f\n", params[i].name, params[i].e2);
            fflush(sin);
        }
    }

    char fen[87];
    size_t fc = s % (fen_cnt - 1);
    strcpy(fen, fens[fc]);
    fen[strlen(fen)-2] = '\0';

    // w_time and b_time are simply named this to follow conventions. in reality, they are just interchangeable based on engine's time ownership. so we swap them naturally with the order. it does not matter.
    double start = now();
    double w_time = 10.0;
    double b_time = 10.0;
    double inc = 0.1;

    // this is the cursed parsing for side. simply go forward and token the empty spaces
    char fen_cpy[87];
    strcpy(fen_cpy, fen);
    char * side = NULL;
    side = strtok(fen_cpy, " ");
    if (!side) {
        printf("ERR FEN\n");
        return -1;
    }
    side = strtok(NULL, " ");
    if (!side) {
        printf("ERR FEN\n");
        return -1;
    }

    double * ftime = NULL; double * stime = NULL;
    if (fwhite){
        ftime = &w_time;
        stime = &b_time;
    } else {
        stime = &w_time;
        ftime = &b_time;
    }


    // if fin is white and the fen makes black move first, we have to actually say that fin isn't white (even though it is) so that sin goes first. the same is true for the opposite case.
    // this is because fwhite actually only determines which engine goes first. if we are using loaded fens, white may no longer be moving first.
    // i could have probably programmed this in a much less cursed way but it is what it is and it works.
    if (side[0] == 'b'){
        fwhite = !fwhite;
    }
    


    printf("FEN: %s\n", fen);
    fprintf(fin, "position fen %s\n", fen);
    fflush(fin);
    fprintf(sin, "position fen %s\n", fen);
    fflush(sin);

    
    char * move;
    while (1){
        // if fwhite (or fblack but fen says it is white to move
        if (fwhite){
            fprintf(fin, "position fen %s moves %s\n", fen, moves);
            fflush(fin);
            double before = now();
            fprintf(fin, "go wtime %d btime %d winc %d binc %d\n", (int)(w_time * 1000), (int)(b_time * 1000), (int)(inc * 1000), (int)(inc * 1000));
            fflush(fin);
            while (fgets(line, sizeof(line), fout)){
                if (strstr(line, "bestmove")) break;
                if (strstr(line, "NO MOVE FOUND")) {
                    illegal_positions += 1;
                    draws += 1;
                    return 0;
                }
                printf("%s", line);
                if (strstr(line, "LOSS")) {
                    t_minus_wins += 1;
                    return -1;
                }
                if (strstr(line, "DRAW")) {
                    draws += 1;
                    return 0;
                }
            }
            printf("\n");
            double fel = now() - before;
            *ftime += inc - fel;
            if (*ftime < 0.0) {
                printf("TIME WAS %f\n", *ftime);
                t_minus_wins += 1;
                time_loss += 1;
                return -1;
            }
            
            move = strtok(line, " ");
            if (!move) {
                printf("CLOSING\n");
                close_engine(fin, fout, sin, sout);
            }
            move = strtok(NULL, " \n");
            if (!move) {
                printf("CLOSING\n");
                close_engine(fin, fout, sin, sout);
            }

            int s = strlen(move);
            move[s] = ' ';
            move[s+1] = '\0';
            strcat(moves, move);

            
            fprintf(sin, "position fen %s moves %s\n", fen, moves);
            fflush(sin);
            before = now();
            fprintf(sin, "go wtime %d btime %d winc %d binc %d\n", (int)(w_time * 1000), (int)(b_time * 1000), (int)(inc * 1000), (int)(inc * 1000));
            fflush(sin);
            while (fgets(line, sizeof(line), sout)){
                if (strstr(line, "bestmove")) break;
                if (strstr(line, "NO MOVE FOUND")) {
                    illegal_positions += 1;
                    draws += 1;
                    return 0;
                }
                printf("%s", line);
                if (strstr(line, "LOSS")) {
                    t_plus_wins += 1;
                    return 1;
                }
                if (strstr(line, "DRAW")) {
                    draws += 1;
                    return 0;
                }
            }
            printf("\n");
            double sel = now() - before;
            *stime += inc - sel;
            if (*stime < 0.0) {
                printf("TIME WAS %f\n", *stime);
                t_plus_wins += 1;
                time_loss += 1;
                return 1;
            }
            move = strtok(line, " ");
            if (!move) {
                printf("CLOSING\n");
                close_engine(fin, fout, sin, sout);
            }
            move = strtok(NULL, " \n");
            if (!move) {
                printf("CLOSING\n");
                close_engine(fin, fout, sin, sout);
            }
            s = strlen(move);
            move[s] = ' ';
            move[s+1] = '\0';
            strcat(moves, move);


        } else {
            fprintf(sin, "position fen %s moves %s\n", fen, moves);
            fflush(sin);
            double before = now();
            fprintf(sin, "go wtime %d btime %d winc %d binc %d\n", (int)(w_time * 1000), (int)(b_time * 1000), (int)(inc * 1000), (int)(inc * 1000));
            fflush(sin);
            while (fgets(line, sizeof(line), sout)){
                if (strstr(line, "bestmove")) break;
                if (strstr(line, "NO MOVE FOUND")) {
                    illegal_positions += 1;
                    draws += 1;
                    return 0;
                }
                printf("%s", line);
                if (strstr(line, "LOSS")) {
                    t_plus_wins += 1;
                    return 1;
                }
                if (strstr(line, "DRAW")) {
                    draws += 1;
                    return 0;
                }
            }
            printf("\n");
            double sel = now() - before;
            *stime += inc - sel;
            if (*stime < 0.0) {
                printf("TIME WAS %f\n", *stime);
                t_plus_wins += 1;
                time_loss += 1;
                return 1;
            }
            move = strtok(line, " ");

            if (!move) {
                close_engine(fin, fout, sin, sout);
            }
            move = strtok(NULL, " \n");
            if (!move) {
                close_engine(fin, fout, sin, sout);
            }

            int s = strlen(move);
            move[s] = ' ';
            move[s+1] = '\0';
            strcat(moves, move);

            fprintf(fin, "position fen %s moves %s\n", fen, moves);
            fflush(fin);
            before = now();
            fprintf(fin, "go wtime %d btime %d winc %d binc %d\n", (int)(w_time * 1000), (int)(b_time * 1000), (int)(inc * 1000), (int)(inc * 1000));
            fflush(fin);

            while (fgets(line, sizeof(line), fout)){
                if (strstr(line, "bestmove")) break;
                if (strstr(line, "NO MOVE FOUND")) {
                    illegal_positions += 1;
                    draws += 1;
                    return 0;
                }
                printf("%s", line);
                if (strstr(line, "LOSS")) {
                    t_minus_wins += 1;
                    return -1;
                }
                if (strstr(line, "DRAW")) {
                    draws += 1;
                    return 0;
                }
            }
            printf("\n");
            double fel = now() - before;
            *ftime += inc - fel;
            if (*ftime < 0.0) {
                printf("TIME WAS %f\n", *ftime);
                t_minus_wins += 1;
                time_loss += 1;
                return -1;
            }
            move = strtok(line, " ");
            if (!move) {
                close_engine(fin, fout, sin, sout);
            }
            move = strtok(NULL, " \n");
            if (!move) {
                close_engine(fin, fout, sin, sout);
            }

            s = strlen(move);
            move[s] = ' ';
            move[s+1] = '\0';
            strcat(moves, move);
        }
        
    }
    
}

void write_checkpoint(const char * file_name, int iterations, int K){
    char path[1024];
    sprintf(path, "./tuner/%s_%d_of_%d.txt", file_name, iterations, K); 
    FILE * f = fopen(path, "w");
    if (!f) return;
    for (int i = 0; i < param_count; i++){
        fprintf(f, ".%s = %f\n", params[i].name, params[i].value);
    }
    fprintf(f, "Time: %d | T+ Wins: %d | T- Wins: %d | Draws: %d | Time Losses: %d Illegal Positions: %d\n", (int)current_time, t_plus_wins, t_minus_wins, draws, time_loss, illegal_positions);
    fclose(f);
}

int main(){
    

    init_params();

    FILE * fin = NULL; FILE * fout = NULL; FILE * sin = NULL; FILE * sout = NULL;
    const int K = 4000;

    const int A = 400;

    double a[PARAM_MAX]; double c[PARAM_MAX]; double r[PARAM_MAX];
    init_fens("./chess.epd");
    init_params_from_file("./jan_special.txt");

    for (int i = 0; i < param_count; i++){
        c[i] = params[i].ck * pow(K, y);
        params[i].ak = params[i].rk * pow(params[i].ck, 2);
        a[i] = params[i].ak * pow(A + K, alpha);
        params[i].value = params[i].start;
    }


    double start_time = now();

    for (int s = 1; s < K; s++){

        for (int i = 0; i < param_count; i++){
            params[i].a = a[i] / pow(A + s, alpha);
            params[i].c = c[i] / pow(s, y);
            params[i].r = params[i].a / pow(params[i].c, 2);
            params[i].delta = (rand() % 2) - 1;
            if (!params[i].delta) params[i].delta = 1;
            params[i].e1 = MIN(MAX(params[i].value + params[i].c * params[i].delta, params[i].min), params[i].max);
            params[i].e2 = MIN(MAX(params[i].value - params[i].c * params[i].delta, params[i].min), params[i].max);
        }

        
        int res = 0;

        for (int i = 0; i < 2; i++){

            memset(moves, 0, sizeof(moves));

            start_engine(&fin, &fout, &sin, &sout);

            if (!fin || !fout || !sin || !sout){
                printf("ERROR FILE NOT CREATED\n");
                exit(0);
            }

            fwhite = i == 0 ? false : true;
            // 1 for f win, -1 for s win
            int game_res = play_game(fin, fout, sin, sout, s);
            res += game_res;
            

            printf("CLOSING\n");
            close_engine(fin, fout, sin, sout);

            fin = NULL;
            fout= NULL;
            sin = NULL;
            sout = NULL;
        }
        res_acc += res;

        for (int i = 0; i < param_count; i++){
            params[i].value += params[i].r * params[i].c * res / params[i].delta;
            params[i].value = MIN(MAX(params[i].value, params[i].min), params[i].max); 
            printf(".%s = %f\n", params[i].name, params[i].value);
        }


        current_time = now() - start_time;
        printf("Result: %d | Iteration: %d\n", res, s);
        printf("Time: %d | T+ Wins: %d | T- Wins: %d | Draws: %d | Time Losses: %d Illegal Positions: %d\n", (int)current_time, t_plus_wins, t_minus_wins, draws, time_loss, illegal_positions);

        if (s % 100 == 0){
            write_checkpoint("jan26", s, K);
        }


        
    }


    return 1;
}


