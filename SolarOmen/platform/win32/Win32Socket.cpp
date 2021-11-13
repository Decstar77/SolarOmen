#include "Win32Socket.h"


namespace cm
{
	static int32 GetLastError()
	{
		return WSAGetLastError();
	}

	static void ReportError(const char* err)
	{
		LPVOID lpMsgBuf;
		DWORD errorNum = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			errorNum,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		MessageBoxA(NULL, (const char*)lpMsgBuf, err, MB_HELP);
	}

	SocketAddress::SocketAddress()
	{
		socketAddress = {};
	}

	SocketAddress::SocketAddress(uint16 inPort)
	{
		socketAddress = {};
		sockaddr_in* in = GetAsSockAddIn();
		in->sin_family = AF_INET;
		in->sin_addr.S_un.S_addr = ADDR_ANY;
		in->sin_port = htons(inPort);
	}

	SocketAddress::SocketAddress(uint32 inAddress, uint16 inPort)
	{
		socketAddress = {};
		sockaddr_in* in = GetAsSockAddIn();
		in->sin_family = AF_INET;
		in->sin_addr.S_un.S_addr = htonl(inAddress);
		in->sin_port = htons(inPort);
	}

	SocketAddress::SocketAddress(CString inAddress, uint16 inPort)
	{
		socketAddress = {};
		sockaddr_in* in = GetAsSockAddIn();
		in->sin_family = AF_INET;
		in->sin_port = htons(inPort);
		InetPtonA(AF_INET, inAddress.GetCStr(), &in->sin_addr);
	}

	CString SocketAddress::GetStringIp()
	{
		char buf[256] = {};

		sockaddr_in* in = GetAsSockAddIn();
		inet_ntop(AF_INET, &in->sin_addr, buf, 256);

		return CString(buf);
	}

	void UDPSocket::Create()
	{
		this->socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (socket != INVALID_SOCKET)
		{

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

	int UDPSocket::ReceiveFrom(void* inBuffer, int bufferLen, SocketAddress& outFrom)
	{
		int32 fromLength = outFrom.GetSize();
		int32 readByteCount = recvfrom(socket, (char*)inBuffer, bufferLen, 0, &outFrom.socketAddress, &fromLength);
		if (readByteCount >= 0)
		{
			return readByteCount;
		}
		else
		{
			ReportError("UDPSocket::ReceiveFrom");
			return 0;
		}
	}

	void UDPSocket::Close()
	{
		closesocket(socket);
	}

}
