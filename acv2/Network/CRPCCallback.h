#pragma once

#include "../../Shared/Network/CRPC.h"

class CRPCCallback
{
public:
	static void Initialize();

private:
	static void ResendFileInformation();

};