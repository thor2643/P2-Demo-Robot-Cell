"""
    For quick implementation and to avoid having the serial communication block the main GUI
    the dispenser communication is done through this python script. A local socket connection
    is used to talk with the main GUI program.
"""

import socket
import serial

import serial


# Set arduino connected port
serial_port = '/dev/ttyUSB0' #'COM4'

# Open serial port
ser = serial.Serial(serial_port, 9600)  

# Set address and port
host = "localhost"
port = 50010

# Create socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((host, port))
server_socket.listen(1)

print(f"Server listening on {host}:{port}")

# Accept connection request from GUI program
client_socket, addr = server_socket.accept()
print(f"Connection from {addr}")