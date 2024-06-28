import sys
import socket

#Init TCP/IP connection
# specify Host and Port 
#Listen to all incoming requests by setting host to be empty
HOST = '' 
PORT = 50005

soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    soc.bind((HOST, PORT))

except socket.error as message: 
    # if any error occurs then with the 
    # help of sys.exit() exit from the program
    print('Bind failed. Error Code : '
        + str(message[0]) + ' Message '
        + message[1])
    sys.exit()


def print_socket_info(sock):
    sock_name = sock.getsockname()
    print(f"Socket is bound to IP: {sock_name[0]} and port: {sock_name[1]}")

print_socket_info(soc)

print('Socket binding operation completed')
print('Now listening...')

#Listen to incoming connections
soc.listen(1)

#Accept the incoming request
conn, address = soc.accept()

# print the address of connection
print('Connected with ' + address[0] + ':'+ str(address[1]))

while True:
    #UR socket script commands expects to recive ASCII encoded characters
    msg = "Run Program" #input("Write command: ")
    msg_bin = msg.encode("ASCII")

    conn.send(msg_bin)

    #print("Message Sent. Expecting to receive an answer...")

    msg_recv = conn.recv(1024)

    msg_decode = msg_recv.decode("ASCII")
    print(f"Received reply: {msg_decode}")