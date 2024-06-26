#include "app.hpp"

// This is a special compile scenario, where only a header is given.
// No cpp file is needed and there the file is also not mentioned in the makefile.
// You only need the "#define STB_IMAGE_IMPLEMENTATION" before the include and make sure the include path is right
// From the github: https://github.com/nothings/stb/blob/master/stb_image.h
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class UR5GUI : public App {
    public:
        UR5GUI(int width, int height):App(width, height), screen_width(width), screen_height(height){
            InitPanelSizes();
            bool ret = LoadTextureFromFile("../P2_Cell_Static.png", &my_image_texture, &my_image_width, &my_image_height);
            IM_ASSERT(ret);
        }

        ~UR5GUI() = default;

        virtual void StartUp() final {

        }

        /*
        This is the main GUI function that is called every frame iteration. 
        All custom GUI stuff is written in this function. Function calls is used to organise the code.
        */
        virtual void Update() final {
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
            makeCamPanel();
            makeCmdPanel();
            makeDashboardPanel();
            HelloWorldWindow();


            if (show_test_popup)
                TestPopupWindow();

            // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

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


            /*
            ImVec2 imageSize(640, 480); // Adjust the size as needed
            //ImVec2 imagePos = ImVec2(0, 19);
            ImVec2 uv_min = ImVec2(0.0f, 0.0f); // UV coordinates for the rectangle corners
            ImVec2 uv_max = ImVec2(1.0f, 1.0f);
            ImVec4 tint_col = ImVec4(0.25f, 0.25f, 0.25f, 0.25f); // No tint
            ImVec4 border_col = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Black border
            */

            // TODO: Change it to take image from robodk instead of file
            //int my_image_width = 0;
            //int my_image_height = 0;
            //GLuint my_image_texture = 0;
            //bool ret = LoadTextureFromFile("../DummyPic.png", &my_image_texture, &my_image_width, &my_image_height);
            //IM_ASSERT(ret);

            ImGui::Image((void*)(intptr_t)my_image_texture, ImVec2(my_image_width, my_image_height));

            ImGui::EndChild();
        }

        void makeCmdPanel() {
            // Begin Hierachy Panel (Top Right)
            ImGuiChildFlags child_flags = ImGuiChildFlags_Border;
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
            ImGui::SetNextWindowPos(ImVec2(cmd_panel_info[0], cmd_panel_info[1]), ImGuiCond_Always);
            ImGui::BeginChild("Interactions", ImVec2(cmd_panel_info[2], cmd_panel_info[3]), child_flags, window_flags);
            ImGui::Indent((int)(cmd_panel_info[2]/2-ImGui::GetFontSize() * 4));
            ImGui::TextUnformatted("Interactions Panel");
            ImGui::Separator();

            
            ImGui::SetNextWindowPos(ImVec2(cmd_panel_info[0], cmd_panel_info[1]+25), ImGuiCond_Always);
            ImGui::BeginChild("Refill Dispensers", ImVec2((int)(cmd_panel_info[2]*2/3), cmd_panel_info[3]-25), child_flags, window_flags);
            ImGui::Indent((int)(cmd_panel_info[2]*2/6 - ImGui::GetFontSize() * 6)); //9 = 1/2*characters in text below
            ImGui::Text("Refill Dispensers");
            ImGui::Unindent((int)(cmd_panel_info[2]*2/6 - ImGui::GetFontSize() * 6));
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing();

            ImGui::SetNextWindowPos(ImVec2(cmd_panel_info[0], cmd_panel_info[1]+50), ImGuiCond_Always);
            ImGui::BeginChild("Cover Refill", ImVec2((int)(cmd_panel_info[2]*2/3*2/3), cmd_panel_info[3]-50), true, window_flags);
            ImGui::Indent(50);
            ImGui::Text("BOTTOM  covers");ImGui::SameLine();ImGui::Text("  TOP");
            ImGui::Spacing();
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
            ImGui::InputText(" Blue ", refill_bot_blue, 16); ImGui::SameLine(); ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4); ImGui::InputText(" ", refill_top_blue, 16);
            ImGui::Spacing();
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4); 
            ImGui::InputText(" Pink ", refill_bot_pink, 16); ImGui::SameLine(); ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4); ImGui::InputText(" ", refill_top_pink, 16);
            ImGui::Spacing();
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
            ImGui::InputText(" Black", refill_bot_black, 16); ImGui::SameLine(); ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4); ImGui::InputText(" ", refill_top_pink, 16);
            ImGui::Spacing(); ImGui::Spacing();
            
            ImGui::Indent(65);
            if (ImGui::Button("ADD")) {
                show_test_popup = true;
            }
            ImGui::EndChild(); // Cover child

            ImGui::SetNextWindowPos(ImVec2(cmd_panel_info[0]+(int)(cmd_panel_info[2]*2/3*2/3), cmd_panel_info[1]+50), ImGuiCond_Always);
            ImGui::BeginChild("FusePcb Refill", ImVec2((int)(cmd_panel_info[2]*2/3*1/3), cmd_panel_info[3]-50), true, window_flags);
            ImGui::Indent(10);
            ImGui::Spacing();ImGui::Spacing();
            ImGui::Text("Fuses");
            ImGui::Spacing();
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
            ImGui::InputText(" ", refill_fuses, 16);
            ImGui::SameLine();
            if (ImGui::Button("ADD")) {
                show_test_popup = true;
            }


            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
            ImGui::Text("PCB's");
            ImGui::Spacing();
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
            ImGui::InputText(" ", refill_pcb, 16);
            ImGui::SameLine();
            if (ImGui::Button("ADD")) {
                show_test_popup = true;
            }

            ImGui::EndChild(); // FusePCB child

            ImGui::EndChild(); // Refill child

            ImGui::SetNextWindowPos(ImVec2(cmd_panel_info[0]+(int)(cmd_panel_info[2]*2/3), cmd_panel_info[1]+25), ImGuiCond_Always);
            ImGui::BeginChild("UR Commands", ImVec2((int)(cmd_panel_info[2]*1/3), cmd_panel_info[3]-25), child_flags, window_flags);
            ImGui::Indent((int)(cmd_panel_info[2]*1/6 - ImGui::GetFontSize() * 4)); //5 = 1/2*characters in text below
            ImGui::Text("UR Commands");
            ImGui::Unindent((int)(cmd_panel_info[2]*1/6 - ImGui::GetFontSize() * 4));
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing();
            ImGui::Indent(45);

            if (ImGui::Button("Conventional\n  Program", ImVec2(ImGui::GetFontSize() * 8, 0))) {
                show_test_popup = true;
            }
            ImGui::Spacing();
            if (ImGui::Button("Fast Program", ImVec2(ImGui::GetFontSize() * 8, 0))) {
                show_test_popup = true;
            }
            ImGui::Spacing();
            if (ImGui::Button("Stop Program", ImVec2(ImGui::GetFontSize() * 8, 0))) {
                show_test_popup = true;
            }
            ImGui::Spacing();
            if (ImGui::Button("EMERGENCY\n  STOP", ImVec2(ImGui::GetFontSize() * 8, 0))) {
                show_test_popup = true;
            }

            ImGui::EndChild(); // URCommand child

            ImGui::EndChild(); // Interactions child
        }

        void makeDashboardPanel() {
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
            // TODO: Make header font bigger
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0], dash_panel_info[1]), ImGuiCond_Always);
            ImGui::BeginChild("Dashboard", ImVec2(dash_panel_info[2], dash_panel_info[3]), true, window_flags);
            ImGui::Indent((int)(dash_panel_info[2]/2-ImGui::GetFontSize() * 4));
            ImGui::TextUnformatted("Dashboard");
            ImGui::Separator();

            // A lot of copy pasting with small modification to create the dashboard panels
            int first_row_ypos = 25;
            int first_row_height = 75;
            int second_row_ypos = first_row_ypos + first_row_height;
            int second_row_height = 150;
            ImGuiChildFlags child_flags = ImGuiChildFlags_Border;

            // State
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0], first_row_ypos), ImGuiCond_Always);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(255, 0, 0, 100));
            ImGui::BeginChild("State", ImVec2((int)(dash_panel_info[2]/3), first_row_height), child_flags, window_flags);
            ImGui::PopStyleColor();
            ImGui::TextUnformatted("State");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
            ImGui::Indent(65);
            ImGui::Text("IDLE");

            ImGui::EndChild();
            

            // Units produced
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0]+(int)(dash_panel_info[2]/3),  first_row_ypos), ImGuiCond_Always);
            ImGui::BeginChild("Units Produced", ImVec2((int)(dash_panel_info[2]/3), first_row_height), child_flags, window_flags);
            ImGui::TextUnformatted("Units Produced");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
            ImGui::Indent(80);
            ImGui::Text("4");
            ImGui::EndChild();

            // Units ordered
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0] + 2*(int)(dash_panel_info[2]/3),  first_row_ypos), ImGuiCond_Always);
            ImGui::BeginChild("Units Ordered", ImVec2((int)(dash_panel_info[2]/3), first_row_height), child_flags, window_flags);
            ImGui::TextUnformatted("Units Ordered");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
            ImGui::Indent(80);
            ImGui::Text("12");

            ImGui::EndChild();

            // Bottom covers left
            // TODO: change to state variable from robot or arduino
            float blue_bot_left = 4;
            float pink_bot_left = 8;
            float black_bot_left = 2;
            float max_bot_covers = 10;
            char blue_bot_buf[16];
            char pink_bot_buf[16];
            char black_bot_buf[16];
            char max_bot_buf[16];
            sprintf(blue_bot_buf, "%d", (int)blue_bot_left);
            sprintf(pink_bot_buf, "%d", (int)pink_bot_left);
            sprintf(black_bot_buf, "%d", (int)black_bot_left);
            sprintf(max_bot_buf, "/%d", (int)max_bot_covers);
            
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0], second_row_ypos), ImGuiCond_Always);
            ImGui::BeginChild("Bottom covers left", ImVec2((int)(dash_panel_info[2]/3), second_row_height), child_flags, window_flags);
            ImGui::TextUnformatted("Bottom covers left");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
            ImGui::Text("Blue: "); ImGui::SameLine(); ImGui::ProgressBar((float)(blue_bot_left/max_bot_covers), ImVec2(100, 0.f), blue_bot_buf);ImGui::SameLine(); ImGui::Text(max_bot_buf);
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Pink: "); ImGui::SameLine(); ImGui::ProgressBar((float)(pink_bot_left/max_bot_covers), ImVec2(100, 0.f), pink_bot_buf);ImGui::SameLine(); ImGui::Text(max_bot_buf);
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Black:"); ImGui::SameLine(); ImGui::ProgressBar((float)(black_bot_left/max_bot_covers), ImVec2(100, 0.f), black_bot_buf);ImGui::SameLine(); ImGui::Text(max_bot_buf);

            ImGui::EndChild();

            // Top covers left
            float blue_top_left = 7;
            float pink_top_left = 10;
            float black_top_left = 1;
            float max_top_covers = 10;
            char blue_top_buf[16];
            char pink_top_buf[16];
            char black_top_buf[16];
            char max_top_buf[16];
            sprintf(blue_top_buf, "%d", (int)blue_top_left);
            sprintf(pink_top_buf, "%d", (int)pink_top_left);
            sprintf(black_top_buf, "%d", (int)black_top_left);
            sprintf(max_top_buf, "/%d", (int)max_top_covers);

            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0]+(int)(dash_panel_info[2]/3),  second_row_ypos), ImGuiCond_Always);
            ImGui::BeginChild("Top covers left", ImVec2((int)(dash_panel_info[2]/3), second_row_height), child_flags, window_flags);
            ImGui::TextUnformatted("Top covers left");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
            ImGui::Text("Blue: "); ImGui::SameLine(); ImGui::ProgressBar((float)(blue_top_left/max_top_covers), ImVec2(100, 0.f), blue_top_buf);ImGui::SameLine(); ImGui::Text(max_top_buf);
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Pink: "); ImGui::SameLine(); ImGui::ProgressBar((float)(pink_top_left/max_top_covers), ImVec2(100, 0.f), pink_top_buf);ImGui::SameLine(); ImGui::Text(max_top_buf);
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Black:"); ImGui::SameLine(); ImGui::ProgressBar((float)(black_top_left/max_top_covers), ImVec2(100, 0.f), black_top_buf);ImGui::SameLine(); ImGui::Text(max_top_buf);

            ImGui::EndChild();

            // Fuses left
            float fuses_left = 45;
            float max_fuses = 100;
            char fuses_buf[16];
            char max_fuses_buf[16];
            sprintf(fuses_buf, "%d", (int)fuses_left);
            sprintf(max_fuses_buf, "/%d", (int)max_fuses);
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0] + 2*(int)(dash_panel_info[2]/3), second_row_ypos), ImGuiCond_Always);
            ImGui::BeginChild("Fuses left", ImVec2((int)(dash_panel_info[2]/3), (int)(second_row_height/2)), child_flags, window_flags);
            ImGui::TextUnformatted("Fuses left");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
            ImGui::Indent();
            ImGui::ProgressBar((float)(fuses_left/max_fuses), ImVec2(100, 0.f), fuses_buf);ImGui::SameLine(); ImGui::Text(max_fuses_buf);
            //ImGui::Text("46");

            ImGui::EndChild();

            // PCB's left
            float pcb_left = 2;
            float max_pcb = 4;
            char pcb_buf[16];
            char max_pcb_buf[16];
            sprintf(pcb_buf, "%d", (int)pcb_left);
            sprintf(max_pcb_buf, "/%d", (int)max_pcb);
            ImGui::SetNextWindowPos(ImVec2(dash_panel_info[0] + 2*(int)(dash_panel_info[2]/3),  second_row_ypos + (int)(second_row_height/2)), ImGuiCond_Always);
            ImGui::BeginChild("PCB's left", ImVec2((int)(dash_panel_info[2]/3), (int)(second_row_height/2)), child_flags, window_flags);
            ImGui::TextUnformatted("PCB's left");
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
            ImGui::Indent();
            ImGui::ProgressBar((float)(pcb_left/max_pcb), ImVec2(100, 0.f), pcb_buf);ImGui::SameLine(); ImGui::Text(max_pcb_buf);

            ImGui::EndChild();


            ImGui::EndChild();
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
            bool show_demo_window = true;
            bool show_another_window = false;
            bool show_test_popup = false;
            int my_image_width = 0;
            int my_image_height = 0;
            GLuint my_image_texture = 0;

            char refill_bot_blue[16] = "0";
            char refill_bot_pink[16] = "0";
            char refill_bot_black[16] = "0";
            char refill_top_blue[16] = "0";
            char refill_top_pink[16] = "0";
            char refill_top_black[16] = "0";
            char refill_fuses[16] = "0";
            char refill_pcb[16] = "0";

            int screen_width = 0;
            int screen_height = 0;

            //Positioning {x_pos, y_pos, width, height}
            int cam_panel_info[4]; 
            int cmd_panel_info[4]; 
            int dash_panel_info[4];

            void InitPanelSizes() {
                //Positioning {x_pos, y_pos, width, height}
                cam_panel_info[0] = 0; cam_panel_info[1] = 0; cam_panel_info[2] = 660; cam_panel_info[3] = 520; 

                cmd_panel_info[0] = 0;
                cmd_panel_info[1] = cam_panel_info[1]+cam_panel_info[3];
                cmd_panel_info[2] = cam_panel_info[2];
                cmd_panel_info[3] = screen_height-cam_panel_info[3];

                dash_panel_info[0] = cam_panel_info[0]+cam_panel_info[2];
                dash_panel_info[1] = 0;
                dash_panel_info[2] = screen_width - cam_panel_info[2];
                dash_panel_info[3] = screen_height;
            }

};



int main(int x, char**){
    UR5GUI GUI(1280, 720);
    GUI.Run();

    return 0;
}