"""
A script to quickly test if a TCP server can accept an incoming connection
"""

import sys
import socket


# Create a socket object
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Define the server address and port
server_address = ('', 50005)

# Connect to the server
client_socket.connect(server_address)

print("connected successfully")

while True:
    x = input()
    if x == "q":
        break

# Close the socket when done
client_socket.close()