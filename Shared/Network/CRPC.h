#pragma once

#include <map>
#include "../RakNet/BitStream.h"

#define RPC_CALLBACK void
#define RPC_ARGS RakNet::BitStream& bsData, int iExtra

typedef RPC_CALLBACK(*RPCFunct_t)(RPC_ARGS);

enum eRPC : unsigned short
{
	// -- CLIENT RPCs --

	// Called when the server requests a region of memory checksum to be calculated.
	MD5_MEMORY_REGION,
	
	// Called when the server requests a file to be md5'd 
	MD5_FILE,

	// Called when the server wants to turn off switch reload for a player.
	TOGGLE_SWITCH_RELOAD,

	// -- SERVER RPCs --

	// Called when a new module (in the current process) or process has been run
	ON_FILE_EXECUTED,
	
	// Called when an md5 checksum on a region of memory has been calculated
	ON_MD5_CALCULATED,

	// Called when a file's md5 has been calculated, most likely in the GTA directory.
	ON_FILE_CALCULATED,

	// Called when a file inside gta3.img's md5 has been calculated.
	ON_IMG_FILE_MODIFIED,

	// Called when a macro is detected on the client machine.
	ON_MACRO_DETECTED,

	// Called when the player has sent the server their hardware ID.
	ON_HARDWAREID_SENT,


};

class CRPC
{
public:
	static void Add(unsigned short usRPCId, RPCFunct_t func);
	static void Process(unsigned short usRPCId, RakNet::BitStream& bsData, int iExtra = NULL);

private:
	static std::map<unsigned short, RPCFunct_t> m_functions;

};
