#ifndef TCP_SOCKET_HPP
#define TCP_SOCKET_HPP

#ifdef _WIN32
  /* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
  #define NTDDI_VERSION NTDDI_VISTA 
  //#define _WIN32_WINNT _WIN32_WINNT_VISTA 

  #include <winsock2.h>
  #include <Ws2tcpip.h>

  const char REUSABLE = '1';

#else
  /* Assume that any non-Windows platform uses POSIX-style sockets instead. */
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <netdb.h>  /* Needed for getaddrinfo() and freeaddrinfo() */
  #include <unistd.h> /* Needed for close() */
  typedef int SOCKET;
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR -1
  #define WSAEWOULDBLOCK EWOULDBLOCK 
  #define WSAENOTSOCK ENOTSOCK
  int REUSABLE = 1;

#endif

class URSocket {
    public:
        URSocket(int port);
        ~URSocket();

        int SockInit(int port);
        int SockQuit(void);
        int SockClose(SOCKET sock);

        bool Connected();
        bool AcceptConnection();
        void HandleConnection(char* msg);
        void Send();
        void Disconnect();


    private:
      bool _connected;

      SOCKET _socket;
      SOCKET _server_socket;

};

#endif 