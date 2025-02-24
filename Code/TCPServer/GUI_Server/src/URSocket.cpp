#include "URSocket.hpp"
#include <iostream>

#ifdef _WIN32
    const char REUSABLE = '1';
#else
    int REUSABLE = 1;
#endif

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
    } else {
        std::cout << "Done initialising socket" << std::endl; 
    }      
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
        //return 0;
    #endif

    // Create TCP socket
	_server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (_server_socket == INVALID_SOCKET) {
        int error_code = GetError();
        std::cout << "Failed to create Socket: " << error_code << "\n";
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

bool URSocket::HandleConnection(char* msg){
    char recv_buf[1024];
    //std::cout << "Receiving data\n";
    int result = recv(_socket, recv_buf, 1024, 0);
    //std::cout << "Received data\n";

    // Check if anything has been received or if an error has occured
    if (result <= 0) 
    {
        int error_code = GetError();

        if (error_code == WSAEWOULDBLOCK) {
            return false;
        } else if (error_code == WSAENOTSOCK) {
            // TODO: Enable reconnection
            std::cout << "FATAL ERROR: Failed to recv as socket is not valid: Error " << error_code << std::endl;
            SockClose(_socket);
            _connected = false;
            return false;
        } else if (result == 0 || error_code == WSAECONNRESET) {
            std::cout << "Failed to receive: socket closed: " << error_code << "\n";
            SockClose(_socket);
            _connected = false;
            return false;
        }else {
            std::cout << "FATAL ERROR: Failed to recv: Error " << error_code << std::endl;
            SockClose(_socket);
            _connected = false;
            return false;
        }

    } else {
        // Else handle received data
        printf("Bytes received: %d\n", result);

        // Copy received buffer into msg pointer
        strcpy(msg, recv_buf);

        return true;
    } 
}

bool URSocket::Send(char* msg, int msg_type){
    int result;

    if (msg_type == 1){ //string type
        result = send(_socket, msg, (int)strlen(msg), 0);
    } else if (msg_type == 2) { //int type
        result = send(_socket, msg, sizeof(int), 0);
    } else if (msg_type == 3) { //for sending cover refill
        int size = 7 * sizeof(int);
        result = send(_socket, msg, size, 0);
    } else if (msg_type == 4) { //for sending fuse and pcb refill
        int size = 2 * sizeof(int);
        result = send(_socket, msg, size, 0);
    }

    // Clear buffer
    memset(msg, 0, 1024);
    
    if (result <= 0) {
        int err = GetError();
        if (err == WSAEWOULDBLOCK) {
            std::cout << "Failed to send: would block: " << err << "\n";
            return false;
        }

        // returning 0 or WSAECONNRESET means closed by host
        else if (result == 0 || err == WSAECONNRESET) {
            std::cout << "Failed to send: socket closed: " << err << "\n";
            SockClose(_socket);
            _connected = false;
            return false;
        }
        else
        {
            // everything else is error
            std::cout << "Failed to send: send Error: " << err;
            SockClose(_socket);
            _connected = false;
            return false;
        }
    } else {
        return true;
    }
}

void URSocket::Disconnect(){

}

bool URSocket::Connected(){
    return _connected;
}



RoboDKClient::RoboDKClient(): _connected(false){ 
    std::cout << "creating client socket... " << std::endl;   
    if(SockInit() != true) {
        std::cout << "Failed to init Socket\n";
    } else {
        std::cout << "Socket created succesfully!\n"; 
    } 
}

RoboDKClient::~RoboDKClient(){
    SockClose(_socket);
    SockQuit();         
}

int RoboDKClient::SockInit(){
    #ifdef _WIN32
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(1,1), &wsa_data);
    #else
        //return 0;
    #endif

    // Create TCP socket
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    
	if (_socket == INVALID_SOCKET) {
        std::cout << "Failed to create Socket\n";
		return false;
	}

    // set socket to reusable
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &REUSABLE, sizeof(int)) == -1) {
        std::cout << "Failed to reuse address\n";
        SockClose(_socket);
        return false;
    }
 
    // set socket to be non-blocking
    if(!set_nonblocking(_socket)){
        std::cout << "Failed to set non-blocking\n";
        SockClose(_socket);
        return false;
    }

    return true;
}

