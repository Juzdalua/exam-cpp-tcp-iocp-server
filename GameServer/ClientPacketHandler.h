#pragma once

enum : uint16
{
	PKT_S_SERVER_CHAT = 9999,
	PKT_S_CREATE_ROOM = 9998,

	PKT_S_DISCONNECT = 999,

	PKT_C_TEST = 1000,
	PKT_S_TEST = 1001,

	PKT_C_SIGNUP = 1002,
	PKT_S_SIGNUP = 1003,

	PKT_C_LOGIN = 1004,
	PKT_S_LOGIN = 1005,

	PKT_C_ENTER_GAME = 1006,
	PKT_S_ENTER_GAME = 1007,

	PKT_C_CHAT = 1008,
	PKT_S_CHAT = 1009,

	PKT_C_MOVE = 1010,
	PKT_S_MOVE = 1011,

	PKT_C_SHOT = 1012,
	PKT_S_SHOT = 1013,

	PKT_C_HIT = 1014,
	PKT_S_HIT = 1015,

	PKT_C_EAT_ROOM_ITEM = 1016,
	PKT_S_EAT_ROOM_ITEM = 1017,

	PKT_C_USE_ITEM = 1018,
	PKT_S_USE_ITEM = 1019,

	PKT_C_CREATE_PARTY = 1020,
	PKT_S_CREATE_PARTY = 1021,

	PKT_C_JOIN_PARTY = 1022,
	PKT_S_JOIN_PARTY = 1023,

	PKT_C_WITHDRAW_PARTY = 1024,
	PKT_S_WITHDRAW_PARTY = 1025,

	PKT_C_MY_PARTY = 1026,
	PKT_S_MY_PARTY = 1027,

	PKT_C_ALL_PARTY = 1028,
	PKT_S_ALL_PARTY = 1029,
};

class ClientPacketHandler
{
public:
	static bool HandlePacket(BYTE* buffer, int32 len, GameProtobufSessionRef& session);

	// Disconnect
	static bool HandleDisconnect(GameProtobufSessionRef& session);

	// Ping-Pong
	static bool HandleTest(BYTE* buffer, int32 len, GameProtobufSessionRef& session);

	// Signup & Login
	static bool HandleSignup(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleLogin(BYTE* buffer, int32 len, GameProtobufSessionRef& session);

	// Enter Game
	static bool HandleEnterGame(BYTE* buffer, int32 len, GameProtobufSessionRef& session);

	// Chat
	static bool HandleChat(BYTE* buffer, int32 len, GameProtobufSessionRef& session);

	// Move
	static bool HandleMove(BYTE* buffer, int32 len, GameProtobufSessionRef& session);

	// Shot & Hit
	static bool HandleShot(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleHit(BYTE* buffer, int32 len, GameProtobufSessionRef& session);

	// Item
	static bool HandleEatRoomItem(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleUseItem(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	
	// Party
	static bool HandleCreateParty(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleJoinParty(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleWithdrawParty(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleGetMyParty(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleGetAllParty(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
};

