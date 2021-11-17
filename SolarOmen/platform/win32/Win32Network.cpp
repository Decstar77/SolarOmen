#include "core/SolarPlatform.h"

#include "Win32Socket.h"


namespace cm
{
	static bool InitializeSockets()
	{
		WSADATA WsaData = {};
		WORD dllVersion = MAKEWORD(2, 2);
		bool result = WSAStartup(dllVersion, &WsaData) == NO_ERROR;

		return result;
	}

	void cm::PlatformNetwork::Initialize()
	{
		return;
		if (InitializeSockets())
		{
			int asServer = 0;
			LOG("Run as server ?");
			std::cin >> asServer;

			if (asServer)
			{
				UDPSocket socket;
				SocketAddress hostAddr = SocketAddress(54000);

				socket.Create();
				socket.Bind(hostAddr);

				LOG(hostAddr.GetStringIp().GetCStr());

				while (true)
				{
					char inputBuffer[1024];
					ZeroArray(inputBuffer);

					SocketAddress client;
					int32 bytesIn = socket.ReceiveFrom(inputBuffer, 1024, client);

					LOG(client.GetStringIp().GetCStr());
				}
				socket.Close();
			}
			else
			{
				SocketAddress serverAddr = SocketAddress("192.168.0.107", 54000);
				UDPSocket socket;
				socket.Create();

				char outBuffer[256] = {};
				socket.SendTo(outBuffer, 256, serverAddr);



				socket.Close();
			}
		}
		else
		{
			Assert(0, "Error initializing sockets");
		}
	}

	void cm::PlatformNetwork::Shutdown()
	{
		WSACleanup();
	}
}