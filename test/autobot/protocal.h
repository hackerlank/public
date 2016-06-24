#ifndef _protocal_h_
#define _protocal_h_

struct packet_crc_t{
	unsigned short	length,
					crc;
	unsigned char	data[0];
};

enum Opcodes
{
	SMSG_RETURN                                 = 0x000,
	CMSG_AUTH_SESSION							= 0x001,
	SMSG_AUTH_RESPONSE							= 0x002,
	CMSG_CHAR_ENUM								= 0x003,
	SMSG_CHAR_ENUM								= 0x004,
	CMSG_PLAYER_ENTERGAME						= 0x005,// enter map also use it // change don't use it when enter game
	SMSG_LOGIN_VERIFY_WORLD						= 0x006,
	CMSG_PLAYER_MOVE							= 0x007,
	SMSG_PLAYER_MOVE							= 0x008,
	CMSG_PING									= 0x009,
	SMSG_UPDATE_OBJECT							= 0x00a,
	SMSG_COMPRESSED_UPDATE_OBJECT				= 0x00b,
	SMSG_COMBAT_START							= 0x00c,	//战斗开始
	CMSG_COMBAT_PLAYER_ACTION					= 0x00d,	//战斗动作数据
	SMSG_COMBAT_ROUND_DATA						= 0x00e,	//战斗动作伤害结果
	SMSG_COMBAT_END								= 0x00f,	//战斗结束数据

	CMSG_MINICHINESECHESS_JOIN					= 0x010,
	SMSG_MINICHINESECHESS_JOIN					= 0x011,
	CMSG_MINICHINESECHESS_EXIT					= 0x012,
	SMSG_MINICHINESECHESS_EXIT					= 0x013,
	CMSG_MINICHINESECHESS_MOVE					= 0x014,
	SMSG_MINICHINESECHESS_MOVE					= 0x015,
	CMSG_MINICHINESECHESS_EAT					= 0x016,
	SMSG_MINICHINESECHESS_EAT					= 0x017,
	SMSG_MINICHINESECHESS_START					= 0x018,
	CMSG_MINICHINESECHESS_WIN					= 0x019,
	SMSG_MINICHINESECHESS_WIN					= 0x01a,
	CMSG_PLAYER_LEAVE_SCENE						= 0x01b,
	SMSG_DESTROY_OBJECT							= 0x01c,
	SMSG_CREATURE_MOVEASTAR						= 0x01d,
	CMSG_OBJECT_CHANGE_ORIE						= 0x01e,
	SMSG_OBJECT_CHANGE_ORIE						= 0x01f,

	SMSG_MINICHINESECHESS_MAP_START				= 0x020,
	SMSG_MINICHINESECHESS_MAP_STOP				= 0x021,
	SMSG_LOGOUT_PLAYER							= 0x022,
	CMSG_PLAYER_RECONNECT						= 0x023,
	SMSG_PLAYER_RECONNECT						= 0x024,
	CMSG_GOSSIP_HELLO							= 0x025,
	CMSG_GOSSIP_SELECT_OPTION					= 0x026,
	CMSG_NPC_TEXT_QUERY							= 0x027,

	CMSG_QUESTGIVER_STATUS_QUERY				= 0x028,
	CMSG_QUESTGIVER_ACCEPT_QUEST				= 0x029,
	CMSG_QUESTGIVER_COMPLETE_QUEST				= 0x02a,
	CMSG_QUESTGIVER_REQUEST_REWARD				= 0x02b,
	CMSG_QUESTGIVER_CHOOSE_REWARD				= 0x02c,
	CMSG_QUESTGIVER_CANCEL						= 0x02d,
	CMSG_QUESTLOG_SWAP_QUEST					= 0x02e,
	CMSG_QUESTLOG_REMOVE_QUEST					= 0x02f,

