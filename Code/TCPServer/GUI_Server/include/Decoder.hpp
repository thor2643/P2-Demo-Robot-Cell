#ifndef DECODER
#define DECODER

#include "tinyxml2.h"

/*
struct UpdateValues {
    char state[16];

    int units_produced;
    int units_ordered;

    int blue_bot_left;
    int pink_bot_left;
    int black_bot_left;
    int blue_top_left;
    int pink_top_left;
    int black_top_left;
    int fuses_left;
    int pcb_left;    
};
*/

struct UpdateValuesChars {
    char state[16];

    char units_produced[8];
    char units_ordered[8];

    char blue_bot_left[8];
    char pink_bot_left[8];
    char black_bot_left[8];
    char blue_top_left[8];
    char pink_top_left[8];
    char black_top_left[8];
    char fuses_left[8];
    char pcb_left[8];    
};


class Decoder {
    public:
        Decoder();
        ~Decoder();

        void decode_upd_msg(char* recv_buf, UpdateValuesChars* upd_vals);
};

#endif