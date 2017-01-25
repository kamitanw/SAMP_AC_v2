#include "ACVerifiedPacket.h"
#include "../MD5_Info/Cmd5Info.h"
#include "../../ACv2_Server/md5.h"

#include <stdlib.h>

namespace ACVerifiedPacket
{
	std::string RawVerifiedPacket()
	{
		/* 
			Request something to MD5. That thing changes and is never the same which
			is supposed to confuse crackers for some time.

			Plus the URL has a misleading name.
		*/
		std::string response_data = Cmd5Info::GetWebsiteText("https://apples.sampcup.com/cain.php");
		std::string number = Cmd5Info::GetWebsiteText("https://apples.sampcup.com/abel.php");

		if (atoi(number.c_str()) > 0)
		{
			// If that random/unknown number is positive, append static data
			response_data.append("9f3973ed275e6c49b3f13a1c0968957f");
		}
		else
		{
			// If that random/unknown number is negative, append different static data
			response_data.append("86ec47700f49aa06e717505af301f14d");
		}

		// Then append the random number itself
		response_data.append(number);

		// MD5 everything
		MD5 md5 = MD5();
		std::string md5ed_data = std::string(md5.digestString((char*)response_data.c_str()));

		// Return hashed data
		return md5ed_data;
	}
}
