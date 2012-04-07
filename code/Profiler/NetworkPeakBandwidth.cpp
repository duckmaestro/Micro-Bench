/*
 * This file is part of Micro Bench
 * <https://github.com/duckmaestro/Micro-Bench>.
 * 
 * Micro Bench is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Micro Bench is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#include "NetworkPeakBandwidth.h"
#include "Windows.h"
#include <WinSock2.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cassert>

NetworkPeakBandwidth::NetworkPeakBandwidth(StopwatchStack& stopwatch)
	: Experiment(L"Network Peak Bandwidth", stopwatch)
{
}

NetworkPeakBandwidth::NetworkPeakBandwidth(std::wstring name, StopwatchStack& stopwatch)
	: Experiment(name, stopwatch)
{
}

NetworkPeakBandwidth::~NetworkPeakBandwidth(void)
{
}

void NetworkPeakBandwidth::InitWinsock()
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(result != 0)
	{
		throw std::exception("Winsock unavailable.");
	}
}

void NetworkPeakBandwidth::CleanupWinsock()
{
	WSACleanup();
}

void NetworkPeakBandwidth::Run(void)
{
	StopwatchStack& ss = _stopwatch;
	auto* tagLocal = L"Network - Peak Bandwidth Local";
	auto* tagRemote = L"Network - Peak Bandwidth Remote";
	const int chunkSize = 1 * 1024 * 1460;
	const int numIterations = 500;

	// 
	InitWinsock();

	// local
	CreateLocalChildServer(1);
	ConnectAndSend(Experiment::LocalMachineAddress, 80, chunkSize, numIterations, tagLocal);
	{
		TickValue minTransmitTimeAsTicks = Mathx::Median<TickValue, TickValue>(
			ss.GetTagHistory(tagLocal),
			[=](TickValue ticks) { return ticks; }
		);
		minTransmitTimeAsTicks -= ss.GetOverheadSimpleAsTicks();

		double peakBandwidth 
			= (double)chunkSize
			/ ss.TicksToSeconds(minTransmitTimeAsTicks);

		PushResult(L"Local (bytes/second)", peakBandwidth);
	}

	// remote
	ConnectAndSend(Experiment::OtherMachineAddress, 80, chunkSize, numIterations, tagRemote);
	{
		TickValue minTransmitTimeAsTicks = Mathx::Median<TickValue, TickValue>(
			ss.GetTagHistory(tagRemote),
			[=](TickValue ticks) { return ticks; }
		);
		minTransmitTimeAsTicks -= ss.GetOverheadSimpleAsTicks();

		double peakBandwidth 
			= (double)chunkSize
			/ ss.TicksToSeconds(minTransmitTimeAsTicks);

		PushResult(L"Remote (bytes/second)", peakBandwidth);
	}

	// 
	CleanupWinsock();

}

void NetworkPeakBandwidth::ChildProcessMain(int argc, wchar_t* argv[])
{
	//__asm int 3;

	// parse number of times to listen
	int numTimesToListen = -1;

	if(argc >= 3)
	{
		auto arg2 = argv[2];
		std::wstringstream parser;
		parser << arg2;
		parser >> numTimesToListen;
		assert(numTimesToListen >= 0);
		assert(numTimesToListen < 1024 * 1024);
	}

	// switch to cpu 2
	SetThreadAffinityMask(GetCurrentThread(), 1 << 1);

	//
	InitWinsock();

	// create socket
	SOCKET socketListener = INVALID_SOCKET;
	{
		socketListener = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
		if(socketListener == INVALID_SOCKET)
		{
			HRESULT error = WSAGetLastError();
			throw std::exception("Could not create socket.");
		}
	}

	// setup listening
	{
		SOCKADDR_IN address;
		address.sin_family = AF_INET;
		address.sin_addr.S_un.S_addr = INADDR_ANY;
		USHORT netPort;
		WSAHtons(socketListener, 80, &netPort);
		address.sin_port = netPort;
		
		std::wcout << "Binding to local port." << std::endl;
		if(0 != bind(socketListener, reinterpret_cast<sockaddr*>(&address), sizeof(address)))
		{
			HRESULT error = WSAGetLastError();
			throw std::exception("Could not bind to local port.");
		}

		std::wcout << "Listening on port " << address.sin_port << std::endl;
		if(SOCKET_ERROR == listen(socketListener, 1))
		{
			HRESULT error = WSAGetLastError();
			throw std::exception("Could not begin listening.");
		}
	}

	// wait to accept incoming connection...
	for(int i = 0; i < numTimesToListen || numTimesToListen == -1; ++i)
	{
		SOCKET socketConnection;
		{
			std::wcout << "Waiting for incoming connection... " << std::endl;
			socketConnection = WSAAccept(socketListener, NULL, NULL, NULL, NULL);
			if(INVALID_SOCKET == socketConnection)
			{
				HRESULT error = WSAGetLastError();
				std::wcout << "Error: " << error << std::endl;
				throw std::exception("Could not wait for incoming connection.");
			}
		}

		// receive data
		{
			char buffer[128  * 1024];
			DWORD bytesRead = -1;
			WSABUF bufferDesc;
			bufferDesc.buf = buffer;
			bufferDesc.len = sizeof(buffer);
			DWORD flags = MSG_WAITALL;
			bool receivedOnce = false;
			while(0 == WSARecv(socketConnection, &bufferDesc, 1, &bytesRead, &flags, NULL, NULL)
				&& bytesRead != 0)
			{
				if(!receivedOnce)
				{
					std::wcout << "Receiving data..." << std::endl;
					receivedOnce = true;
				}
			}

			std::wcout << "No more data." << std::endl;
		}

		// close connection
		{
			if(0 != shutdown(socketConnection, SD_RECEIVE))
			{
				HRESULT error = WSAGetLastError();
				throw std::exception("Could not end connection.");
			}

			if(0 != closesocket(socketConnection))
			{
				HRESULT error = WSAGetLastError();
				throw std::exception("Could not close socket.");
			}
		}
	}

	// close listener
	{
		if(0 != closesocket(socketListener))
		{
			HRESULT error = WSAGetLastError();
			throw std::exception("Could not close socket.");
		}
	}

	//
	CleanupWinsock();
}

void* NetworkPeakBandwidth::CreateLocalChildServer(int numTimesToListen)
{
	wchar_t path[1024];
	GetModuleFileName(NULL, path, 1024);
	
	STARTUPINFO startupInfo;
	ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
	startupInfo.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION processInfo;
	ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

	std::wstringstream stringBuilder;
	stringBuilder 
		<< L"Profiler.exe"
		<< L" -NetworkBandwidthChildServer"
	;
	if(numTimesToListen >= 0)
	{
		stringBuilder
			<< L" "
			<< numTimesToListen
		;
	}

	BOOL success = CreateProcessW(
		path,
		const_cast<LPWSTR>(stringBuilder.str().c_str()),
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&startupInfo,
		&processInfo
	);

	if(!success)
	{
		throw std::exception("Could not create process.");
	}

	return processInfo.hProcess;
}

void NetworkPeakBandwidth::ConnectAndSend(uint32_t address, uint16_t port, int chunkSize, int numChunks, const void* tag)
{
	StopwatchStack& ss = _stopwatch;

	// create socket
	SOCKET socket = INVALID_SOCKET;
	{
		socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
		if(socket == INVALID_SOCKET)
		{
			HRESULT result = WSAGetLastError();
			throw std::exception("Could not create socket.");
		}

		// http://msdn.microsoft.com/en-us/library/windows/desktop/ms740476(v=vs.85).aspx
		// http://stackoverflow.com/questions/1017507/doubt-regarding-winsock-kernel-buffer-and-nagle-algorithm
		// setsockopt(socket, SOL_SOCKET, SO_SNDBUF, 
	}
	
	// connect to server
	{
		SOCKADDR_IN socketAddress;
		socketAddress.sin_family = AF_INET;
		socketAddress.sin_addr.S_un.S_addr = static_cast<ULONG>(address);
		USHORT netPort;
		WSAHtons(socket, port, &netPort);
		socketAddress.sin_port = netPort;

		if(0 != WSAConnect(socket, reinterpret_cast<sockaddr*>(&socketAddress), sizeof(socketAddress), NULL, NULL, NULL, NULL))
		{
			HRESULT error = WSAGetLastError();
			throw std::exception("Could not connect.");
		}
	}

	// send data
	{
		// prepare data
		uint8_t* dataToSend = static_cast<uint8_t*>(std::malloc(chunkSize));
		for(uint8_t* iter = dataToSend; iter < dataToSend + chunkSize; ++iter)
		{
			*iter = 0x01;
		}
		dataToSend[0] = 0x00;

		WSABUF buffers[1];
		buffers[0].buf = reinterpret_cast<char*>(dataToSend);
		buffers[0].len = chunkSize;

		// send
		for(int r = 0; r < numChunks; ++r)
		{
			DWORD bytesSent = 0;
			bool recordTiming = r >= numChunks / 4 && r <= numChunks * 3 / 4;
			if(recordTiming) // ignore measuring the first and last 25% of sends, in case the OS is buffering bursts.
			{
				ss.Push(tag);
			}
			if(0 != WSASend(socket, buffers, 1, &bytesSent, 0, NULL, NULL))
			{
				HRESULT error = WSAGetLastError();
				throw std::exception("Could not send.");
			}
			if(recordTiming)
			{
				ss.Pop();
			}
		}
	}

	// close connection
	{
		if(0 != shutdown(socket, SD_BOTH))
		{
			HRESULT error = WSAGetLastError();
			throw std::exception("Could not end connection.");
		}
	}

	// close socket
	{
		if(0 != closesocket(socket))
		{
			HRESULT error = WSAGetLastError();
			throw std::exception("Could not close socket.");
		}
	}
}
