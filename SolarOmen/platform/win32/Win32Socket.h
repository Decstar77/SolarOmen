#pragma once
#include "core/SolarCore.h"

#include <winsock2.h>
#include <ws2tcpip.h>

namespace cm
{
	class SocketAddress
	{
	public:
		SocketAddress();
		SocketAddress(uint16 inPort);
		SocketAddress(uint32 inAddress, uint16 inPort);
		SocketAddress(CString inAddress, uint16 inPort);
		SocketAddress(PlatformAddress platformSocket);

		uint32 GetIp();
		uint32 GetPort();
		CString GetStringIp() const;

		PlatformAddress ToPlatformAddress() const;

		inline uint32 GetSize() const { return sizeof(sockaddr); }

	private:
		friend class UDPSocket;
		inline sockaddr_in* GetAsSockAddIn() const { return (sockaddr_in*)&socketAddress; }
		sockaddr socketAddress;

		// @NOTE: Just caching these for easy of use
		uint16 port;
		uint32 ipAddress;
	};


	class UDPSocket
	{
	public:
		UDPSocket() { socket = {}; }


		void Create();
		bool32 Bind(const SocketAddress& inBindAddress);
		bool32 SetNonBlocking();
		int SendTo(const void* inData, int inLen, const SocketAddress& inTo);
		int ReceiveFrom(void* inBuffer, int inLen, SocketAddress* outFrom);
		void Close();

	private:

		UDPSocket(SOCKET inSocket) : socket(inSocket) {}
		SOCKET socket;
	};

}
