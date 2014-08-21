#include "CRPCCallback.h"
#include "../CLog.h"
#include "../CLoader.h"
#include "../Misc.h"
#include "Network.h"
#include "../Misc.h"
#include "../VMProtectSDK.h"
#include "../Addresses.h"
#include "../CClientUpdater.h"
#include "../CHookManager.h"
#include "../CMem.h"

#include <Boost\thread.hpp>


void CRPCCallback::Initialize()
{
	CRPC::Add(MD5_MEMORY_REGION, MD5_Memory_Region);
	CRPC::Add(MD5_FILE, MD5_File);
	CRPC::Add(TOGGLE_SWITCH_RELOAD, ToggleSwitchReload);
	CRPC::Add(SET_FRAME_LIMIT, SetFrameLimit);
	CRPC::Add(EXIT_PROCESS, ExitThisProcess);
	CRPC::Add(VERSION_NOT_COMPATIBLE, VersionNotCompatible);
	CRPC::Add(TOGGLE_CROUCH_BUG, ToggleCrouchBug);
	CRPC::Add(TOGGLE_LITE_FOOT, ToggleLiteFoot);
	CRPC::Add(TOGGLE_UNLIMITED_SPRINT, ToggleUnlimitedSprint);

	CHookManager::SetConnectPatches();
	boost::thread ResendFilesThread(&ResendFileInformation);
}

void CRPCCallback::ResendFileInformation()
{
	// Send the server our hardware ID incase they wanna ban us.
	RakNet::BitStream bsData;
	
	// Get the number of required bytes in the hardwareID.
	INT nSize = VMProtectGetCurrentHWID(NULL, 0);

	// Allocate a buffer.
	char *pBuf = new char[nSize];

	// Get the hardware ID.
	VMProtectGetCurrentHWID(pBuf, nSize);

	// Write the hardwareID to the packet
	bsData.Write(pBuf);

	// Write the user's AC version to the packet.
	bsData.Write(CURRENT_MAJOR_VERSION);

	// Send the info to the server.
	Network::SendRPC(ON_INITIAL_INFO, &bsData);
	
	// Free memory.
	delete[] pBuf; 

	// Send the server the processes and modules that were loaded while we weren't connected.
	CLoader::Processes.ResendFiles();
	CLoader::Modules.ResendFiles();
	CLoader::GtaDirectory.Scan(Misc::GetGTADirectory());
}

void CRPCCallback::MD5_Memory_Region(RakNet::BitStream &bsData, int iExtra)
{
	// Create variables to store the info the server sent.
	int address, size;

	if (bsData.Read(address) && bsData.Read(size))
	{
		// Calculate the result of the MD5 hash at the address and put it in an std::string
		std::string md5 = Misc::MD5_Memory(address, size);

		// Send the result of the hash back to the server.
		RakNet::BitStream bitStream;
		bitStream.Write(address);
		bitStream.Write(size);
		bitStream.Write(md5.c_str());

		// Send the RPC to the server.
		Network::SendRPC(ON_MD5_CALCULATED, &bitStream);
	}
}

void CRPCCallback::MD5_File(RakNet::BitStream &bsData, int iExtra)
{
	// Create a new variable to hold the file path sent to us by the server.
	unsigned char file[MAX_PATH+1];

	// Read data sent to us by the server (which in this case is the file name)
	if (bsData.Read(file))
	{
		// MD5 that file and store the result.
		std::string result = CLoader::GtaDirectory.MD5_Specific_File((char*)file);

		// Convert the file into a std::string, so we can remove any macros that were sent in the file name.
		std::string szFile(reinterpret_cast<char*>(file));
		
		// Find the occurand of $(GtaDirectory) macro.
		int i = szFile.find("$(GtaDirectory)/");

		RakNet::BitStream bsData;
		
		// Cut out the $(GtaDirectory) macro when we send it back to the server.
		bsData.Write(szFile.substr(i+16).c_str());

		// and of course, send the MD5 as a const char*
		bsData.Write(result.c_str());

		// Call the RPC.
		Network::SendRPC(ON_FILE_CALCULATED, &bsData);
	}
}

void CRPCCallback::ToggleSwitchReload(RakNet::BitStream &bsData, int iExtra)
{
	// Create a variable to hold what the server wants to do
	bool toggle;
	
	// Read the data sent to us by the server.
	if (bsData.Read(toggle))
	{
		// Unprotect the switch reload address.
		DWORD dwOldProt;
		VirtualProtect(SWITCH_RELOAD, 6, PAGE_EXECUTE_READWRITE, &dwOldProt);

		// Set the player's ability to switch reload.
		if (!toggle)
		{
			memcpy(SWITCH_RELOAD, "\x90\x90\x90\x90\x90\x90", 6); // nop
		}
		else
		{
			memcpy(SWITCH_RELOAD, "\x89\x88\xA8\x05\x00\x00", 6); // mov [eax+000005A8],ecx
		}
	}
}

void CRPCCallback::SetFrameLimit(RakNet::BitStream &bsData, int iExtra)
{
	// Create a new variable to hold the new frame limit.
	int fpslimit;

	// Read the value sent to us by the server.
	if (bsData.Read(fpslimit))
	{
		// Make sure the frame limiter is disabled in the game.
		CHookManager::SetFrameLimiterPatch();

		// Set the new frame limit.
		Misc::SetFPSLimit(fpslimit);
	}
}

void CRPCCallback::ToggleCrouchBug(RakNet::BitStream &bsData, int iExtra)
{
	// Create a new variable to hold the value the server sent.
	int toggle;

	// Read the value the server sent us.
	if (bsData.Read(toggle))
	{
		// Set the crouch bug status to the one sent to us by the server.
		Misc::SetCrouchBug(toggle);
	}
}

void CRPCCallback::ExitThisProcess(RakNet::BitStream &bsData, int iExtra)
{
	// Disconnect from the network, and exit the process.
	Network::Disconnect();
	ExitProcess(0);
}

void CRPCCallback::VersionNotCompatible(RakNet::BitStream &bsData, int iExtra)
{
	// Disconnect from the server.
	Network::Disconnect();
}

void CRPCCallback::ToggleLiteFoot(RakNet::BitStream &bsData, int iExtra)
{
	// Create new variable to hold the value the server sent us.
	bool toggle;

	if (bsData.Read(toggle))
	{
		CHookManager::SetLiteFoot(toggle);
	}
}

void CRPCCallback::ToggleUnlimitedSprint(RakNet::BitStream &bsData, int iExtra)
{
	// Create a new variable to hold the server data.
	bool toggle;

	// Read the data
	if (bsData.Read(toggle))
	{
		// Set infinite run to what the server wants.
		CMem::PutSingle<BYTE>(0xB7CEE4, toggle);
	}
}