	SMSG_MESSAGECHAT							= 0x030,
	SMSG_UPDATE_SUGGEST_QUEST_LIST				= 0x031,
	SMSG_QUEST_DETAILS							= 0x032,
	SMSG_PLAYER_CHANGE_SCENE					= 0x033,
	SMSG_QUESTGIVER_ACCEPT_QUEST				= 0x034,
	SMSG_QUESTUPDATE_ADD_KILLORTALK				= 0x035,
	SMSG_QUESTGIVER_OFFER_REWARD				= 0x036,
	SMSG_QUESTUPDATE_ADD_ITEM					= 0x037,
	SMSG_ITEM_PUSH_RESULT						= 0x038,
	CMSG_DESTROY_ITEM							= 0x039,
	SMSG_DESTROY_ITEM							= 0x03A,
	CMSG_SWAP_INV_ITEM							= 0x03B,
	SMSG_INVENTORY_CHANGE_FAILURE				= 0x03C,
	CMSG_INV_USE_ITEM							= 0x03D,
	CMSG_EQUIP_UNLOAD							= 0x03E,
	SMSG_BAG_ERROR								= 0x03F,

	CMSG_GAMEOBJECT_USE							= 0x040,
	CMSG_CAST_SPELL								= 0x041,
	SMSG_CAST_FAILED							= 0x042,
	SMSG_CLEAR_COOLDOWN							= 0x043,
	SMSG_SPELL_PREPARE							= 0x044,
	SMSG_SPELL_GO								= 0x045,
	SMSG_SPELLNONMELEEDAMAGELOG					= 0x046,
	SMSG_LOOT_RESPONSE							= 0x047,
	CMSG_LOOT_MASTER_GIVE						= 0x048,
	CMSG_AUTOSTORE_LOOT_ITEM					= 0x049,
	SMSG_LOOT_REMOVED							= 0x04A,
	CMSG_AUTOSTORE_LOOT_ALL						= 0x04B,
	CMSG_TELEPORT								= 0x04C,
	SMSG_AUTOSTORE_LOOT_ITEM_RESPON				= 0x04D,
	CMSG_REPOP_REQUEST							= 0x04E,
	SMSG_COMBAT_NEXT_ROUND						= 0x04F,

	CMSG_COMBAT_VERIFY							= 0x050,
	SMSG_QUESTLOG_REMOVE_QUEST					= 0x051,
	SMSG_COMBAT_END_REWARD_MSG					= 0x052,
	CMSG_PET_QUERY_STABLE						= 0x053,
	SMSG_PET_QUERY_STABLE						= 0x054,
	SMSG_PET_NOTIFY_REFRESH						= 0x055,
	CMSG_PET_INACTIVE							= 0x056,
	CMSG_PET_ACTIVE								= 0x057,
	CMSG_PET_DELETE								= 0X058,
	CMSG_QUERY_NPCSTORELIST						= 0x059,
	SMSG_QUERY_NPCSTORELIST						= 0x05A,
	CMSG_NPC_BUYITEM							= 0x05B,
	CMSG_NPC_SELLITEM							= 0x05C,
	SMSG_ALERT_MSG								= 0x05D,
    SMSG_PARTY_COMMAND_RESULT                   = 0x05E,
    CMSG_GROUP_INVITE                           = 0x05F,

    CMSG_GROUP_ACCEPT                           = 0x060,
    CMSG_GROUP_DECLINE                          = 0x061,
    CMSG_GROUP_UNINVITE_GUID                    = 0x062,
    CMSG_GROUP_UNINVITE                         = 0x063,
    CMSG_GROUP_SETLEADER                        = 0x064,
    CMSG_GROUP_DISBAND                          = 0x065,
    CMSG_REQUEST_PARTY_MEMBER_STATS             = 0x066,
    SMSG_GROUP_INVITE                           = 0x067,
    SMSG_GROUP_DECLINE                          = 0x068,
    SMSG_PARTY_MEMBER_STATS_FULL                = 0x069,
    SMSG_GROUP_LIST                             = 0x06A,
    SMSG_GROUP_DESTROYED                        = 0x06B,
    CMSG_GROUP_REQUEST_JOIN                     = 0x06C,
    CMSG_GROUP_MAP_LIST                         = 0x06D,
    SMSG_GROUP_MAP_LIST                         = 0x06E,
    SMSG_GROUP_REQUEST_JOIN                     = 0x06F,

