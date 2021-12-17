#include "core/SolarPlatform.h"

#include "Win32Socket.h"


namespace cm
{
	namespace Platform
	{
		static bool InitializeSockets()
		{
			WSADATA WsaData = {};
			WORD dllVersion = MAKEWORD(2, 2);
			bool result = WSAStartup(dllVersion, &WsaData) == NO_ERROR;

			return result;
		}

		void IntializeNetworking()
		{
			if (InitializeSockets())
			{
			}
			else
			{
				Assert(0, "Error initializing sockets");
			}
		}



		static bool32 mainSocketBound = false;
		static UDPSocket mainSocket = {};

		PlatformAddress NetworkStart(uint16 port)
		{
			SocketAddress hostAddr = SocketAddress(port);
			mainSocket.Create();
			mainSocket.Bind(hostAddr);
			mainSocket.SetNonBlocking();
			Debug::LogInfo(CString("Server started on ").Add(hostAddr.GetStringIp()).Add(port));
			mainSocketBound = true;

			PlatformAddress platformSocket = hostAddr.ToPlatformAddress();

			return platformSocket;
		}

		int32 NetworkReceive(void* buf, int32 bufSizeBytes, PlatformAddress* platformAddress)
		{
			Assert(bufSizeBytes <= MAX_NETWORK_PACKET_SIZE, "Network buffer size to large !");

			if (mainSocketBound)
			{
				uint8 buffer[MAX_NETWORK_PACKET_SIZE] = {};
				uint32 bufferSize = sizeof(buffer);

				SocketAddress address = {};
				int32 bytes = mainSocket.ReceiveFrom(buffer, bufferSize, &address);

				if (bytes <= 0)
				{
					return bytes;
				}

				if (platformAddress)
				{
					*platformAddress = address.ToPlatformAddress();
				}

				memcpy(buf, buffer, bytes);

				return bytes;
			}

			return -1;
		}

		void NetworkSend(void* buf, int32 bufSizeBytes, const PlatformAddress& address)
		{
			Assert(bufSizeBytes <= MAX_NETWORK_PACKET_SIZE, "Network buffer size to large !");
			SocketAddress sendToAddress = SocketAddress(address);
			mainSocket.SendTo(buf, bufSizeBytes, sendToAddress);
		}

		void NetworkSend(void* buf, int32 bufSizeBytes, const CString& address, uint16 port)
		{
			Assert(bufSizeBytes <= MAX_NETWORK_PACKET_SIZE, "Network buffer size to large !");
			SocketAddress sendToAddress = SocketAddress(address, port);
			mainSocket.SendTo(buf, bufSizeBytes, sendToAddress);
		}

		void ShutdownNetworking()
		{
			WSACleanup();
		}
	}
}