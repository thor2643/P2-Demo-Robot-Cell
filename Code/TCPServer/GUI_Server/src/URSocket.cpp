#include "URSocket.hpp"
#include <iostream>

bool set_nonblocking(SOCKET my_socket){
  #ifdef _WIN32
    // set the socket to non-blocking
    u_long mode = 1;  // 1 enables non-blocking mode
    if (ioctlsocket(my_socket, FIONBIO, &mode) == SOCKET_ERROR) {
      WSACleanup();
      return false;
    }

    return true;
  #else
    if (fcntl(my_socket, F_SETFL, fcntl(my_socket, F_GETFL) | O_NONBLOCK) == -1) {
      return false;
    }

    return true;
  #endif
}

int GetError(){
    #ifdef _WIN32
        return WSAGetLastError();
    #else
        return errno;
  #endif
}

void print_socket_info(SOCKET socket) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    if (getsockname(socket, (struct sockaddr*)&addr, &addr_len) == 0) {
        std::cout << "Socket is bound to IP: " << inet_ntoa(addr.sin_addr)
                  << " and port: " << ntohs(addr.sin_port) << std::endl;
    } else {
        std::cerr << "Failed to get socket name" << std::endl;
    }
}

URSocket::URSocket(int port): _connected(false){
    std::cout << "Initialising socket... " << std::endl;

    if(SockInit(port) != true) {
        std::cout << "Failed to init Socket\n";
    }  

    std::cout << "Done initialising socket" << std::endl;       
}

URSocket::~URSocket(){
    SockQuit();         
}

int URSocket::SockInit(int port)
{
    std::cout << "creating server socket... " << std::endl;
    #ifdef _WIN32
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(1,1), &wsa_data);
    #else
        return 0;
    #endif

    // Create TCP socket
	_server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (_server_socket == INVALID_SOCKET) {
        std::cout << "Failed to create Socket\n";
		return false;
	}

    // set socket to reusable
    if (setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, &REUSABLE, sizeof(int)) == -1) {
        std::cout << "Failed to reuse address\n";
        SockClose(_server_socket);
        return false;
    }
 
    // set socket to be non-blocking
    if(!set_nonblocking(_server_socket)){
        std::cout << "Failed to set non-blocking\n";
        SockClose(_server_socket);
        return false;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; //inet_addr("192.168.56.103");//;
    serverAddress.sin_port = htons(port);

    // bind to address and port
    if (bind(_server_socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cout << "Failed to bind socket\n";
        SockClose(_server_socket);
        return false;
    }

    print_socket_info(_server_socket);

    // listen to incoming connections
    if (listen(_server_socket,5) == SOCKET_ERROR) {
        std::cout << "Failed to listen\n";
        SockClose(_server_socket);
        return false;
    }

    std::cout << "Server Running on port: " << port << std::endl;

    return true;
}

int URSocket::SockQuit(void)
{
    #ifdef _WIN32
        return WSACleanup();
    #else
        return 0;
    #endif
}

int URSocket::SockClose(SOCKET sock){
    int status = 0;

    #ifdef _WIN32
    status = shutdown(sock, SD_BOTH);
    if (status == 0) { status = closesocket(sock); }
    #else
    status = shutdown(sock, SHUT_RDWR);
    if (status == 0) { status = close(sock); }
    #endif

    return status;

}

bool URSocket::AcceptConnection(){
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);


    /* Used for debugging connections. Prints if any incoming connections are available.
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(_server_socket, &read_fds);

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    int result = select(_server_socket + 1, &read_fds, NULL, NULL, &timeout);
    if (result > 0 && FD_ISSET(_server_socket, &read_fds)) {
        std::cout << "There are pending connections" << std::endl;
    } else {
        std::cout << "No pending connections" << std::endl;
    }*/

    SOCKET socket = accept(_server_socket, (struct sockaddr*)&addr, &addrLen);

    if (socket == INVALID_SOCKET) {
        int error_code = GetError();
        if (error_code != WSAEWOULDBLOCK) {
            std::cout << "FATAL ERROR: Failed to accept" << error_code << std::endl;
            SockClose(socket);
            return false;
        }

        return false;
    }

    // set socket to be non-blocking
    if(!set_nonblocking(socket)){
        std::cout << "Failed to set non-blocking\n";
        SockClose(socket);
        return false;
    }

    std::cout << "Client Connection Established" << std::endl;
    _connected = true;
    _socket = socket;

    return true;

}



void URSocket::HandleConnection(char* msg){
    char recv_buf[1024];
    int result = recv(_socket, recv_buf, 1024, 0);

    // Check if anything has been received or if an error has occured
    if (result <= 0) 
    {
        int error_code = GetError();

        if (error_code == WSAEWOULDBLOCK) {
            return;
        } else if (error_code == WSAENOTSOCK) {
            std::cout << "FATAL ERROR: Failed to recv as socket is no valid: Error " << error_code << std::endl;
            SockClose(_socket);
            return;
        } else {
            std::cout << "FATAL ERROR: Failed to recv: Error " << error_code << std::endl;
            SockClose(_socket);
            return;
        }

    }

    // Else handle received data
    printf("Bytes received: %d\n", result);

    // Put the recv bytes into msg and format it as a string
    sprintf(msg, "%s", recv_buf);






}

void URSocket::Send(){

}

void URSocket::Disconnect(){

}

bool URSocket::Connected(){
    return _connected;
}
