# P2-Demo-Robot-Cell
This GitHub repository contains the relevant code and files for a robot demonstration cell initially developed by a second semester group at Aalborg University and later refined for robustness and visual appearance. The project aims to demonstrate how an entrance to autonomy does not necessarily have to be as expensive as one might think. This single robot cell stands in contrast to a bigger Festo demonstartion line which, like this robot cell, assembles a dummy phone. Beside this introduction to the cell, build and run instructions for the different applications are given to enable other persons or groups at Aalborg University to operate the cell. See the robot cell in action on [YouTube](https://youtu.be/gxAgMTmDVhU).
![Image of the whole robot cell](Media/BilledeAfCelleCropped.jpg)

## How To Run the Cell
The first step to run the cell is making sure that everything is plugged in. There are three plugs that must be plugged into a power outlet: one for the robot, one for the Raspberry Pi and one for the dispensers. When the Raspberry Pi is plugged in it will automatically boot up and start the GUI application after some time. The next step is to start the right program on the UR5 robot. To do so, one must first initialise the robot by going the initialsation screen, turn ON the robot, START the robot, and holding the AUTO button until it is ready. Please notice that this old Polyscope version (1.8) moves a lot when initialising the robot, so make sure that the gripper is not close to any of the dispensers and the table; be careful when holding the AUTO button down. Before pushing AUTO the robot can be moved by hand by pushing the button on the backside of the teach pendant. 

## TCPServer
To build the GUIServer application please refer to the comments written in imgui example_glfw_opengl.cpp to see dependencies build process for different platforms. The Makefile has been modified to suit a desired file structure for the project. Run make in GUIServer folder (in MSYS2 on windows) to build the application. The App class structure has been set up following a tutorial on YT:
[ImGUI code setup](https://www.youtube.com/watch?v=OYQp0GuoByM&t=1240s) 

![Screenshot of the GUI used for the robot cell demonstration.](Media/URSimAndGUI.png)


