#pragma once
#include <WS2tcpip.h>
#include <chrono>
#include <iostream>

enum class CommandState
{
    Unknown,
    Quit,
    Message
};

class Timer
{
public:
    void start()
    {
        m_StartTime = std::chrono::system_clock::now();
        m_EndTime = std::chrono::system_clock::now();
        m_bRunning = true;
    }

    void stop()
    {
        m_EndTime = std::chrono::system_clock::now();
        m_bRunning = false;
    }

    double elapsedMilliseconds()
    {
        std::chrono::time_point<std::chrono::system_clock> endTime;

        if (m_bRunning)
        {
            endTime = std::chrono::system_clock::now();
        }
        else
        {
            endTime = m_EndTime;
        }

        return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_StartTime).count();
    }

    double elapsedSeconds()
    {
        return elapsedMilliseconds() / 1000.0;
    }

private:
    std::chrono::time_point<std::chrono::system_clock> m_StartTime;
    std::chrono::time_point<std::chrono::system_clock> m_EndTime;
    bool                                               m_bRunning = false;
};

enum class PacketType
{
    Update,
    Message,
};

class PacketBase
{
public:
    PacketType m_type = PacketType::Update;
    uint32_t m_packetSize = 0;
};

class PacketMessage
{
public:
    PacketMessage()
    {
        m_base.m_type = PacketType::Message;
        m_base.m_packetSize = sizeof(PacketMessage);
    }
    PacketBase m_base;
    char m_buffer[200];
};

enum class ThreadState
{
    BrandNew,
    Live,
    NeedToStop,
    Dead
};

// Multiplayer Game Programming, Andre LaMothe, Todd Barron
// https://www.amazon.com/Multiplayer-Programming-Prima-Techs-Development/dp/0761532986
class SocketObject
{
private:
public:
    SOCKET skSocket = INVALID_SOCKET;
    SocketObject() {}
    ~SocketObject()
    {
        Disconnect();
    }

    int Bind(int iPort)
    {
        sockaddr_in saServerAddress;
        skSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (skSocket == INVALID_SOCKET)
        {
            return false;
        }
        memset(&saServerAddress, 0, sizeof(sockaddr_in));
        saServerAddress.sin_family = AF_INET;
        saServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        saServerAddress.sin_port = htons(iPort);
        if (bind(skSocket, (sockaddr*)&saServerAddress, sizeof(sockaddr)) == SOCKET_ERROR)
        {
            Disconnect();
            return false;
        }
        else
            return true;
    }

    void Disconnect()
    {
        if (skSocket != INVALID_SOCKET)
        {
            closesocket(skSocket);
            skSocket = INVALID_SOCKET;
        }
    }

    int Listen(void)
    {
        return listen(skSocket, 32);
    }

