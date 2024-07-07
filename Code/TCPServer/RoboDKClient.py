"""
    Program created for the original GUIServer.cpp to show image of RoboDK simulation.
    Uses RoboDK API to retrieve image from the camera in RoboDK (RoboDK/P2-Cell) and
    saves it to a file location from which the GUI program can read the image.
    To avoid simultaneous accessing the image file a socket communication is used to 
    request the image from this program, which then confirms when the image is ready.
"""

import socket
from robodk import robolink
import time

#args=["-NEWINSTANCE", "-NOUI", "-SKIPINI", "-EXIT_LAST_COM"]
RDK = robolink.Robolink()
station = RDK.AddFile('RoboDK/P2-Cell.rdk')
cam_handle = RDK.Item("Camera 1")

def capture_image(image_path):
    # If no image path is given e.g. "". Then the image bytes are returned and can be decode using cv2.
    # Consider sending the bytes directly to GUI thorugh TCP socket and decode on GUI side.
    # This avoids the overhead of writing a file and reading the file.
    # See: https://robodk.com/doc/en/PythonAPI/examples.html#camera-live-stream
    return RDK.Cam2D_Snapshot(image_path, cam_handle, 'Color')

def start_server(host, port, image_path):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen(1)
    print(f"Server listening on {host}:{port}")

    client_socket, addr = server_socket.accept()
    print(f"Connection from {addr}")

    success = capture_image(image_path)
    time.sleep(0.01)

    if success:
        client_socket.send("READY".encode("ascii"))
    else:
        client_socket.send("ERROR".encode("ascii"))
    
    while True:
        # Receive request
        request = client_socket.recv(1024).decode("ascii")

        if request == "capture":
            print("Capturing new image")
            success = capture_image(image_path)
            if success:
                client_socket.send("READY".encode("ascii"))
            else:
                client_socket.send("ERROR".encode("ascii"))
        else:
            print(request)

        #client_socket.close()

if __name__ == "__main__":
    image_path = "C:/Users/Thor9/Downloads/Test.png"
    start_server('localhost', 50008, image_path)