#pragma once
#include "ClientPacketHandler.h"

unordered_map<uint16, string> packetIdToString =
{
	{PKT_S_SERVER_CHAT, "PKT_S_SERVER_CHAT"},

	{PKT_C_TEST, "PKT_C_TEST"},
	{PKT_S_TEST, "PKT_S_TEST"},

	{PKT_C_SIGNUP, "PKT_C_SIGNUP"},
	{PKT_S_SIGNUP, "PKT_S_SIGNUP"},

	{PKT_C_LOGIN, "PKT_C_LOGIN"},
	{PKT_S_LOGIN, "PKT_S_LOGIN"},

	{PKT_C_ENTER_GAME, "PKT_C_ENTER_GAME"},
	{PKT_S_ENTER_GAME, "PKT_S_ENTER_GAME"},

	{PKT_C_CHAT, "PKT_C_CHAT"},
	{PKT_S_CHAT, "PKT_S_CHAT"},

	{PKT_C_MOVE, "PKT_C_MOVE"},
	{PKT_S_MOVE, "PKT_S_MOVE"},

	{PKT_C_SHOT, "PKT_C_SHOT"},
	{PKT_S_SHOT, "PKT_S_SHOT"},
};

class PacketUtils
{
};

