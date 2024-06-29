#ifndef DECODER
#define DECODER

#include "tinyxml2.h"

// Used to store the incoming data from the UR robot
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

// Decoder conatining decoder function for different messages.
// Currently only the update message is received from the UR robot.
class Decoder {
    public:
        Decoder();
        ~Decoder();

        void decode_upd_msg(char* recv_buf, UpdateValuesChars* upd_vals);
};

#endif