    bool Accept(SocketObject& skAcceptSocket)
    {
        sockaddr_in saClientAddress;
        int iClientSize = sizeof(sockaddr_in);
        skAcceptSocket.skSocket = accept(skSocket, (struct sockaddr*)&saClientAddress, &iClientSize);
        if (skAcceptSocket.skSocket == INVALID_SOCKET)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    bool Connect(char* szServerAddress, int iPort)
    {
        struct sockaddr_in serv_addr;
        // LPHOSTENT lpHost;
        int err;

        skSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (skSocket == INVALID_SOCKET)
        {
            return false;
        }
        memset(&serv_addr, 0, sizeof(sockaddr_in));
        serv_addr.sin_family = AF_INET;

        //serv_addr.sin_addr.s_addr = inet_addr(szServerAddress);
        inet_pton(AF_INET, szServerAddress, &serv_addr.sin_addr.s_addr);

        if (serv_addr.sin_addr.s_addr == INADDR_NONE)
        {
            struct addrinfo hints;
            ZeroMemory(&hints, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            struct addrinfo* result = NULL;
            struct addrinfo* ptr = NULL;
            if (getaddrinfo(szServerAddress, 0, &hints, &result) != 0)
            {
                return FALSE;
            }

            int i = 1;
            for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

                std::cout << "getaddrinfo response" << i++;
                std::cout << "\tFlags: 0x" << std::ios::hex << ptr->ai_flags << "\n";
                std::cout << "\tFamily: ";
                switch (ptr->ai_family) {
                case AF_UNSPEC:
                    std::cout << "Unspecified\n";
                    break;
                case AF_INET:
                {
                    std::cout << "AF_INET (IPv4)\n";
                    struct sockaddr_in* sockaddr_ipv4;
                    sockaddr_ipv4 = (struct sockaddr_in*)ptr->ai_addr;

                    //printf("\tIPv4 address %s\n", inet_ntoa(sockaddr_ipv4->sin_addr));
                    char ipstringbuffer[46];
                    inet_ntop(ptr->ai_family, sockaddr_ipv4, ipstringbuffer, INET_ADDRSTRLEN);

                    serv_addr.sin_addr = sockaddr_ipv4->sin_addr;
                }break;
                case AF_INET6:
                {

                    std::cout << "AF_INET6 (IPv6)\n";
                    // the InetNtop function is available on Windows Vista and later
                    // sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
                    // printf("\tIPv6 address %s\n",
                    //    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );

                    // We use WSAAddressToString since it is supported on Windows XP and later
                    LPSOCKADDR sockaddr_ip;
                    sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
                    // The buffer length is changed by each call to WSAAddresstoString
                    // So we need to set it for each iteration through the loop for safety
                    DWORD ipbufferlength = 46;
                    wchar_t ipstringbuffer[46];
                    INT iRetval;
                    iRetval = WSAAddressToStringW(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL,
                        ipstringbuffer, &ipbufferlength);
                    if (iRetval)
                        printf("WSAAddressToString failed with %u\n", WSAGetLastError());
                    else
                        wprintf(L"\tIPv6 address %s\n", ipstringbuffer);
                }break;
                case AF_NETBIOS:
                    printf("AF_NETBIOS (NetBIOS)\n");
                    break;
                default:
                    printf("Other %ld\n", ptr->ai_family);
                    break;
                }
                printf("\tSocket type: ");
                switch (ptr->ai_socktype) {
                case 0:
                    printf("Unspecified\n");
                    break;
                case SOCK_STREAM:
                    printf("SOCK_STREAM (stream)\n");
                    break;
                case SOCK_DGRAM:
                    printf("SOCK_DGRAM (datagram) \n");
                    break;
                case SOCK_RAW:
                    printf("SOCK_RAW (raw) \n");
                    break;
                case SOCK_RDM:
                    printf("SOCK_RDM (reliable message datagram)\n");
                    break;
                case SOCK_SEQPACKET:
                    printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
                    break;
                default:
                    printf("Other %ld\n", ptr->ai_socktype);
                    break;
                }
                printf("\tProtocol: ");
                switch (ptr->ai_protocol) {
                case 0:
                    printf("Unspecified\n");
                    break;
                case IPPROTO_TCP:
                    printf("IPPROTO_TCP (TCP)\n");
                    break;
                case IPPROTO_UDP:
                    printf("IPPROTO_UDP (UDP) \n");
                    break;
                default:
                    printf("Other %ld\n", ptr->ai_protocol);
                    break;
                }
                printf("\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
                printf("\tCanonical name: %s\n", ptr->ai_canonname);
            }
            /*lpHost = gethostbyname(szServerAddress);
            if (lpHost != NULL)
            {
                serv_addr.sin_addr.s_addr = ((LPIN_ADDR)lpHost->h_addr)->s_addr;
            }
            else
            {
                return FALSE;
            }*/
        }

        serv_addr.sin_port = htons(iPort);

        err = connect(skSocket, (struct sockaddr*)&serv_addr, sizeof(sockaddr));
        if (err == SOCKET_ERROR)
        {
            Disconnect();
            return false;
        }
        return true;
    }

    int Send(char* szBuffer, int iBufLen, int iFlags)
    {
        return send(skSocket, szBuffer, iBufLen, iFlags);
    }

    int Recv(char* szBuffer, int iBufLen, int iFlags)
    {
        return recv(skSocket, szBuffer, iBufLen, iFlags);
    }
};

void DoServer(int iListenPort);
void DoClient(char*, int iListenPort);
