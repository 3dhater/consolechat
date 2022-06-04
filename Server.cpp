#include "common.h"
#include <iostream>
#include <thread>
#include <string>
#include <stdio.h>
using namespace std;

static std::string g_cmd;

struct ServerContext
{
    SocketObject* recvObject = 0;
    ThreadState m_state = ThreadState::BrandNew;
};

void ServerRecv(ServerContext* c)
{
    c->m_state = ThreadState::Live;
    char receiveBuffer[0xffff];
    PacketMessage* pcktMsg = 0;
    while (true)
    {
        if (c->m_state == ThreadState::NeedToStop)
            break;

        int ret = c->recvObject->Recv(receiveBuffer, 0xffff, 0);
        if (ret > 0)
        {
            PacketBase* pb = (PacketBase*)receiveBuffer;
            switch (pb->m_type)
            {
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
    }
    c->m_state = ThreadState::Dead;
}

void DoServer(int iListenPort)
{
    SocketObject ServerSocketObject;
    SocketObject ClientSocketObjectSend;
    SocketObject ClientSocketObjectRecv;
    printf("<Server> Attempting to listen on Port %i\n", iListenPort);
    if (ServerSocketObject.Bind(iListenPort))
    {
        printf("<Server> Listening for Recv\n");
        if (ServerSocketObject.Listen() == 0)
        {
            if (ServerSocketObject.Accept(ClientSocketObjectRecv))
                printf("<Server> Client Connected\n");
        }
        else
        {
            printf("<Server> FAIL\n");
        }

        printf("<Server> Listening for Send\n");
        if (ServerSocketObject.Listen() == 0)
        {
            if (ServerSocketObject.Accept(ClientSocketObjectSend))
                printf("<Server> Client Connected\n");
        }
        else
        {
            printf("<Server> FAIL\n");
        }
    }
    else
    {
        printf("<Server> FAIL\n");
    }

    ServerContext c;
    c.recvObject = &ClientSocketObjectRecv;

    PacketMessage pcktMsgOut;

    std::thread t(ServerRecv, &c);

    while (true)
    {
        std::cin >> g_cmd;
        if (strcmp(g_cmd.c_str(), "quit") == 0)
        {
            c.m_state = ThreadState::NeedToStop;
            ClientSocketObjectRecv.Disconnect();
            break;
        }

        int charWritten = snprintf(pcktMsgOut.m_buffer, 199, "%s", g_cmd.data());
        int ret = ClientSocketObjectSend.Send((char*)&pcktMsgOut, sizeof(PacketMessage), 0);
        if (ret == 0 || ret == -1)
        {
            printf("<Server> Client disconnected\n");

            c.m_state = ThreadState::NeedToStop;
            ClientSocketObjectRecv.Disconnect();
            break;
        }
    }

    if (t.joinable())
    {
        t.join();
    }

    ClientSocketObjectSend.Disconnect();
    ClientSocketObjectRecv.Disconnect();
}
