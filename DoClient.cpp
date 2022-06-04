#include "common.h"
#include <iostream>
#include <thread>
#include <string>
#include <stdio.h>
using namespace std;

static std::string g_cmd;

struct ClientContext
{
    SocketObject* recvObject = 0;
    ThreadState m_state = ThreadState::BrandNew;
};

void ClientRecv(ClientContext* c)
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

void DoClient(char* szServerIP, int iServerListenPort)
{
    SocketObject ClientSocketObjectSend;
    SocketObject ClientSocketObjectRecv;
    printf("<Client> Connecting to %s, Port %i\n", szServerIP, iServerListenPort);
    
    if (ClientSocketObjectSend.Connect(szServerIP, iServerListenPort))
        printf("<Client> Connection for Send - OK\n");
    else
        printf("<Client> FAIL\n");

    if (ClientSocketObjectRecv.Connect(szServerIP, iServerListenPort))
        printf("<Client> Connection for Recv - OK\n");
    else
        printf("<Client> FAIL\n");

    ClientContext c;
    c.recvObject = &ClientSocketObjectRecv;
    PacketBase pcktUpdate;
    PacketMessage pcktMsgOut;

    std::thread t(ClientRecv, &c);

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
            printf("<Client> Server disconnected\n");

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

