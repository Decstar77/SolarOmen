#pragma once

#include "SolarPlatform.h"

namespace cm
{
	namespace Network
	{
		namespace
		{
			static uint32 sequence = 1;
			static uint32 remoteSequence = 1;

			struct ReliablePacket
			{
				real32 time;
				uint32 seq;
				char data[256];
			};

			static FixedArray<ReliablePacket, 100> reliablePackets;
		}

		inline void Update(real32 dt)
		{

		}

		inline void SendReliable(void* buffer, int32 bufferSizeBytes)
		{
			ReliablePacket packet = {};
			packet.seq = sequence++;
			//packet
		}

		template<typename T>
		inline void SendReliable(T* data)
		{

		}

		template<typename T>
		inline void SendUnreliable(T* data)
		{

		}
	}
}
