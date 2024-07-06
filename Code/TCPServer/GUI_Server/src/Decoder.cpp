#include "Decoder.hpp"
#include <iostream>

Decoder::Decoder() {

}
Decoder::~Decoder(){

}

void Decoder::decode_upd_msg(char* recv_buf, UpdateValuesChars* upd_vals){

    /* Received message will look something like this 
        "<update_msg>"
        "<robot_state>RUNNING</robot_state>"
        "<assembled_phones>100</assembled_phones>"
        "<cycle_time>14.856</cycle_time>"
        "<components_left>"
        "<top_blue>5</top_blue><top_pink>10</top_pink><top_black>10</top_black>"
        "<bottom_blue>8</bottom_blue><bottom_pink>10</bottom_pink><bottom_black>3</bottom_black>"
        "<fuses>54</fuses><pcb>2</pcb>"
        "</components_left>"
        "</update_msg>";
    */
    
	tinyxml2::XMLDocument doc;

    // The buffer can be passed directly as the ASCII format fromt the UR5
    // mathches the char format of c++ (atleast for the tested platforms).
    std::cout << "Before parsing\n";
	if(doc.Parse(recv_buf) != tinyxml2::XML_SUCCESS) {
        std::cout << "Could not parse the received data\n";
        std::cout << "Received: \n" << recv_buf << "\n";
        return;
    } 
    std::cout << "After parsing\n";

    // Now the different values can extrapolated and put into the upd_vals struct
    // Definition of upd_vals can be found and in Decoder.hpp
    tinyxml2::XMLElement* mainElement = doc.FirstChildElement("update_msg");

    tinyxml2::XMLElement* state = mainElement->FirstChildElement("robot_state");
    tinyxml2::XMLElement* units_produced = mainElement->FirstChildElement("assembled_phones");
    tinyxml2::XMLElement* cycle_time = mainElement->FirstChildElement("cycle_time");
    tinyxml2::XMLElement* components_left = mainElement->FirstChildElement("components_left");
    tinyxml2::XMLElement* blue_top_left = components_left->FirstChildElement("top_blue");
    tinyxml2::XMLElement* pink_top_left = components_left->FirstChildElement("top_pink");
    tinyxml2::XMLElement* black_top_left = components_left->FirstChildElement("top_black");
    tinyxml2::XMLElement* blue_bot_left = components_left->FirstChildElement("bottom_blue");
    tinyxml2::XMLElement* pink_bot_left = components_left->FirstChildElement("bottom_pink");
    tinyxml2::XMLElement* black_bot_left = components_left->FirstChildElement("bottom_black");
    tinyxml2::XMLElement* fuses_left = components_left->FirstChildElement("fuses");
    tinyxml2::XMLElement* pcb_left = components_left->FirstChildElement("pcb");

    
    // Copy valuee into upd_vals struct
    strcpy(upd_vals->state, state->GetText());
    strcpy(upd_vals->units_produced, units_produced->GetText());
    strcpy(upd_vals->cycle_time, cycle_time->GetText());
    strcpy(upd_vals->blue_top_left, blue_top_left->GetText());
    strcpy(upd_vals->pink_top_left, pink_top_left->GetText());
    strcpy(upd_vals->black_top_left, black_top_left->GetText());
    strcpy(upd_vals->blue_bot_left, blue_bot_left->GetText());
    strcpy(upd_vals->pink_bot_left, pink_bot_left->GetText());
    strcpy(upd_vals->black_bot_left, black_bot_left->GetText());
    strcpy(upd_vals->fuses_left, fuses_left->GetText());
    strcpy(upd_vals->pcb_left, pcb_left->GetText());
}

