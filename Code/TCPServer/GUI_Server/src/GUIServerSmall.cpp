#include "app.hpp"
#include "URSocket.hpp"
#include "Decoder.hpp"
#include <deque>
#include <stdlib.h>

// This is a special compile scenario, where only a header is given.
// No cpp file is needed and there the file is also not mentioned in the makefile.
// You only need the "#define STB_IMAGE_IMPLEMENTATION" before the include and make sure the include path is right
// From the github: https://github.com/nothings/stb/blob/master/stb_image.h
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


class UR5GUISmall : public App {
    public:
        UR5GUISmall(int width, int height, URSocket* ur_sock, RoboDKClient* disp_sock):App(width, height), _URSocket(ur_sock), _DispenserClient(disp_sock), screen_width(width), screen_height(height){
            InitPanelSizes();
            InitUpdStruct();
            std::cout << "Initialising GUI\n";
            //bool ret = LoadTextureFromFile("../P2_Cell_Static.png", &my_image_texture, &my_image_width, &my_image_height);
            //IM_ASSERT(ret);

            // Make sure order list is empty
            order_list.clear();
        }

        ~UR5GUISmall() = default;

        virtual void StartUp() final {

        }

        /*
        This is the main GUI function that is called every frame iteration. 
        All custom GUI stuff is written in this function. Function calls is used to organise the code.
        */
        virtual void Update() final {
            // Check for new data at socket
            // if not connected
            if (!_URSocket->Connected()) {
                //std::cout << "waiting for connection..." << std::endl;
                _URSocket->AcceptConnection(); // Accept any incoming connection.
            }
            else 
            {
                // handle the current connection and update state
                if(_URSocket->HandleConnection(recv_msg)){
                    _Decoder.decode_upd_msg_str(recv_msg, &_UpdValsChar);
                }  
            }

            if (!_DispenserClient->Connected()) {
                //std::cout << "waiting for connection..." << std::endl;
                _DispenserClient->Connect(dispenser_IP, dispenser_port); // Accept any incoming connection.
            }
            else 
            {
                // handle the current connection and update state
                if(_DispenserClient->HandleConnection(recv_msg)){
                    if (strncmp(recv_msg, "READY", 5) == 0){
                        dispenser_ready = true;
                        std::cout << "Dispenser ready!";
                        //sprintf(send_msg, "capture");
                        //_DispenserClient->Send(send_msg);

                    } else if (strncmp(recv_msg, "FINISHED", 8) == 0) {
                        component_ready = true;
                    } else {
                        std::cout << "An error occured\n";
                        std::cout << "Got message: " << recv_msg << ".\n";
                        std::cout << "Comparison result: " << strcmp(recv_msg, "READY") << "\n"; 
                    }
                }  
            }

            // Handle dispenser and UR synchronization.
            if (!order_list.empty()){
                if(dispenser_ready){
                    // Send order to dispensers
                    sprintf(send_msg, "%d", order_list.front());
                    _DispenserClient->Send(send_msg);
                    
                    //Update components
                    bot_cover_num = send_msg[0] - '0'; //atoi(reinterpret_cast<const char*>(send_msg[0]));
                    top_cover_num = send_msg[1] - '0'; //atoi(reinterpret_cast<const char*>(send_msg[1]));
                    //bot_cover_num = atoi((const char*)send_msg[0]);
                    //top_cover_num = atoi((const char*)send_msg[1]);

                    updateCovers(bot_cover_num, top_cover_num);

                    // Remove order from list and set dispenser not ready
                    order_list.pop_front();
                    dispenser_ready = false;
                }
            }   

            if (component_ready && !robot_stopped){
                int id = htonl(1);
                memcpy(send_msg, &id, sizeof(int));

                if (_URSocket->Send(send_msg, 2)){
                    component_ready = false;                        
                } else {
                    strcpy(popup_msg, "Failed to reach robot!\n\n"
                                        "Please check that program runs on UR robot."
                                        "Otherwise, try to restart UR program");
                    order_list.empty();
                    show_popup = true;
                }    

                //If done update state on UR robot
                if (order_list.empty()) {
                    int id = htonl(2);
                    memcpy(send_msg, &id, sizeof(int));

                    if (_URSocket->Send(send_msg, 2)){
                        strcpy(popup_msg, "Program will stop when current process finishes.");
                        // Clear order deque to stop dispensers
                        robot_stopped = true;
                    } else {
                        strcpy(popup_msg, "Failed to send stop command!\n\n"
                                            "Please check that program runs on UR robot."
                                            "Otherwise, try to restart UR program");
                    }  
                    show_popup = true;
                }     
            }           
            
            // Resize the window to fit the glfw window.
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);
            ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);

            // Begin main window
            ImGui::Begin("Main Window", nullptr, ImGuiWindowFlags_NoTitleBar 
                                                | ImGuiWindowFlags_NoResize 
                                                | ImGuiWindowFlags_NoBringToFrontOnFocus 
                                                | ImGuiWindowFlags_NoScrollbar 
                                                | ImGuiWindowFlags_NoScrollWithMouse);
            
            //Child components are defined in the following functions
            MakeMainPanel();

            if (show_popup){
                PopupWindow();
            }

            if (show_refill_window){
                makeRefillWindow();
            }
            
            if (!show_refill_window && keyboard_open){
                system("pkill wvkbd-mobintl");
                keyboard_open = false;
            }

            // End main window
            ImGui::End();

        }

        void makeRefillWindow() {
            // Begin Hierachy Panel (Top Right)
            ImGuiChildFlags child_flags = ImGuiChildFlags_Border;
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
            ImGui::SetNextWindowPos(ImVec2(cmd_panel_info[0], cmd_panel_info[1]), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(cmd_panel_info[2], cmd_panel_info[3]), ImGuiCond_Always);

            ImGui::Begin("Refill", &show_refill_window, window_flags);
            //ImGui::Indent((int)(cmd_panel_info[2]/2-ImGui::GetFontSize() * 4));
            //ImGui::TextUnformatted("Interactions Panel");
            //ImGui::Separator();

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
            ImGui::SetNextWindowPos(ImVec2(cmd_panel_info[0], cmd_panel_info[1]+20), ImGuiCond_Always);
            ImGui::BeginChild("Refill Dispensers", ImVec2((int)(cmd_panel_info[2]), cmd_panel_info[3]-25), child_flags, window_flags);
            ImGui::Indent((int)(cmd_panel_info[2]*1/2 - ImGui::GetFontSize() * 3)); //9 = 1/2*characters in text below
            ImGui::Text("Refill Dispensers");
            ImGui::Unindent((int)(cmd_panel_info[2]*1/2 - ImGui::GetFontSize() * 6));
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

            ImGui::SetNextWindowPos(ImVec2(cmd_panel_info[0], cmd_panel_info[1]+50), ImGuiCond_Always);
            ImGui::BeginChild("Cover Refill", ImVec2((int)(cmd_panel_info[2]*2/3), cmd_panel_info[3]-45), true, window_flags);
            ImGui::Indent(50);
            ImGui::Text("BOTTOM  covers");ImGui::SameLine();ImGui::Text("  TOP");
            ImGui::Spacing();
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
            ImGui::InputText(" Blue ", refill_bot_blue, 16); ImGui::SameLine(); ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4); ImGui::InputText("##blue2", refill_top_blue, 16);
            ImGui::Spacing();
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4); 
            ImGui::InputText(" Pink ", refill_bot_pink, 16); ImGui::SameLine(); ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4); ImGui::InputText("##pink2", refill_top_pink, 16);
            ImGui::Spacing();
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
            ImGui::InputText(" Black", refill_bot_black, 16); ImGui::SameLine(); ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4); ImGui::InputText("##black2", refill_top_black, 16);
            ImGui::Spacing(); ImGui::Spacing(); 
            
            ImGui::Indent(90);
            if (ImGui::Button("ADD")) {
                //sprintf(send_msg, "5,%s,%s,%s,%s,%s,%s", refill_bot_blue, refill_bot_pink, refill_bot_black, refill_top_blue, refill_top_pink, refill_top_black);
                //_URSocket->Send(send_msg);  
                if (send_cover_refill()) {
                    strcpy(popup_msg, "Refill command sent successfully!");
                } else {
                    strcpy(popup_msg, "Refill command could not be sent!\n\n"
                                        "Please check connection or that inputs are valid integers.");
                }
                show_popup = true;   
            }

            ImGui::EndChild(); // Cover child

            ImGui::SetNextWindowPos(ImVec2(cmd_panel_info[0]+(int)(cmd_panel_info[2]*2/3), cmd_panel_info[1]+50), ImGuiCond_Always);
            ImGui::BeginChild("FusePcb Refill", ImVec2((int)(cmd_panel_info[2]*1/3), cmd_panel_info[3]-50), true, window_flags);
            ImGui::Indent(10);
            ImGui::Spacing();ImGui::Spacing();
            ImGui::Text("Fuses");
            ImGui::Spacing();
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
            ImGui::InputText("##Fuses", refill_fuses, 16);
            ImGui::SameLine();

            if (ImGui::Button("ADD##1")) {
                int fuses_refill_int = atoi(refill_fuses);
                if (fuses_refill_int != 0){
                    int id = htonl(6);
                    fuses_refill_int = htonl(fuses_refill_int);
                    memcpy(send_msg, &id, sizeof(int));
                    memcpy(send_msg + sizeof(int), &fuses_refill_int, sizeof(int));

                    if (_URSocket->Send(send_msg, 4)){
                        strcpy(popup_msg, "Refill command sent successfully!");
                    } else {
                        strcpy(popup_msg, "Failed to send refill command!\n\n"
                                        "Please check that program runs on UR robot."
                                        "Otherwise, try to restart UR program");
                    }            
                } else {
                    std::cout << "Could not convert string to int. Please check input\n";
                    strcpy(popup_msg, "Could not convert string to int!\n\n"
                                        "Please check that inputs are valid integers.");
                } 

                show_popup = true;
            }


            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); 
            ImGui::Text("PCB's");
            ImGui::Spacing();
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
            ImGui::InputText("##PCBs", refill_pcb, 16);
            ImGui::SameLine();
            if (ImGui::Button("ADD##2")) {
                int pcb_refill_int = atoi(refill_pcb);
                if (pcb_refill_int != 0){
                    int id = htonl(7);
                    pcb_refill_int = htonl(pcb_refill_int);
                    memcpy(send_msg, &id, sizeof(int));
                    memcpy(send_msg + sizeof(int), &pcb_refill_int, sizeof(int));

                    if (_URSocket->Send(send_msg, 4)){
                        strcpy(popup_msg, "Refill command sent successfully!");
                    } else {
                        strcpy(popup_msg, "Failed to send refill command!\n\n"
                                        "Please check that program runs on UR robot."
                                        "Otherwise, try to restart UR program");
                    } 
                } else {
                    std::cout << "Could not convert string to int. Please check input\n";
                    strcpy(popup_msg, "Could not convert string to int!\n\n"
                                        "Please check connection or that inputs are valid integers.");
                } 
                show_popup = true;
            }


            ImGui::EndChild(); // FusePCB child
            ImGui::PopStyleVar();
            ImGui::EndChild(); // Refill child

            ImGui::End(); // Interactions child
        }

        void MakeMainPanel() {
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
            // TODO: Make header font bigger
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0], dash_panel_info[1]), ImGuiCond_Always);
            ImGui::BeginChild("Dashboard", ImVec2(dash_panel_info[2], dash_panel_info[3]), true, window_flags);
            ImGui::Indent((int)(dash_panel_info[2]/2-ImGui::GetFontSize()*2));
            //ImGui::TextUnformatted("Dashboard");

            // A lot of copy pasting with small modification to create the dashboard panels
            static int first_row_ypos = 0;
            static int first_row_height = 75;
            static int second_row_ypos = first_row_ypos + first_row_height;
            static int second_row_height = 75;
            static int third_row_ypos = second_row_ypos + second_row_height;
            static int third_row_height = 175;
            ImGuiChildFlags child_flags = ImGuiChildFlags_Border;

            // State
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0], first_row_ypos), ImGuiCond_Always);
            if (strcmp(_UpdValsChar.state, "RUNNING") == 0){
                ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 255, 0, 100));
            } else {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(255, 0, 0, 100));
            }
            ImGui::BeginChild("State", ImVec2((int)(dash_panel_info[2]/3), first_row_height), child_flags, window_flags);
            ImGui::PopStyleColor();
            ImGui::TextUnformatted("State");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing();
            ImGui::Indent(90);
            ImGui::Text(_UpdValsChar.state);

            ImGui::EndChild();
            
            // Units produced
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0]+(int)(dash_panel_info[2]/3),  first_row_ypos), ImGuiCond_Always);
            ImGui::BeginChild("Units Produced", ImVec2((int)(dash_panel_info[2]/3), first_row_height), child_flags, window_flags);
            ImGui::TextUnformatted("Units Produced");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); 
            ImGui::Indent(110);    
            ImGui::Text(_UpdValsChar.units_produced);
            ImGui::EndChild();

            // Units ordered
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0] + 2*(int)(dash_panel_info[2]/3),  first_row_ypos), ImGuiCond_Always);
            ImGui::BeginChild("Units Ordered", ImVec2((int)(dash_panel_info[2]/3), first_row_height), child_flags, window_flags);
            ImGui::TextUnformatted("Units Ordered");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); 
            ImGui::Indent(110);
            ImGui::Text(_UpdValsChar.units_ordered);

            ImGui::EndChild();

            // Cycle time
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0], second_row_ypos), ImGuiCond_Always);
            ImGui::BeginChild("Robot cycle time", ImVec2((int)(dash_panel_info[2]/3), second_row_height), child_flags, window_flags);
            ImGui::TextUnformatted("Robot Cycle Time");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); 
            ImGui::Indent(90);
            static char cycle_time[8];
            sprintf(cycle_time, "%s s", _UpdValsChar.cycle_time);
            ImGui::Text(cycle_time); // Update from UR

            ImGui::EndChild();
            
            // Producing time
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0]+(int)(dash_panel_info[2]/3),  second_row_ypos), ImGuiCond_Always);
            ImGui::BeginChild("Producing time ratio", ImVec2((int)(dash_panel_info[2]/3), second_row_height), child_flags, window_flags);
            ImGui::TextUnformatted("Producing Time Ratio");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); 
            ImGui::Indent(100);    
            ImGui::Text("77.7%%"); // Update from UR
            ImGui::EndChild();

            // Up time
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0] + 2*(int)(dash_panel_info[2]/3),  second_row_ypos), ImGuiCond_Always);
            ImGui::BeginChild("Up time", ImVec2((int)(dash_panel_info[2]/3), second_row_height), child_flags, window_flags);
            ImGui::TextUnformatted("Up Time Ratio");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing();
            ImGui::Indent(105);
            ImGui::Text("93%%");

            ImGui::EndChild();

            /*
            // Bottom covers left
            static float blue_bot_left;
            static float pink_bot_left;
            static float black_bot_left;
            static float max_bot_covers = 10;
            static char max_bot_buf[16];

            blue_bot_left = atof(_UpdValsChar.blue_bot_left);
            pink_bot_left = atof(_UpdValsChar.pink_bot_left);
            black_bot_left = atof(_UpdValsChar.black_bot_left);
            */
            static float max_bot_covers = 10;
            static char max_bot_buf[16];
            sprintf(max_bot_buf, "/%d", (int)max_bot_covers);
            
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0], third_row_ypos), ImGuiCond_Always);
            ImGui::BeginChild("Bottom covers left", ImVec2((int)(dash_panel_info[2]/3), third_row_height), child_flags, window_flags);
            ImGui::TextUnformatted("Bottom Covers Left");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
            ImGui::Text("Blue: "); ImGui::SameLine(); ImGui::ProgressBar((float)(blue_bot_left/max_bot_covers), ImVec2(130, 0.f), blue_bot_left_char);ImGui::SameLine(); ImGui::Text(max_bot_buf);
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Pink: "); ImGui::SameLine(); ImGui::ProgressBar((float)(pink_bot_left/max_bot_covers), ImVec2(130, 0.f), pink_bot_left_char);ImGui::SameLine(); ImGui::Text(max_bot_buf);
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Black:"); ImGui::SameLine(); ImGui::ProgressBar((float)(black_bot_left/max_bot_covers), ImVec2(130, 0.f), black_bot_left_char);ImGui::SameLine(); ImGui::Text(max_bot_buf);

            ImGui::EndChild();

            // Top covers left
            /*
            static float blue_top_left;
            static float pink_top_left;
            static float black_top_left;
            static float max_top_covers = 10;
            static char max_top_buf[16];

            blue_top_left = atof(_UpdValsChar.blue_top_left);
            pink_top_left = atof(_UpdValsChar.pink_top_left);
            black_top_left = atof(_UpdValsChar.black_top_left);
            */
            static float max_top_covers = 10;
            static char max_top_buf[16];
            sprintf(max_top_buf, "/%d", (int)max_top_covers);

            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0]+(int)(dash_panel_info[2]/3),  third_row_ypos), ImGuiCond_Always);
            ImGui::BeginChild("Top covers left", ImVec2((int)(dash_panel_info[2]/3), third_row_height), child_flags, window_flags);
            ImGui::TextUnformatted("Top Covers Left");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
            ImGui::Text("Blue: "); ImGui::SameLine(); ImGui::ProgressBar((float)(blue_top_left/max_top_covers), ImVec2(130, 0.f), blue_top_left_char);ImGui::SameLine(); ImGui::Text(max_top_buf);
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Pink: "); ImGui::SameLine(); ImGui::ProgressBar((float)(pink_top_left/max_top_covers), ImVec2(130, 0.f), pink_top_left_char);ImGui::SameLine(); ImGui::Text(max_top_buf);
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Black:"); ImGui::SameLine(); ImGui::ProgressBar((float)(black_top_left/max_top_covers), ImVec2(130, 0.f), black_top_left_char);ImGui::SameLine(); ImGui::Text(max_top_buf);

            ImGui::EndChild();

            // Fuses left
            static float fuses_left;  
            static float max_fuses = 100;
            static char max_fuses_buf[16];

            fuses_left = atof(_UpdValsChar.fuses_left);
            sprintf(max_fuses_buf, "/%d", (int)max_fuses);

            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0] + 2*(int)(dash_panel_info[2]/3), third_row_ypos), ImGuiCond_Always);
            ImGui::BeginChild("Fuses left", ImVec2((int)(dash_panel_info[2]/3), (int)(third_row_height/2)), child_flags, window_flags);
            ImGui::TextUnformatted("Fuses Left");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
            ImGui::Indent();
            ImGui::ProgressBar((float)(fuses_left/max_fuses), ImVec2(150, 0.f), _UpdValsChar.fuses_left);ImGui::SameLine(); ImGui::Text(max_fuses_buf);

            ImGui::EndChild();

            // PCB's left
            static float pcb_left; 
            static float max_pcb = 4;
            static char max_pcb_buf[16];

            pcb_left = atof(_UpdValsChar.pcb_left);
            sprintf(max_pcb_buf, "/%d", (int)max_pcb);

            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0] + 2*(int)(dash_panel_info[2]/3),  third_row_ypos + (int)(third_row_height/2)), ImGuiCond_Always);
            ImGui::BeginChild("PCB's left", ImVec2((int)(dash_panel_info[2]/3), (int)(third_row_height/2)), child_flags, window_flags);
            ImGui::TextUnformatted("PCB's Left");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
            ImGui::Indent();
            ImGui::ProgressBar((float)(pcb_left/max_pcb), ImVec2(150, 0.f), _UpdValsChar.pcb_left);ImGui::SameLine(); ImGui::Text(max_pcb_buf);

            ImGui::EndChild();

            ImGui::Spacing(); ImGui::Spacing();ImGui::Spacing(); ImGui::Spacing();ImGui::Spacing(); ImGui::Spacing();
            ImGui::Unindent(330);
            
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

            if (ImGui::Button("    Refill\nDispensers", ImVec2(ImGui::GetFontSize() * 6, ImGui::GetFontSize() * 3))) {
                show_refill_window = true;
                //system("lxterminal -e 'bash -c \"wvkbd-mobintl\"' &");
                system("env DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY wvkbd-mobintl > /dev/null 2>&1 &"); //echo raspberry | sudo -S -k wvkbd-mobintl");// > /dev/null 2>&1 &");
                keyboard_open = true;
                
            }

            ImGui::SameLine(0, 30);

            if (ImGui::Button("Custom\n  Order", ImVec2(ImGui::GetFontSize() * 6, ImGui::GetFontSize() * 3))) {
                //int id = htonl(3);
                //memcpy(send_msg, &id, sizeof(int));
                //_URSocket->Send(send_msg, 2);

                strcpy(popup_msg, "Not yet implemented...\n"); 
                show_popup = true;
            }

            ImGui::SameLine(0, 30);

            if (ImGui::Button("  Run\nDemo", ImVec2(ImGui::GetFontSize() * 6, ImGui::GetFontSize() * 3))) {
                //strcpy(send_msg, "3RUN FAST");
                //_URSocket->Send(send_msg);
                for (int i = 0; i < (int)(sizeof(demo_order)/sizeof(int)); i++){
                    // Copy demo orders into order list
                    order_list.push_back(demo_order[i]);
                }

                robot_stopped = false;
                strcpy(popup_msg, "Demo program started!");
                show_popup = true;
                sprintf(_UpdValsChar.units_ordered, "%d", atoi(_UpdValsChar.units_ordered)+3);

                /*
                int id = htonl(1);
                memcpy(send_msg, &id, sizeof(int));

                if (_URSocket->Send(send_msg, 2)){
                    strcpy(popup_msg, "Demo program started!");
                } else {
                    strcpy(popup_msg, "Failed to send start command!\n\n"
                                        "Please check that program runs on UR robot."
                                        "Otherwise, try to restart UR program");
                }       
                show_popup = true;*/
            }

            ImGui::SameLine(0, 30);

            if (ImGui::Button("    Stop\nProgram", ImVec2(ImGui::GetFontSize() * 6, ImGui::GetFontSize() * 3))) {
                //strcpy(send_msg, "2STOP PROGRAM");
                //_URSocket->Send(send_msg);
                int id = htonl(2);
                memcpy(send_msg, &id, sizeof(int));

                if (_URSocket->Send(send_msg, 2)){
                    strcpy(popup_msg, "Program will stop when current process finishes.");
                    // Clear order deque to stop dispensers
                    order_list.clear();
                    robot_stopped = true;
                } else {
                    strcpy(popup_msg, "Failed to send stop command!\n\n"
                                        "Please check that program runs on UR robot."
                                        "Otherwise, try to restart UR program");
                }  
                show_popup = true;
            }

            ImGui::SameLine(0, 30);

            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 0, 0, 100));
            if (ImGui::Button("EMERGENCY\n      STOP", ImVec2(ImGui::GetFontSize() * 6, ImGui::GetFontSize() * 3))) {
                //strcpy(send_msg, "4STOP");
                //_URSocket->Send(send_msg);
                int id = htonl(4);
                memcpy(send_msg, &id, sizeof(int));

                if (_URSocket->Send(send_msg, 2)){
                    strcpy(popup_msg, "Emergency stop sent!\n\n Restart program on UR robot to reconnect");
                    // Clear order deque to stop dispensers
                    order_list.clear();
                    robot_stopped = true;
                } else {
                    strcpy(popup_msg, "Failed to send emergency stop command!\n\n"
                                        "Please check that program runs on UR robot."
                                        "Otherwise, try to restart UR program");
                } 
                show_popup = true;
            }
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            ImGui::EndChild(); // Dashboard child
        }

        // TODO: Send start signal to robot over TCP
        void PopupWindow(){          
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
            
            ImGui::SetNextWindowPos(ImVec2(cmd_panel_info[0]+100, cmd_panel_info[1]), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(250, 200), ImGuiCond_Always);  
            ImGui::Begin("Program Started", &show_popup, window_flags);
            
            ImGui::TextWrapped(popup_msg);
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

            ImGui::Indent(100);

            if (ImGui::Button("OK", ImVec2(ImGui::GetFontSize() * 2, ImGui::GetFontSize() * 2))){
                show_popup = false;
            }

            ImGui::End();
           
        }

        // Simple helper function to load an image into a OpenGL texture with common settings
        bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height){
            // Load from file
            int image_width = 0;
            int image_height = 0;
            unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
            if (image_data == NULL)
                return false;

            // Create a OpenGL texture identifier
            GLuint image_texture;
            glGenTextures(1, &image_texture);
            glBindTexture(GL_TEXTURE_2D, image_texture);

            // Setup filtering parameters for display
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

            // Upload pixels into texture
            #if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
                glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            #endif
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
            stbi_image_free(image_data);

            *out_texture = image_texture;
            *out_width = image_width;
            *out_height = image_height;

            return true;
        }
        

        private:
            bool show_demo_window = false;
            bool show_another_window = false;
            bool show_refill_window = false;
            bool show_popup = false;

            bool robot_stopped = true;
            bool dispenser_ready = false;
            bool component_ready = false;
            bool any_orders = false;
            bool keyboard_open = false;


            int my_image_width = 0;
            int my_image_height = 0;

            int bot_cover_num = 0;
            int top_cover_num = 0;

            int blue_bot_left = 10;
            int pink_bot_left = 10;
            int black_bot_left = 10;
            int blue_top_left = 10;
            int pink_top_left = 10;
            int black_top_left = 10;

            char blue_bot_left_char[16] = "10";
            char pink_bot_left_char[16] = "10";
            char black_bot_left_char[16] = "10";
            char blue_top_left_char[16] = "10";
            char pink_top_left_char[16] = "10";
            char black_top_left_char[16] = "10";

            GLuint my_image_texture = 0;
            URSocket* _URSocket;
            RoboDKClient* _DispenserClient;

            const char* dispenser_IP = "127.0.0.1";
            int dispenser_port = 50010;

            Decoder _Decoder;

            //UpdateValues _UpdVals;
            UpdateValuesChars _UpdValsChar;

            char refill_bot_blue[16] = "0";
            char refill_bot_pink[16] = "0";
            char refill_bot_black[16] = "0";
            char refill_top_blue[16] = "0";
            char refill_top_pink[16] = "0";
            char refill_top_black[16] = "0";
            char refill_fuses[16] = "0";
            char refill_pcb[16] = "0";

            std::deque<int> order_list;
            const int demo_order[3] = {31, 23, 32}; // blue/black, pink/blue, blue/pink
            //char next_order[2];
            

            int screen_width;
            int screen_height;

            char recv_msg[1024];
            char send_msg[1024];
            char popup_msg[1024];

            //Positioning {x_pos, y_pos, width, height}
            int cam_panel_info[4]; 
            int cmd_panel_info[4]; 
            int dash_panel_info[4];


            void InitPanelSizes() {
                //Positioning {x_pos, y_pos, width, height}
                cam_panel_info[0] = 0; cam_panel_info[1] = 0; cam_panel_info[2] = 660; cam_panel_info[3] = 520; 

                cmd_panel_info[0] = 160;
                cmd_panel_info[1] = 100; //cam_panel_info[1]+cam_panel_info[3];
                cmd_panel_info[2] = 560;
                cmd_panel_info[3] = 250;

                dash_panel_info[0] = cam_panel_info[0]; //+cam_panel_info[2];
                dash_panel_info[1] = 0;
                dash_panel_info[2] = screen_width; // - cam_panel_info[2];
                dash_panel_info[3] = screen_height;
            }

            void InitUpdStruct() {
                strcpy(_UpdValsChar.state, "IDLE");
                strcpy(_UpdValsChar.units_produced, "0");
                strcpy(_UpdValsChar.units_ordered, "0");
                strcpy(_UpdValsChar.cycle_time, "0");
                strcpy(_UpdValsChar.blue_bot_left, "10");
                strcpy(_UpdValsChar.blue_top_left, "10");
                strcpy(_UpdValsChar.pink_bot_left, "10");
                strcpy(_UpdValsChar.pink_top_left, "10");
                strcpy(_UpdValsChar.black_bot_left, "10");
                strcpy(_UpdValsChar.black_top_left, "10");
                strcpy(_UpdValsChar.fuses_left, "100");
                strcpy(_UpdValsChar.pcb_left, "4");
                strcpy(_UpdValsChar.cycle_time, "0");
            }

            bool send_cover_refill() {
                char* end_ptr;
                int id = htonl(5);

                // Convert input text to ints and make a sanity check 
                int refill_top_blue_int = (int)strtol(refill_top_blue, &end_ptr, 10);
                if (end_ptr == refill_top_blue){     //if no characters were converted these pointers are equal
                    printf("ERROR: can't convert string to number\n");
                    return false;
                }
                blue_top_left += refill_top_blue_int;
                sprintf(blue_top_left_char, "%d", blue_top_left);
                refill_top_blue_int = htonl(refill_top_blue_int);

                int refill_top_pink_int = (int)strtol(refill_top_pink, &end_ptr, 10);
                if (end_ptr == refill_top_pink){     //if no characters were converted these pointers are equal
                    printf("ERROR: can't convert string to number\n");
                    return false;
                }
                pink_top_left += refill_top_pink_int;
                sprintf(pink_top_left_char, "%d", pink_top_left);
                refill_top_pink_int = htonl(refill_top_pink_int);

                int refill_top_black_int = (int)strtol(refill_top_black, &end_ptr, 10);
                if (end_ptr == refill_top_black){     //if no characters were converted these pointers are equal
                    printf("ERROR: can't convert string to number\n");
                    return false;
                }
                black_top_left += refill_top_black_int;
                sprintf(black_top_left_char, "%d", black_top_left);
                refill_top_black_int = htonl(refill_top_black_int);

                int refill_bot_blue_int = (int)strtol(refill_bot_blue, &end_ptr, 10);
                if (end_ptr == refill_bot_blue){     //if no characters were converted these pointers are equal
                    printf("ERROR: can't convert string to number\n");
                    return false;
                }
                blue_bot_left += refill_bot_blue_int;
                sprintf(blue_bot_left_char, "%d", blue_bot_left);
                refill_bot_blue_int = htonl(refill_bot_blue_int);

                int refill_bot_pink_int = (int)strtol(refill_bot_pink, &end_ptr, 10);
                if (end_ptr == refill_top_blue){     //if no characters were converted these pointers are equal
                    printf("ERROR: can't convert string to number\n");
                    return false;
                }
                pink_bot_left += refill_bot_pink_int;
                sprintf(pink_bot_left_char, "%d", pink_bot_left);
                refill_bot_pink_int = htonl(refill_bot_pink_int);

                int refill_bot_black_int = (int)strtol(refill_bot_black, &end_ptr, 10);
                if (end_ptr == refill_bot_black){     //if no characters were converted these pointers are equal
                    printf("ERROR: can't convert string to number\n");
                    return false;
                }
                black_bot_left += refill_bot_black_int;
                sprintf(black_bot_left_char, "%d", black_bot_left);
                refill_bot_black_int = htonl(refill_bot_black_int);

                // If conversion were succesfull the ints can be copied into send buffer and sent.
                memcpy(send_msg, &id, sizeof(int));
                memcpy(send_msg + sizeof(int), &refill_top_blue_int, sizeof(int));
                memcpy(send_msg + 2 * sizeof(int), &refill_top_pink_int, sizeof(int));
                memcpy(send_msg + 3 * sizeof(int), &refill_top_black_int, sizeof(int));
                memcpy(send_msg + 4 * sizeof(int), &refill_bot_blue_int, sizeof(int));
                memcpy(send_msg + 5 * sizeof(int), &refill_bot_pink_int, sizeof(int));
                memcpy(send_msg + 6 * sizeof(int), &refill_bot_black_int, sizeof(int));
                
                // Send with msg type 3, which ensures correct size
                if (_URSocket->Send(send_msg, 3)){
                    return true;
                } else {
                    return false;
                }; 
  
            }

            void updateCovers(int bot_num, int top_num){
                switch (bot_num)
                {
                case 1:
                    black_bot_left--;
                    sprintf(black_bot_left_char, "%d", black_bot_left);
                    break;
                case 2:
                    pink_bot_left--;
                    sprintf(pink_bot_left_char, "%d", pink_bot_left);
                    break;
                case 3:
                    blue_bot_left--;
                    sprintf(blue_bot_left_char, "%d", blue_bot_left);
                    break;
                default:
                    break;
                }

                switch (top_num)
                {
                case 1:
                    black_top_left--;
                    sprintf(black_top_left_char, "%d", black_top_left);
                    break;
                case 2:
                    pink_top_left--;
                    sprintf(pink_top_left_char, "%d", pink_top_left);
                    break;
                case 3:
                    blue_top_left--;
                    sprintf(blue_top_left_char, "%d", blue_top_left);
                    break;
                default:
                    break;
                }

            }
};



int main(int x, char**){
    URSocket* URSock = new URSocket(50005);
    RoboDKClient* DispenserClient = new RoboDKClient();

    UR5GUISmall GUI(800, 480, URSock, DispenserClient);
    GUI.Run();

    return 0;
}
