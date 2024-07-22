"""
    For quick implementation and to avoid having the serial communication block the main GUI
    the dispenser communication is done through this python script. A local socket connection
    is used to talk with the main GUI program.
"""
import time
import socket
import serial

# Set socket address and port
host = "localhost"
port = 50010

# Create socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR,1)
server_socket.bind((host, port))
server_socket.listen(1)

client_socket, addr = server_socket.accept()
print(f"Connection from {addr}")

#client_socket.send("READY".encode("ascii"))

# Set arduino connected port
serial_port = '/dev/ttyUSB0' #'COM4'

# Open serial port
ser = serial.Serial(serial_port, 9600)  
#ser.open()


while True:
    # Tell GUI client that it can now receive dispenser commands
    client_socket.send("READY".encode("ascii"))

    # Wait for commands
    request = client_socket.recv(1024).decode("ascii")

    if request.isdigit():
        # Send the motor number to the Arduino
        ser.write(request.encode())
    elif request == "STOP":
        break
    else:
        client_socket.send("ERROR".encode("ascii"))
        print(f"Received non-digit: {request}")
        continue

    # Wait for components to be pushed out
    data = ser.readline().decode().strip()

    if data == "OUT":
        time.sleep(2) #2 sec to slide down
        print("Component ready!")
        client_socket.send("FINISHED".encode("ascii"))
    else:
        print("Error")
        client_socket.send("ERROR".encode("ascii"))

    data = ser.readline().decode().strip()

    if data == "FINISHED":
        print("Dispenser ready!")
        #client_socket.send("FINISHED".encode("ascii"))
    else:
        print("Error")
        client_socket.send("ERROR".encode("ascii"))

    # To avoid congestion wait 200 ms
    time.sleep(0.2)

    


client_socket.close()
ser.close()

