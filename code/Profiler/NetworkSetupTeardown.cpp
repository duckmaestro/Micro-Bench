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


#include "NetworkSetupTeardown.h"
#include "Windows.h"
#include <WinSock2.h>
#include <cassert>

NetworkSetupTeardown::NetworkSetupTeardown(StopwatchStack& stopwatch)
	: NetworkPeakBandwidth(L"Network Setup & Teardown", stopwatch)
{

}

NetworkSetupTeardown::~NetworkSetupTeardown(void)
{

}

void NetworkSetupTeardown::Run(void)
{
	StopwatchStack& ss = _stopwatch;
	auto* tagLocalSetup = L"Network - Setup Local";
	auto* tagLocalTeardown = L"Network - Teardown Local";
	auto* tagRemoteSetup = L"Network - Setup Remote";
	auto* tagRemoteTeardown = L"Network - Teardown Remote";
	const int numIterations = 500;

	// 
	InitWinsock();

	// local
	CreateLocalChildServer(numIterations);
	for(int i = 0; i < numIterations; ++i)
	{
		ConnectAndDisconnect(Experiment::LocalMachineAddress, 80, tagLocalSetup, tagLocalTeardown);
		Sleep(100); // give the server some time to be ready again
	}
	PushResultMedianAsMicroseconds(
		L"Local Setup (μs)",
		ss.GetTagHistory(tagLocalSetup),
		-ss.GetOverheadSimpleAsTicks()
	);
	PushResultMedianAsMicroseconds(
		L"Local Teardown (μs)",
		ss.GetTagHistory(tagLocalTeardown),
		-ss.GetOverheadSimpleAsTicks()
	);

	// remote
	for(int i = 0; i < numIterations; ++i)
	{
		ConnectAndDisconnect(Experiment::OtherMachineAddress, 80, tagRemoteSetup, tagRemoteTeardown);
		Sleep(100); // give the server some time to be ready again
	}
	PushResultMedianAsMicroseconds(
		L"Remote Setup (μs)",
		ss.GetTagHistory(tagRemoteSetup),
		-ss.GetOverheadSimpleAsTicks()
	);
	PushResultMedianAsMicroseconds(
		L"Remote Teardown (μs)",
		ss.GetTagHistory(tagRemoteTeardown),
		-ss.GetOverheadSimpleAsTicks()
	);

	// 
	CleanupWinsock();


}

void NetworkSetupTeardown::ConnectAndDisconnect(uint32_t address, uint16_t port, const void* tagSetup, const void* tagTeardown)
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

		//setsockopt(
	}

	// prep necessary structure for graceful shutdown
	char buffer[1024];
	WSABUF bufferDesc;
	bufferDesc.buf = buffer;
	bufferDesc.len = sizeof(buffer);
	DWORD bytesReceived;
	DWORD flags = MSG_WAITALL;

	
	// connect to server
	{
		SOCKADDR_IN socketAddress;
		socketAddress.sin_family = AF_INET;
		socketAddress.sin_addr.S_un.S_addr = static_cast<ULONG>(address);
		USHORT netPort;
		WSAHtons(socket, port, &netPort);
		socketAddress.sin_port = netPort;

		ss.Push(tagSetup);
		auto connectResult = WSAConnect(socket, reinterpret_cast<sockaddr*>(&socketAddress), sizeof(socketAddress), NULL, NULL, NULL, NULL);
		ss.Pop();
		if(0 != connectResult)
		{
			HRESULT error = WSAGetLastError();
			throw std::exception("Could not connect.");
		}
	}

	// close connection
	{
		ss.Push(tagTeardown);

		// shutdown this direction
		auto shutdownResult = shutdown(socket, SD_SEND);
		// wait for graceful shutdown
		auto recvResult = WSARecv(socket, &bufferDesc, 1, &bytesReceived, &flags, NULL, NULL);

		// 
		ss.Pop();

		// now check error codes after the fact
		if(shutdownResult != shutdown(socket, SD_SEND))
		{
			HRESULT error = WSAGetLastError();
			throw std::exception("Could not end connection.");
		}

		if(recvResult != 0 || bytesReceived != 0)
		{
			HRESULT error = WSAGetLastError();
			throw std::exception("Unexpected receive result.");
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
