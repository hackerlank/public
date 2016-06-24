#ifndef _protocol_h_
#define _protocol_h_

#ifdef _WIN32
#include <SDKDDKVer.h>
#endif

enum eMsg:unsigned short{
	MSG_RAW,
	MSG_GAME,
	MSG_LOGIN,
	MSG_LOGINREP,
};

struct MsgRaw{
	size_t			mid;
	char			message[32];
};

struct MsgGame{
	unsigned short	len;
	eMsg			mid;
};

struct MsgLogin:public MsgGame{
	char			name[32];
};

struct MsgLoginRep:public MsgGame{
	unsigned char	result;
};


#endif // _protocol_h_