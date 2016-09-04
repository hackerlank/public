#ifndef _protocol_h_
#define _protocol_h_

enum eDefine{
    DEF_MAX_NODES           =1000,
    DEF_MAX_GAMES_PER_NODE  =100000,
};

enum eMsg:unsigned short{
    MSG_BEGIN       =1000,
	MSG_RAW,
	
	//Client---------------- Login
    MSG_CS_BEGIN    =2000,
	MSG_CS_LOGIN,
	MSG_SC_LOGIN,

	MSG_CS_END,
	//Client---------------- Lobby
    MSG_CL_BEGIN    =4000,
	MSG_CL_ENTER,
	MSG_LC_ENTER,
	MSG_LC_EXIT,

    MSG_CL_END,
    //Client---------------- Node
    MSG_CN_BEGIN    =6000,
    MSG_CN_ENTER,
    MSG_NC_ENTER,
    MSG_CN_CREATE,
    MSG_NC_CREATE,
    MSG_CN_JOIN,
    MSG_NC_JOIN,
    MSG_NC_START,
    MSG_CN_DISCARD,
    MSG_NC_DISCARD,
    MSG_CN_MELD,
    MSG_NC_MELD,
    MSG_CN_DISMISS_A,
    MSG_NC_DISMISS_A,
    MSG_CN_DISMISS_B,
    MSG_NC_DISMISS_B,
    MSG_NC_SETTLE,
    MSG_NC_FINISH,
    
    MSG_CN_END,
    MSG_END,
};
#endif // _protocol_h_