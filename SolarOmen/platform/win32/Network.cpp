#include "core/SolarCore.h"
#include <ws2tcpip.h>
#include <winsock2.h>

namespace cm::Platform
{


	static bool InitializeSockets()
	{
		WSADATA WsaData = {};
		WORD dllVersion = MAKEWORD(2, 2);
		bool result = WSAStartup(dllVersion, &WsaData) == NO_ERROR;

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
						int clientLength = sizeof(client);

						while (true)
						{
							char buf[1024] = {};

							int bytesIn = recvfrom(handle, buf, 1024, 0, (sockaddr*)&client, &clientLength);


							char clientIp[256] = {};
							//inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);
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

//
//addrinfo* ptr = NULL;
//
//
//sockaddr_in* sockaddr_ipv4;
////    struct sockaddr_in6 *sockaddr_ipv6;
//LPSOCKADDR sockaddr_ip;
//
//char ipstringbuffer[46];
//DWORD ipbufferlength = 46;
//
//int iResult;
//INT iRetval;
//
//addrinfo hint;
//memset(&hint, 0, sizeof(hint));
//hint.ai_family = AF_INET;
//addrinfo* result;
//
//int error = getaddrinfo("www.google.com", "0",
//	&hint, &result);
//int i = 1;
//char myId[256] = {};
//for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
//
//	printf("getaddrinfo response %d\n", i++);
//	printf("\tFlags: 0x%x\n", ptr->ai_flags);
//	printf("\tFamily: ");
//	switch (ptr->ai_family) {
//	case AF_UNSPEC:
//		printf("Unspecified\n");
//		break;
//	case AF_INET:
//		printf("AF_INET (IPv4)\n");
//		sockaddr_ipv4 = (struct sockaddr_in*)ptr->ai_addr;
//
//
//		inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, myId, 256);
//
//		printf("\tIPv4 address %s\n", myId);
//		break;
//	default:
//		printf("Other %ld\n", ptr->ai_family);
//		break;
//	}
//	printf("\tSocket type: ");
//	switch (ptr->ai_socktype) {
//	case 0:
//		printf("Unspecified\n");
//		break;
//	case SOCK_STREAM:
//		printf("SOCK_STREAM (stream)\n");
//		break;
//	case SOCK_DGRAM:
//		printf("SOCK_DGRAM (datagram) \n");
//		break;
//	case SOCK_RAW:
//		printf("SOCK_RAW (raw) \n");
//		break;
//	case SOCK_RDM:
//		printf("SOCK_RDM (reliable message datagram)\n");
//		break;
//	case SOCK_SEQPACKET:
//		printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
//		break;
//	default:
//		printf("Other %ld\n", ptr->ai_socktype);
//		break;
//	}
//	printf("\tProtocol: ");
//	switch (ptr->ai_protocol) {
//	case 0:
//		printf("Unspecified\n");
//		break;
//	case IPPROTO_TCP:
//		printf("IPPROTO_TCP (TCP)\n");
//		break;
//	case IPPROTO_UDP:
//		printf("IPPROTO_UDP (UDP) \n");
//		break;
//	default:
//		printf("Other %ld\n", ptr->ai_protocol);
//		break;
//	}
//}