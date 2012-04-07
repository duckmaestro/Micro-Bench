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


#include "NetworkRoundTripTime.h"
#include "Mathx.h"
#include "Windows.h"
#include <IPHlpApi.h>
#include <IcmpAPI.h>
#include <cassert>

NetworkRoundTripTime::NetworkRoundTripTime(StopwatchStack& stopwatch)
	: Experiment(L"Network Round Trip Time", stopwatch)
	, _icmpHandle(0)
	, _icmpResponseBuffer(0)
	, _icmpRequestBuffer("Hello, network.")
{
	_icmpHandle = IcmpCreateFile();
	if(_icmpHandle == INVALID_HANDLE_VALUE)
	{
		throw std::exception("could not initialize icmp helper.");
	}	
}

NetworkRoundTripTime::~NetworkRoundTripTime(void)
{
	IcmpCloseHandle(_icmpHandle);

	if(_icmpResponseBuffer != 0)
	{
		std::free(_icmpResponseBuffer);
		_icmpResponseBuffer = 0;
	}
}

void NetworkRoundTripTime::Run(void)
{
	// http://www.microsoft.com/technet/support/ee/transform.aspx?prodname=windows%20operating%20system&prodver=5.2&evtid=4226&evtsrc=tcpip&lcid=1033
	// http://msdn.microsoft.com/en-us/library/aa366051(VS.85).aspx
	// http://msdn.microsoft.com/en-us/library/aa366053(v=vs.85).aspx

	StopwatchStack& ss = _stopwatch;
	const int numIterations = 500;
	_icmpResponseBufferSize = sizeof(ICMP_ECHO_REPLY) * 2 + (_icmpRequestBuffer.size() + 1) + 8;
	_icmpResponseBuffer = std::malloc(_icmpResponseBufferSize);


	// local
	{
		auto tagLocalTimed = L"RoundTripTime Local -- Timed";
		auto tagLocalReported = L"RoundTripTime Local -- Reported";

		ULONG destination = LocalMachineAddress;

		for(int i = 0; i < numIterations; ++i)
		{
			PerformPing(tagLocalTimed, tagLocalReported, destination);
			Sleep(100); // give the server some time to be ready again
		}

		PushResultMinimumAsMicroseconds(
			L"Local Ping Timed (μs)",
			ss.GetTagHistory(tagLocalTimed),
			-ss.GetOverheadSimpleAsTicks()
		);

		PushResultMinimumAsMicroseconds(
			L"Local Ping Reported (μs)",
			ss.GetTagHistory(tagLocalReported),
			0
		);
	}

	// remote
	{
		auto tagRemoteTimed = L"RoundTripTime Remote -- Timed";
		auto tagRemoteReported = L"RoundTripTime Remote -- Reported";

		ULONG destination = OtherMachineAddress;
		
		for(int i = 0; i < numIterations; ++i)
		{
			PerformPing(tagRemoteTimed, tagRemoteReported, destination);
			Sleep(100); // give the server some time to be ready again
		}

		PushResultMinimumAsMicroseconds(
			L"Remote Ping Timed (μs)",
			ss.GetTagHistory(tagRemoteTimed),
			-ss.GetOverheadSimpleAsTicks()
		);

		PushResultMinimumAsMicroseconds(
			L"Remote Ping Reported (μs)",
			ss.GetTagHistory(tagRemoteReported),
			0
		);
	}

	std::free(_icmpResponseBuffer);
	_icmpResponseBuffer = 0;
}

void NetworkRoundTripTime::PerformPing(const wchar_t* tagForTimed, const wchar_t* tagForReported, uint32_t destinationAddress)
{
	assert(_icmpResponseBuffer != NULL);
	StopwatchStack& ss = _stopwatch;
	ZeroMemory(_icmpResponseBuffer, _icmpResponseBufferSize);

	ss.Push(tagForTimed);

	DWORD replyCount = 
		IcmpSendEcho2(
			_icmpHandle,
			NULL, //conditionResponseReceived,
			NULL,
			NULL,
			destinationAddress,
			const_cast<char*>(_icmpRequestBuffer.c_str()), // hacky
			(_icmpRequestBuffer.size() + 1),
			NULL,
			_icmpResponseBuffer,
			_icmpResponseBufferSize,
			1000
		);

	ss.Pop();

	if(replyCount == 0)
	{
		HRESULT error = HRESULT_FROM_WIN32(GetLastError());
		throw new std::exception("Send Echo failed.");
	}

	ICMP_ECHO_REPLY* reply = static_cast<ICMP_ECHO_REPLY*>(_icmpResponseBuffer);
	if(reply->Status != IP_SUCCESS)
	{
		throw new std::exception("Echo status not success.");
	}
	ss.PushWithValue(tagForReported, 0);
	ss.PopWithValue(tagForReported, ss.MillisecondsToTicks(reply->RoundTripTime));

}