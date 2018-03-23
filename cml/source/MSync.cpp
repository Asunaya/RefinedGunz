#include "stdafx.h"
#include "MSync.h"

#ifdef WIN32

#include <Windows.h>

MCriticalSection::MCriticalSection() { InitializeCriticalSection(&cs()); }
MCriticalSection::~MCriticalSection() { DeleteCriticalSection(&cs()); }
void MCriticalSection::lock() { EnterCriticalSection(&cs()); }
void MCriticalSection::unlock() { LeaveCriticalSection(&cs()); }

CRITICAL_SECTION& MCriticalSection::cs() {
	static_assert(sizeof(buf) == sizeof(CRITICAL_SECTION) &&
		alignof(decltype(buf)) == alignof(CRITICAL_SECTION), "");
	return reinterpret_cast<CRITICAL_SECTION&>(*(char*)buf);
}

static_assert(WAIT_OBJECT_0 == 0, "");

MSignalEvent::MSignalEvent(bool ManualReset) : EventHandle{ CreateEventA(NULL, ManualReset, FALSE, NULL) } {}

MSignalEvent::~MSignalEvent() {
	if (EventHandle) {
		CloseHandle(EventHandle);
		EventHandle = NULL;
	}
}

bool MSignalEvent::SetEvent() {
	return ::SetEvent(EventHandle) != FALSE; }
bool MSignalEvent::ResetEvent() {
	return ::ResetEvent(EventHandle) != FALSE; }

u32 WaitForMultipleEvents(size_t NumEvents, MSignalEvent* const * EventArray, u32 Timeout)
{
	HANDLE Handles[MAXIMUM_WAIT_OBJECTS];
	for (size_t i = 0; i < NumEvents; ++i)
		Handles[i] = EventArray[i]->GetEventHandle();

	auto ret = WaitForMultipleObjects(NumEvents, Handles, FALSE, Timeout);

	switch (ret)
	{
	case WAIT_FAILED:
		return MSync::WaitFailed;
	case WAIT_TIMEOUT:
		return MSync::WaitTimeout;
	default:
		return ret;
	}
}

#else

#include <sys/select.h>
#include <sys/eventfd.h>

MSignalEvent::MSignalEvent(bool ManualReset) : EventHandle{ eventfd(0, 0) } {}
MSignalEvent::~MSignalEvent() { close(EventHandle); }

bool MSignalEvent::SetEvent()
{
	u64 buf = 1;
	write(EventHandle, &buf, sizeof(buf));
}

bool MSignalEvent::ResetEvent()
{
	u64 buf;
	read(EventHandle, &buf, sizeof(buf));
}

u32 WaitForMultipleEvents(size_t NumEvents, MSignalEvent* const * EventArray, u32 Timeout)
{
	fd_set fds;
	FD_ZERO(&fds);

	int max_fd = INT_MIN;
	for (int i = 0; i < int(NumEvents); ++i) {
		const auto fd = EventArray[i]->GetEventHandle();
		FD_SET(fd, &fds);
		if (fd > max_fd)
			max_fd = fd;
	}

	if (max_fd == INT_MIN)
		return MSync::WaitFailed;

	timeval tv;
	const bool BlockIndefinitely = Timeout == MSync::Infinite;
	if (!BlockIndefinitely)
	{
		// Set the seconds from the Timeout value.
		tv.tv_sec = Timeout / 1000;
		// Set the microseconds.
		tv.tv_usec = (Timeout % 1000) * 1000;
	}

	auto ret = select(max_fd, &fds, nullptr, nullptr, BlockIndefinitely ? nullptr : &tv);
	if (ret == 0)
		return MSync::WaitTimeout;
	if (ret < 0)
		return MSync::WaitFailed;

	for (int i = 0; i < int(NumEvents); ++i) {
		if (FD_ISSET(EventArray[i]->GetEventHandle(), &fds)) {
			return static_cast<u32>(i);
		}
	}

	return MSync::WaitFailed;
}

#endif

u32 MSignalEvent::Await(u32 Timeout) {
	auto this_lvalue = this;
	return WaitForMultipleEvents(1, &this_lvalue, Timeout);
}