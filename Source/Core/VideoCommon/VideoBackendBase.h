// Copyright 2011 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <string>
#include <vector>

#include "Common/ChunkFile.h"
#include "VideoCommon/PerfQueryBase.h"

namespace MMIO { class Mapping; }


enum FieldType
{
	FIELD_ODD = 0,
	FIELD_EVEN = 1,
};

enum EFBAccessType
{
	PEEK_Z = 0,
	POKE_Z,
	PEEK_COLOR,
	POKE_COLOR
};

struct SCPFifoStruct
{
	// fifo registers
	volatile u32 CPBase;
	volatile u32 CPEnd;
	u32 CPHiWatermark;
	u32 CPLoWatermark;
	volatile u32 CPReadWriteDistance;
	volatile u32 CPWritePointer;
	volatile u32 CPReadPointer;
	volatile u32 CPBreakpoint;
	volatile u32 SafeCPReadPointer;
	// Super Monkey Ball Adventure require this.
	// Because the read&check-PEToken-loop stays in its JITed block I suppose.
	// So no possiblity to ack the Token irq by the scheduler until some sort of PPC watchdog do its mess.
	volatile u16 PEToken;

	volatile u32 bFF_GPLinkEnable;
	volatile u32 bFF_GPReadEnable;
	volatile u32 bFF_BPEnable;
	volatile u32 bFF_BPInt;
	volatile u32 bFF_Breakpoint;

	volatile u32 bFF_LoWatermarkInt;
	volatile u32 bFF_HiWatermarkInt;

	volatile u32 bFF_LoWatermark;
	volatile u32 bFF_HiWatermark;
};

class VideoBackendBase
{
public:
	virtual ~VideoBackendBase() {}

	void EmuStateChange(EMUSTATE_CHANGE);

	virtual unsigned int PeekMessages() = 0;

	virtual bool Initialize(void* window_handle) = 0;
	virtual void Shutdown() = 0;
	void RunLoop(bool enable);

	virtual std::string GetName() const = 0;
	virtual std::string GetDisplayName() const { return GetName(); }

	virtual void ShowConfig(void*) = 0;

	virtual void Video_Prepare() = 0;
	void Video_EnterLoop();
	void Video_ExitLoop();
	virtual void Video_Cleanup() = 0; // called from gl/d3d thread

	void Video_BeginField(u32, u32, u32, u32);
	void Video_EndField();

	u32 Video_AccessEFB(EFBAccessType, u32, u32, u32);
	u32 Video_GetQueryResult(PerfQueryType type);
	u16 Video_GetBoundingBox(int index);

	void Video_AddMessage(const std::string& msg, unsigned int milliseconds);
	void Video_ClearMessages();
	bool Video_Screenshot(const std::string& filename);

	void Video_SetRendering(bool bEnabled);

	void Video_GatherPipeBursted();

	int Video_Sync(int ticks);

	// Registers MMIO handlers for the CommandProcessor registers.
	void RegisterCPMMIO(MMIO::Mapping* mmio, u32 base);

	static void PopulateList();
	static void ClearList();
	static void ActivateBackend(const std::string& name);

	// waits until is paused and fully idle, and acquires a lock on that state.
	// or, if doLock is false, releases a lock on that state and optionally unpauses.
	// calls must be balanced and non-recursive (once with doLock true, then once with doLock false).
	void PauseAndLock(bool doLock, bool unpauseOnUnlock = true);

	// the implementation needs not do synchronization logic, because calls to it are surrounded by PauseAndLock now
	void DoState(PointerWrap &p);

	void CheckInvalidState();

	void UpdateWantDeterminism(bool want);

protected:
	void InitializeShared();

	bool m_initialized = false;
	bool m_invalid = false;
};

extern std::vector<VideoBackendBase*> g_available_video_backends;
extern VideoBackendBase* g_video_backend;
