#include "app.hpp"
#include "URSocket.hpp"
#include "Decoder.hpp"

// This is a special compile scenario, where only a header is given.
// No cpp file is needed and there the file is also not mentioned in the makefile.
// You only need the "#define STB_IMAGE_IMPLEMENTATION" before the include and make sure the include path is right
// From the github: https://github.com/nothings/stb/blob/master/stb_image.h
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class UR5GUISmall : public App {
    public:
        UR5GUISmall(int width, int height, URSocket* sock):App(width, height), _URSocket(sock), screen_width(width), screen_height(height){
            InitPanelSizes();
            InitUpdStruct();
            std::cout << "Initialising GUI\n";
            bool ret = LoadTextureFromFile("../P2_Cell_Static.png", &my_image_texture, &my_image_width, &my_image_height);
            IM_ASSERT(ret);
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
                    _Decoder.decode_upd_msg(recv_msg, &_UpdValsChar);
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
            //makeCamPanel();
            //makeCmdPanel();
            makeDashboardPanel();
            //HelloWorldWindow();


            if (show_test_popup){
                TestPopupWindow();
            }

            if (show_refill_window){
                makeCmdPanel();
            }
                
                

            // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
            if (show_demo_window){
                ImGui::ShowDemoWindow(&show_demo_window);
            }
                

            // End main window
            ImGui::End();

        }

        void makeCamPanel() {
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
            ImGui::SetNextWindowPos(ImVec2(cam_panel_info[0], cam_panel_info[1]), ImGuiCond_Always);
            ImGui::BeginChild("CamFeed", ImVec2(cam_panel_info[2], cam_panel_info[3]), true, window_flags);
            ImGui::Indent((int)(cam_panel_info[2]/2 - ImGui::GetFontSize() * 9)); // 11 = half the characters in the title
            ImGui::TextUnformatted("RoboDK Live Simulation");
            ImGui::Unindent((int)(cam_panel_info[2]/2 - ImGui::GetFontSize() * 9));
            ImGui::Separator();

            ImGui::Image((void*)(intptr_t)my_image_texture, ImVec2(my_image_width, my_image_height));

            ImGui::EndChild();
        }

        void makeCmdPanel() {
              
            ImGuiIO& io = ImGui::GetIO();
            io.WantTextInput = true;
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
                sprintf(send_msg, "5,%s,%s,%s,%s,%s,%s", refill_bot_blue, refill_bot_pink, refill_bot_black, refill_top_blue, refill_top_pink, refill_top_black);
                _URSocket->Send(send_msg);
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
                sprintf(send_msg, "6,%s", refill_fuses);
                _URSocket->Send(send_msg);
            }


            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); 
            ImGui::Text("PCB's");
            ImGui::Spacing();
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
            ImGui::InputText("##PCBs", refill_pcb, 16);
            ImGui::SameLine();
            if (ImGui::Button("ADD##2")) {
                TestPopupWindow();
                sprintf(send_msg, "7,%s", refill_pcb);
                _URSocket->Send(send_msg);
            }


            ImGui::EndChild(); // FusePCB child
            ImGui::PopStyleVar();
            ImGui::EndChild(); // Refill child

            ImGui::End(); // Interactions child
        }

        void makeDashboardPanel() {
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
            ImGui::BeginChild("Cycle time", ImVec2((int)(dash_panel_info[2]/3), second_row_height), child_flags, window_flags);
            ImGui::TextUnformatted("Cycle Time");
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

            // Bottom covers left
            static float blue_bot_left;
            static float pink_bot_left;
            static float black_bot_left;
            static float max_bot_covers = 10;
            static char max_bot_buf[16];

            blue_bot_left = atof(_UpdValsChar.blue_bot_left);
            pink_bot_left = atof(_UpdValsChar.pink_bot_left);
            black_bot_left = atof(_UpdValsChar.black_bot_left);
            sprintf(max_bot_buf, "/%d", (int)max_bot_covers);
            
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0], third_row_ypos), ImGuiCond_Always);
            ImGui::BeginChild("Bottom covers left", ImVec2((int)(dash_panel_info[2]/3), third_row_height), child_flags, window_flags);
            ImGui::TextUnformatted("Bottom Covers Left");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
            ImGui::Text("Blue: "); ImGui::SameLine(); ImGui::ProgressBar((float)(blue_bot_left/max_bot_covers), ImVec2(130, 0.f), _UpdValsChar.blue_bot_left);ImGui::SameLine(); ImGui::Text(max_bot_buf);
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Pink: "); ImGui::SameLine(); ImGui::ProgressBar((float)(pink_bot_left/max_bot_covers), ImVec2(130, 0.f), _UpdValsChar.pink_bot_left);ImGui::SameLine(); ImGui::Text(max_bot_buf);
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Black:"); ImGui::SameLine(); ImGui::ProgressBar((float)(black_bot_left/max_bot_covers), ImVec2(130, 0.f), _UpdValsChar.black_bot_left);ImGui::SameLine(); ImGui::Text(max_bot_buf);

            ImGui::EndChild();

            // Top covers left
            static float blue_top_left;
            static float pink_top_left;
            static float black_top_left;
            static float max_top_covers = 10;
            static char max_top_buf[16];

            blue_top_left = atof(_UpdValsChar.blue_top_left);
            pink_top_left = atof(_UpdValsChar.pink_top_left);
            black_top_left = atof(_UpdValsChar.black_top_left);
            sprintf(max_top_buf, "/%d", (int)max_top_covers);

            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0]+(int)(dash_panel_info[2]/3),  third_row_ypos), ImGuiCond_Always);
            ImGui::BeginChild("Top covers left", ImVec2((int)(dash_panel_info[2]/3), third_row_height), child_flags, window_flags);
            ImGui::TextUnformatted("Top Covers Left");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
            ImGui::Text("Blue: "); ImGui::SameLine(); ImGui::ProgressBar((float)(blue_top_left/max_top_covers), ImVec2(130, 0.f), _UpdValsChar.blue_top_left);ImGui::SameLine(); ImGui::Text(max_top_buf);
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Pink: "); ImGui::SameLine(); ImGui::ProgressBar((float)(pink_top_left/max_top_covers), ImVec2(130, 0.f), _UpdValsChar.pink_top_left);ImGui::SameLine(); ImGui::Text(max_top_buf);
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Black:"); ImGui::SameLine(); ImGui::ProgressBar((float)(black_top_left/max_top_covers), ImVec2(130, 0.f), _UpdValsChar.black_top_left);ImGui::SameLine(); ImGui::Text(max_top_buf);

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
            }

            ImGui::SameLine(0, 30);

            if (ImGui::Button("Conventional\n    Program", ImVec2(ImGui::GetFontSize() * 6, ImGui::GetFontSize() * 3))) {
                strcpy(send_msg, "1RUN SIMPLE");
                _URSocket->Send(send_msg);
            }

            ImGui::SameLine(0, 30);

            if (ImGui::Button("    Fast\nProgram", ImVec2(ImGui::GetFontSize() * 6, ImGui::GetFontSize() * 3))) {
                strcpy(send_msg, "3RUN FAST");
                _URSocket->Send(send_msg);
            }

            ImGui::SameLine(0, 30);

            if (ImGui::Button("    Stop\nProgram", ImVec2(ImGui::GetFontSize() * 6, ImGui::GetFontSize() * 3))) {
                strcpy(send_msg, "2STOP PROGRAM");
                _URSocket->Send(send_msg);
            }

            ImGui::SameLine(0, 30);

            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 0, 0, 100));
            if (ImGui::Button("EMERGENCY\n      STOP", ImVec2(ImGui::GetFontSize() * 6, ImGui::GetFontSize() * 3))) {
                strcpy(send_msg, "4STOP");
                _URSocket->Send(send_msg);
            }
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            ImGui::EndChild(); // Dashboard child
        }

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        void HelloWorldWindow()
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // TODO: Send start signal to robot over TCP
        void TestPopupWindow(){            
            ImGui::Begin("Program Started", &show_test_popup);

            ImGui::TextWrapped(recv_msg);
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
            bool show_test_popup = false;
            int my_image_width = 0;
            int my_image_height = 0;

            GLuint my_image_texture = 0;
            URSocket* _URSocket;

            const char* robodk_IP = "127.0.0.1";
            int robodk_port = 50008;

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

            int screen_width;
            int screen_height;

            char recv_msg[1024];

            char send_msg[1024];

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

};



int main(int x, char**){
    URSocket* URSock = new URSocket(50005);
    //RoboDKClient* RoboClient = new RoboDKClient();

    UR5GUISmall GUI(800, 480, URSock);
    GUI.Run();

    return 0;
}
