#include "Network.h"

#include <ws2tcpip.h>
#include <winsock2.h>

namespace cm::Platform
{
	static bool InitializeSockets()
	{
		WSADATA WsaData = {};
		WORD dllVersion = MAKEWORD(2, 2);
		bool32 result = WSAStartup(dllVersion, &WsaData) == NO_ERROR;

		return result;
	}

	void ShutdownSockets()
	{
		WSACleanup();
	}


	void CreateNetworking()
	{
		return;
		if (InitializeSockets())
		{
			SOCKET handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (handle > 0)
			{
				sockaddr_in address = {};
				address.sin_family = AF_INET;
				address.sin_addr.s_addr = INADDR_ANY;
				address.sin_port = htons(30000);


				char myId[256] = {};
				inet_ntop(AF_INET, &address.sin_addr, myId, 256);
				LOG(myId);

				if (bind(handle, (const sockaddr*)&address, sizeof(sockaddr_in)) != SOCKET_ERROR)
				{
					DWORD nonBlocking = 1;
					if (ioctlsocket(handle, FIONBIO, &nonBlocking) == 0)
					{
						//unsigned int a = 207;
						//unsigned int b = 45;
						//unsigned int c = 186;
						//unsigned int d = 98;
						//unsigned short port = 30000;
						//int32 sent_bytes = sendto(handle, (const char*)packet_data, packet_size, 0, (sockaddr*)&address, sizeof(sockaddr_in));

						sockaddr_in client = {};
						int32 clientLength = sizeof(client);

						while (true)
						{
							char buf[1024] = {};

							int bytesIn = recvfrom(handle, buf, 1024, 0, (sockaddr*)&client, &clientLength);


							char clientIp[256] = {};
							//inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);
							LOG(clientIp);
						}
					}
					else
					{
						// @TODO: Error
					}

				}
				else
				{
					// @TODO: Error
				}
			}
			else
			{
				// @TODO: Error
			}
		}
		else
		{
			// @TODO: Error
		}
	}

	void ShudownNewtorking()
	{
		return;
		ShutdownSockets();
	}
}