int RoboDKClient::SockQuit()
{
    #ifdef _WIN32
        return WSACleanup();
    #else
        //return 0;
    #endif
}

int RoboDKClient::SockClose(SOCKET sock){
    int status = 0;

    #ifdef _WIN32
    status = shutdown(sock, SD_BOTH);
    if (status == 0) { status = closesocket(sock); }
    #else
    status = shutdown(sock, SHUT_RDWR);
    if (status == 0) { status = close(sock); }
    #endif
    
    SockQuit();
    return status;

}

bool RoboDKClient::Connect(const char* host, int port){
    // Server address
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port); // Port should match the server port

	if (inet_pton(AF_INET, host, &serverAddr.sin_addr) != 1) {
		std::cout << "Ivalid address\n";
        SockClose(_socket);
        return false;
	}

    // Try to connect to server
    int result = connect(_socket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    
    if (result == SOCKET_ERROR) {
        int error_code = GetError();

        if (error_code == WSAEWOULDBLOCK || error_code == WSAEALREADY || error_code == WSAEINVAL) {
            // In non-blockin mode the connect() return error will return error
            // while it is finishing the connection routine. Read more at winsock2 or sys/socket websites.
            return false;
        } else if (error_code == WSAEISCONN){
            // The socket has connected succesfully and we therefore continue
        } else {
            std::cout << "Failed to connect" << error_code << std::endl;
            return false;
        }
    }

    // set socket to reusable
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &REUSABLE, sizeof(int)) == -1) {
        std::cout << "Failed to reuse address\n";
        SockClose(_socket);
        return false;
    }

	// set the socket to non-blocking
    set_nonblocking(_socket);

	std::cout << "Client Connection Established" << std::endl;
    _connected = true;
	return true;
}

bool RoboDKClient::HandleConnection(char* msg){
    char recv_buf[1024];
    int result = recv(_socket, recv_buf, 1024, 0);

    // Check if anything has been received or if an error has occured
    if (result <= 0) 
    {
        int error_code = GetError();

        if (error_code == WSAEWOULDBLOCK) {
            return false;
        } else if (error_code == WSAENOTSOCK) {
            // TODO: Enable reconnection
            std::cout << "FATAL ERROR: Failed to recv as socket is not valid: Error " << error_code << std::endl;
            SockClose(_socket);
            _connected = false;
            return false;
        } else if (result == 0 || error_code == WSAECONNRESET) {
            std::cout << "Failed to send: socket closed: " << error_code << "\n";
            SockClose(_socket);
            _connected = false;
            return false;
        }else {
            std::cout << "FATAL ERROR: Failed to recv: Error " << error_code << std::endl;
            SockClose(_socket);
            _connected = false;
            return false;
        }

    } else {
        // Else handle received data
        printf("Bytes received: %d\n", result);

        // Copy received buffer into msg pointer
        strcpy(msg, recv_buf);

        return true;
    } 
}

void RoboDKClient::Send(char* msg){
    int result = send(_socket, msg, (int)strlen(msg), 0);

    if (result <= 0) {
        int err = GetError();
        if (err == WSAEWOULDBLOCK) {
            std::cout << "Failed to send: would block: " << err << "\n";
            return;
        }

        // returning 0 or WSAECONNRESET means closed by host
        if (result == 0 || err == WSAECONNRESET) {
            std::cout << "Failed to send: socket closed: " << err << "\n";
            SockClose(_socket);
            _connected = false;
        }
        else
        {
            // everything else is error
            std::cout << "Failed to send: send Error: " << err;
            SockClose(_socket);
            _connected = false;
        }
        return;
    }
}

void RoboDKClient::Disconnect(){
    // everything else is error
    std::cout << "Disconnecting and closing socket.\n";
    SockClose(_socket);
    _connected = false;
}

bool RoboDKClient::Connected(){
    return _connected;
}
