"""
    A script to quickly send script commands directly to the UR robot.
"""

import sys
import socket


# Create a socket object
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Define the server address and port
# Set ip address of the robot, but keep the port at 30002 (see URScript documentation)
server_address = ('169.254.15.109', 30002)

# Connect to the UR robot
client_socket.connect(server_address)

print("connected successfully")

# Script command to send
# See URScript documentation for possible commands
script_command = "movej([-1.745609, -1.516291, 2.226883, -1.110363, -4.987156, 3.255737],1.8, 0.5,0,0)" + "\n"

# Send the script command
client_socket.send(script_command.encode("ascii"))

# Close the socket when done
client_socket.close()