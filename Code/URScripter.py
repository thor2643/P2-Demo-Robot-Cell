"""
A script to quickly send script commands directly to the UR robot.
"""

import sys
import socket


# Create a socket object
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Define the server address and port
# Set ip address of the robot, but keep the port at 30002
server_address = ('169.254.15.109', 30002)

# Connect to the server
client_socket.connect(server_address)

print("connected successfully")

#Script command to send
script_command = "movej([-1.7456, -1.5051, 2.1969, -1.0906, -4.9803, 3.2557], 1.8, 0.336, 0, 0)" + "\n"

client_socket.send(script_command.encode("ascii"))

# Close the socket when done
client_socket.close()