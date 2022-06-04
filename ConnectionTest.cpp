// ConnectionTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <thread>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#include "common.h"

using namespace std;

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 0), &wsaData);

    if (argc < 3) {
        cout << "----------------------------------------" << endl;
        cout << " ConnectionTest Help " << endl;
        cout << "----------------------------------------" << endl;
        cout << "Usage: ConnectionTest[client / server] [ip, port / port, port]" << endl;
        cout << "" << endl;
        cout << "Example: ConnectionTest client 198.168.0.1 6000" << endl;
        cout << "" << endl;
        cout << "Example: ConnectionTest server 6000" << endl;
        cout << "" << endl;
        return(0);
    }

   // std::thread * th = 0;
   // ThreadContext thCtx;

    bool isServer = false;
    if (!_stricmp(argv[1], "server"))
        DoServer(atoi(argv[2]));
    else
        DoClient(argv[2], atoi(argv[3]));

    //if (isServer)
    //    th = new std::thread(vServerConnection, &thCtx, atoi(argv[2]));
    //else
    //    th = new std::thread(vClientConnection, &thCtx, argv[2], atoi(argv[3]));


    //// commands here
    //while (true)
    //{
    //    std::cout << ">>> ";
    //    std::getline(std::cin, g_cmd);
    //    if (strcmp(g_cmd.c_str(), "quit") == 0)
    //    {
    //        thCtx.m_state = ThreadState::NeedToStop;
    //        g_cmdState = CommandState::Quit;
    //        break;
    //    }
   
    //    g_cmdState = CommandState::Message;
    //}

    //if (th)
    //{
    //    if (th->joinable())
    //        th->join();
    //    delete th;
    //}

    WSACleanup();

    return(1);
}

/*
void vServerConnection(ThreadContext* ctx, int iListenPort)
{
    ctx->m_state = ThreadState::Live;

    SocketObject ServerSocketObject;
    SocketObject ClientSocketObject;
    cout << "<Server> Attempting to listen on Port " << iListenPort << endl;

    PacketBase pcktUpdate;
    PacketMessage pcktMsgOut;
    PacketMessage* pcktMsg = 0;

    DWORD timeout = 10;

    char receiveBuffer[0xffff];

    if (ServerSocketObject.Bind(iListenPort)) {
        cout << "<Server> Listening" << endl;
        if (ServerSocketObject.Listen() == 0)
        {
            if (ServerSocketObject.Accept(ClientSocketObject))
            {
                setsockopt(ClientSocketObject.skSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(DWORD));
                cout << "<Server> Client Connected to Port " << iListenPort << endl;

                Timer timer;
                timer.start();

                while (true)
                {
                    if (timer.elapsedMilliseconds() > 200.f)
                    {
                        int ret = ClientSocketObject.Send((char*)&pcktUpdate, sizeof(PacketBase), 0);
                        if (!ret || ret == -1)
                        {
                            break;
                        }
                        ret = ClientSocketObject.Recv(receiveBuffer, 0xffff, 0);
                        if (ret > 0)
                        {
                            PacketBase* pb = (PacketBase*)receiveBuffer;
                            switch (pb->m_type)
                            {
                            default:
                                break;
                            case PacketType::Update:
                            break;
                            case PacketType::Message:
                            {
                                pcktMsg = (PacketMessage*)receiveBuffer;
                                if (pcktMsg)
                                {
                                    pcktMsg->m_buffer[199] = 0;
                                    cout << "<Chat>: " << pcktMsg->m_buffer << endl;
                                }
                            }break;
                            }
                        }
                        timer.start();
                    }

                    if (g_cmdState == CommandState::Message)
                    {
                        int charWritten = snprintf(pcktMsgOut.m_buffer, 199, "%s", g_cmd.data());
                        int ret = ClientSocketObject.Send((char*)&pcktMsgOut, sizeof(PacketMessage), 0);
                        g_cmdState = CommandState::Unknown;
                    }

                    if (ctx->m_state == ThreadState::NeedToStop)
                    {
                        break;
                    }
                }
            }
            else
                cout << "<Server> Failed to connect client" << endl;
        }

        ClientSocketObject.Disconnect();
        cout << "<Server> Client Disconnected" << endl;
    }
    else 
    {
        cout << "<Server> Failed to Listen" << endl;
    }
    ctx->m_state = ThreadState::Dead;
}


void vClientConnection(ThreadContext* ctx, char* szServerIP, int iServerListenPort)
{
    ctx->m_state = ThreadState::Live;

    SocketObject ClientSocketObject;
    cout << "<Client> Connecting to " << szServerIP << ", Port " << iServerListenPort << endl;
    // Connect to the IP and port 
    if (ClientSocketObject.Connect(szServerIP, iServerListenPort)) 
    {
        cout << "<Client> Connected" << endl;

        PacketBase pcktUpdate;
        PacketMessage* pcktMsg = 0;
        PacketMessage pcktMsgOut;

        PacketBase* pcktBase = 0;
        char receiveBuffer[0xffff];

        while (true)
        {
            int ret = ClientSocketObject.Recv(receiveBuffer, 0xffff, 0);
            if (ret)
            {
                PacketBase* pb = (PacketBase*)receiveBuffer;
                switch (pb->m_type)
                {
                default:
                    break;
                case PacketType::Update:
                {
                  //  cout << "<Client> Update" << endl;
                    if (g_cmdState == CommandState::Message)
                    {
                        int charWritten = snprintf(pcktMsgOut.m_buffer, 199, "%s", g_cmd.data());
                        int ret = ClientSocketObject.Send((char*)&pcktMsgOut, sizeof(PacketMessage), 0);
                        g_cmdState = CommandState::Unknown;
                    }
                    else
                    {
                        ClientSocketObject.Send((char*)&pcktUpdate, sizeof(PacketBase), 0);
                    }
                }break;
                case PacketType::Message:
                {
                    pcktMsg = (PacketMessage*)receiveBuffer;
                    if (pcktMsg)
                    {
                        pcktMsg->m_buffer[199] = 0;
                        cout << "<Chat>: " << pcktMsg->m_buffer << endl;
                    }
                }break;
                }
            }

            if (ctx->m_state == ThreadState::NeedToStop)
            {
                break;
            }

        }

        // Disconnect from the server
        ClientSocketObject.Disconnect();
        cout << "<Client> Disconnected From Server" << endl;
    }
    else {
        cout << "<Client> Failed to Connect" << endl;
    }
    ctx->m_state = ThreadState::Dead;
}
*/
