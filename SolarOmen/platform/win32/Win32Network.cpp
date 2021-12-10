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
				//UDPSocket socket = {};
				//socket.Create();
				//SocketAddress hostAddr = SocketAddress(54000);
				//socket.Bind(hostAddr);


				//if (asServer)
				//{
				//	UDPSocket socket;
				//	SocketAddress hostAddr = SocketAddress(54000);

				//	socket.Create();
				//	socket.Bind(hostAddr);

				//	LOG(hostAddr.GetStringIp().GetCStr());

				//	while (true)
				//	{
				//		char inputBuffer[1024];
				//		ZeroArray(inputBuffer);

				//		SocketAddress client;
				//		int32 bytesIn = socket.ReceiveFrom(inputBuffer, 1024, client);

				//		LOG(client.GetStringIp().GetCStr());
				//	}
				//	socket.Close();
				//}
				//else
				//{
				//	SocketAddress serverAddr = SocketAddress("192.168.0.107", 54000);
				//	UDPSocket socket;
				//	socket.Create();

				//	char outBuffer[256] = {};
				//	socket.SendTo(outBuffer, 256, serverAddr);



				//	socket.Close();
				//}
			}
			else
			{
				Assert(0, "Error initializing sockets");
			}
		}

		static bool32 connectionEsablished = false;
		static bool32 mainSocketBound = false;
		static UDPSocket mainSocket = {};
		static bool32 sendAddressValid = false;
		static SocketAddress sendToAddress = {};

		void NetworkStart()
		{
			SocketAddress hostAddr = SocketAddress(54000);
			mainSocket.Create();
			mainSocket.Bind(hostAddr);
			mainSocket.SetNonBlocking();
			Debug::LogInfo(CString("Server started on ").Add(hostAddr.GetStringIp()));
			mainSocketBound = true;
		}

		void NetworkStart(const CString& ip)
		{
			SocketAddress myAddr = SocketAddress(54001);
			mainSocket.Create();
			mainSocket.Bind(myAddr);
			mainSocket.SetNonBlocking();
			Debug::LogInfo(CString("Client stated on ").Add(myAddr.GetStringIp()));
			sendToAddress = SocketAddress("192.168.0.107", 54000);
			sendAddressValid = true;
			mainSocketBound = true;
		}

		bool32 NetworkConnectionEsablished()
		{
			return connectionEsablished;
		}

		struct ConnectionSnapShot
		{
			uint8 type;
		};

		int32 NetworkReceive(void* buf, int32 bufSizeBytes)
		{
			Assert(bufSizeBytes <= 256, "Network buffer size to large !");

			if (mainSocketBound)
			{
				uint8 buffer[256] = {};
				uint32 bufferSize = sizeof(buffer);

				SocketAddress address = {};
				int32 bytes = mainSocket.ReceiveFrom(buffer, bufferSize, &address);

				if (bytes <= 0)
				{
					return bytes;
				}

				ConnectionSnapShot* snap = (ConnectionSnapShot*)buffer;
				if (snap->type == 1)
				{
					if (!sendAddressValid)
					{
						sendToAddress = address;
						sendAddressValid = true;
						connectionEsablished = true;
						Debug::LogInfo(CString("Connected to ").Add(sendToAddress.GetStringIp()));
						NetworkSend(snap, sizeof(ConnectionSnapShot));
					}
					else
					{
						connectionEsablished = true;
						Debug::LogInfo("Connection received");
					}
				}
				else
				{
					memcpy(buf, buffer, bufSizeBytes);
				}

				return bytes;
			}

			return -1;
		}

		void NetworkSend(void* buf, int32 bufsizeBytes)
		{
			if (sendAddressValid)
			{
				if (buf)
				{
					mainSocket.SendTo(buf, bufsizeBytes, sendToAddress);
				}
				else
				{
					ConnectionSnapShot snap = {};
					snap.type = 1;
					mainSocket.SendTo((void*)&snap, sizeof(snap), sendToAddress);
				}
			}
		}

		void ShutdownNetworking()
		{
			WSACleanup();
		}
	}
}