    CMSG_GROUP_REQUEST_ACCEPT                   = 0x070,
    CMSG_GROUP_REQUEST_DENY                     = 0x071,
    CMSG_GROUP_QUERY_MEMBERS                    = 0x072,
    SMSG_GROUP_QUERY_MEMBERS                    = 0x073,
    CMSG_GROUP_LEAVE                            = 0x074,
    SMSG_GROUP_LEAVE                            = 0x075,
    SMSG_CRYSTAL_STATS                          = 0x076,		//五色宝石数据更新
    CMSG_SPELL_LEVEL_UP                         = 0x077,
    SMSG_SPELL_STAT_CHANGE                      = 0x078,
    SMSG_SHORTCUTS_LIST                         = 0x079,
    CMSG_SHORTCUTS_SET                          = 0x07A,
	SMSG_QUESTBOARD_LIST						= 0x07B,
	SMSG_QUESTBOARD_ONE							= 0x07C,
	SMSG_QUESTBOARD_STATE						= 0x07D,
	CMSG_QUESTBOARD_UPDATE						= 0x07E,
	CMSG_GM										= 0x07F,		// GM命令

	CMSG_CHAT									= 0x080,		// 聊天
	CMSG_PET_OPERATE							= 0x081,		// 宠物操作
	SMSG_PET_SYNCPROPERTY						= 0x082,		// 同步单条属性
	SMSG_PET_LISTS								= 0x083,		// 宠物列表
	CMSG_TALK_NPC								= 0x084,		// 和npc对话
	
	CMSG_ADD_FRIEND								= 0x085,		// 添加好友
	CMSG_DEL_RELATION							= 0x086,		// 删除
	CMSG_MOV_RELATION							= 0x087,		// 移动关系
	SMSG_SYNC_RELATION							= 0x088,		// 同步列表
	SMSG_SYNC_RELATION_ONLINE					= 0x089,		// 好友上线
	SMSG_ERROR_TIP								= 0x08A,		// 错误提示
	CMSG_PET_RENAME								= 0x08B,		// 修改名字
	CMSG_SEND_MAIL								= 0x08C,		// 发送邮件
	CMSG_DELE_MAIL								= 0x08D,		// 删除邮件
	CMSG_READ_MAIL								= 0x08E,		// 请求读邮件
	SMSG_SYNC_MAIL_LIST							= 0x08F,		// 同步邮件列表

	SMSG_READ_MAIL								= 0x090,		// 返回邮件内容
	SMSG_RELATION_ERROR							= 0x091,		// 好友错误提示
	CMSG_SCAREER_ASK							= 0x092,
	SMSG_SCAREER_SYNC							= 0x093,
	// quest
	SMSG_QUEST_SYNC_LIST						= 0x094,
	SMSG_QUEST_SYNC_DEL							= 0x095,
	SMSG_QUEST_SYNC_SUGGEST						= 0x096,
	SMSG_QUEST_SUGGET_DEL						= 0x097,
	CMSG_QUEST_OPERATE							= 0x098,
	SMSG_QUEST_SYNC_DATA						= 0x099,
	SMSG_MAIL_RESULT							= 0x09A,
	CMSG_MAIL_ASK_MAIL_LIST						= 0x09B,
	SMSG_QUEST_UPDATE							= 0x09C,
    SMSG_MOUNTS_SEND_MOUNTSLIST                 = 0x09D,
    CMSG_MOUNTS_ACTIVE                          = 0x09E,
    CMSG_MOUNTS_INACTIVE                        = 0x09F,

    CMSG_MOUNTS_UPGRADE                         = 0x0A0,
    SMSG_MOUNTS_UP_RESULT                       = 0x0A1,
    CMSG_CLICK_CREATURE                         = 0x0A2,
	SMSG_COMBAT_UNITLEAVE                       = 0x0A3,
	CMSG_TELE			                        = 0x0A4,
	CMSG_CREAT_ROLE								= 0x0A5,
	SMSG_CREATE_ROLE							= 0x0A6,
    CMSG_MOUNTSSUPERMAN                         = 0x0A7,
    SMSG_MOUNTSSUPERMAN                         = 0x0A8,
	CMSG_ITEM_AUGMENT                           = 0x0A9,
	SMSG_ITEM_AUGMENT_REPLY                     = 0x0AA,
	CMSG_ITEM_ROLL                              = 0x0AB,
	CMSG_ITEM_INPUT                             = 0x0AC,
	CMSG_BAG_OPERATION                          = 0x0AD,
	SMSG_BAG_OPERATION                          = 0x0AE,
	//new message
	SMSG_QUEST_INFO								= 0x0AF,
	//
	NUM_MSG_TYPES								= 0x0B0
};

const size_t CMSG_ROBOT=0;

#endif // _protocal_h_