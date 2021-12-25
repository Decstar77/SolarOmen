#include "Win32Socket.h"


namespace cm
{
	static int32 GetLastError()
	{
		return WSAGetLastError();
	}

	static void ReportError(const char* err)
	{
		LPSTR lpMsgBuf;
		DWORD errorNum = GetLastError();

		FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			errorNum,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&lpMsgBuf,
			0, NULL);

		MessageBoxA(NULL, (const char*)lpMsgBuf, err, MB_HELP);
	}

	static CString GetLocalIPName(uint16 inPort)
	{
		char hostName[256];
		gethostname(hostName, 256);

		addrinfo hints = {};
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;

		char portStr[16] = {};
		sprintf_s(portStr, "%d", inPort);

		addrinfo* addrs = nullptr;
		getaddrinfo(hostName, portStr, &hints, &addrs);

		CString ip = {};
		for (addrinfo* addr = addrs; addr != NULL; addr = addr->ai_next)
		{
			if (addr->ai_family == AF_INET) {
				sockaddr_in* ipv = (sockaddr_in*)addr->ai_addr;
				void* addr2 = &(ipv->sin_addr);
				char addrName[256];
				inet_ntop(addr->ai_family, addr2, addrName, 256);
				ip = CString(addrName);
			}
		}

		freeaddrinfo(addrs);

		return ip;
	}

	static IN_ADDR GetLocalIP(uint16 inPort)
	{
		char hostName[256] = {};

		gethostname(hostName, 256);

		addrinfo hints = {};
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;

		char portStr[16] = {};
		sprintf_s(portStr, "%d", inPort);

		addrinfo* addrs = nullptr;
		getaddrinfo(hostName, portStr, &hints, &addrs);
		IN_ADDR result = {};
		for (addrinfo* addr = addrs; addr != NULL; addr = addr->ai_next)
		{
			if (addr->ai_family == AF_INET) {
				sockaddr_in* ipv = (sockaddr_in*)addr->ai_addr;
				result = ipv->sin_addr;
			}
		}

		freeaddrinfo(addrs);
		return result;
	}

	SocketAddress::SocketAddress()
	{
		socketAddress = {};
		port = 0;
	}

	SocketAddress::SocketAddress(uint16 inPort)
	{
		socketAddress = {};
		sockaddr_in* in = GetAsSockAddIn();
		in->sin_family = AF_INET;
		in->sin_addr = GetLocalIP(inPort);
		in->sin_port = htons(inPort);

		port = inPort;
		ipAddress = ntohl(in->sin_addr.S_un.S_addr);
	}

	SocketAddress::SocketAddress(uint32 inAddress, uint16 inPort)
	{
		socketAddress = {};
		sockaddr_in* in = GetAsSockAddIn();
		in->sin_family = AF_INET;
		in->sin_addr.S_un.S_addr = htonl(inAddress);
		in->sin_port = htons(inPort);

		port = inPort;
		ipAddress = inAddress;
	}

	SocketAddress::SocketAddress(CString inAddress, uint16 inPort)
	{
		socketAddress = {};
		sockaddr_in* in = GetAsSockAddIn();
		in->sin_family = AF_INET;
		in->sin_port = htons(inPort);
		InetPtonA(AF_INET, inAddress.GetCStr(), &in->sin_addr);

		port = port;
		ipAddress = ntohl(in->sin_addr.S_un.S_addr);
	}

	SocketAddress::SocketAddress(PlatformAddress platformSocket)
	{
		socketAddress = {};
		sockaddr_in* in = GetAsSockAddIn();
		in->sin_family = AF_INET;
		in->sin_addr.S_un.S_addr = htonl(platformSocket.ipAddress);
		in->sin_port = htons(platformSocket.port);

		port = platformSocket.port;
		ipAddress = platformSocket.ipAddress;
	}

	CString SocketAddress::GetStringIp() const
	{
		char buf[256] = {};

		sockaddr_in* in = GetAsSockAddIn();
		inet_ntop(AF_INET, &in->sin_addr, buf, 256);

		return CString(buf);
	}

	PlatformAddress SocketAddress::ToPlatformAddress() const
	{
		PlatformAddress platformSocket = {};
		platformSocket.port = port;
		platformSocket.ipAddress = ipAddress;
		platformSocket.stringIP = GetStringIp();

		return platformSocket;
	}

	void UDPSocket::Create()
	{
		this->socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (socket != INVALID_SOCKET)
		{
			Debug::LogInfo("Socket created");
		}
		else
		{
			ReportError("SocketUtil::CreateUDPSocket");
		}
	}

	bool32 UDPSocket::Bind(const SocketAddress& inBindAddress)
	{
		int32 err = bind(socket, &inBindAddress.socketAddress, inBindAddress.GetSize());
		if (err != NO_ERROR)
		{
			ReportError("UDPSocket::Bind");
			return false;
		}
		else
		{
			Debug::LogInfo("Socket bound");
		}

		return true;
	}

	bool32 UDPSocket::SetNonBlocking()
	{
		DWORD nonBlocking = 1;
		if (ioctlsocket(this->socket, FIONBIO, &nonBlocking) != 0)
		{
			ReportError("UDPSocket::SetNonBlocking");
			return false;
		}

		return true;
	}

	int UDPSocket::SendTo(const void* inData, int inLen, const SocketAddress& inTo)
	{
		int32 byteSentCount = sendto(socket, (const char*)inData, inLen, 0, &inTo.socketAddress, inTo.GetSize());
		if (byteSentCount >= 0)
		{
			return byteSentCount;
		}
		else
		{
			ReportError("UDPSocket::SendTo");
			return 0;
		}
	}

	int UDPSocket::ReceiveFrom(void* inBuffer, int bufferLen, SocketAddress* outFrom)
	{
		int32 fromLength = outFrom->GetSize();
		int32 readByteCount = recvfrom(socket, (char*)inBuffer, bufferLen, 0, &outFrom->socketAddress, &fromLength);

		if (readByteCount > 0)
		{
			sockaddr_in* in = outFrom->GetAsSockAddIn();
			outFrom->port = ntohs(in->sin_port);
			outFrom->ipAddress = ntohl(in->sin_addr.S_un.S_addr);
		}

		return readByteCount;
	}

	void UDPSocket::Close()
	{
		closesocket(socket);
	}

}
