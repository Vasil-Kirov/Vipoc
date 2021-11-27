#include "hot_reload.h"

float NormalizeCoordinate(float X, float MaxX, float MinX)
{
	float Result;
	Result = (2.0f * ((X - MinX) / (MaxX - MinX))) - 1.0f;
	return Result;
}
float NormalizeTexCoordinate(float X, float MaxX, float MinX)
{
	float Result;
	Result = ((X - MinX) / (MaxX - MinX));
	return Result;
}

vec4 CalculateAtlasPos(atlas_member Member, int AtlasWidth, int AtlasHeight)
{
	vec4 AtlasPos = {};
	AtlasPos.x1 = NormalizeTexCoordinate(Member.x, AtlasWidth, 0);
	AtlasPos.y1 = NormalizeTexCoordinate(Member.y, AtlasHeight, 0);
	AtlasPos.x2 = NormalizeTexCoordinate(Member.x + Member.w, AtlasWidth, 0);
	AtlasPos.y2 = NormalizeTexCoordinate(Member.y + Member.h, AtlasHeight, 0);
	return AtlasPos;
}

vec4 CalculateWorldPos(atlas_member Member, float X, float Y, vp_game Game)
{
	vec4 Pos;
	Pos.x1 = NormalizeCoordinate(X, Game.config.w, 0);
	Pos.y1 = NormalizeCoordinate(Y, Game.config.h, 0);
	Pos.x2 = NormalizeCoordinate(X + Member.w, Game.config.w, 0);
	Pos.y2 = NormalizeCoordinate(Y + Member.h, Game.config.h, 0);
	return Pos;
}

vp_render_target
CalculateRenderTarget(atlas_member Member, float X, float Y, int AtlasWidth, int AtlasHeight, int LayerID, vp_game Game, int XOffset, int YOffset)
{
	vp_render_target Result = {};
	Result.layer_id = LayerID;
	Result.world_position = CalculateWorldPos(Member, X-XOffset, Y-YOffset, Game);
	Result.texture_position = CalculateAtlasPos(Member, AtlasWidth, AtlasHeight);
	return Result;
}


__declspec(dllexport) GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
	int RenderLastIndex = 0;
	Targets[RenderLastIndex++] = CalculateRenderTarget(Members[1], PlayerX, PlayerY, AtlasSize.w, AtlasSize.h, 2, Game, XOffset, YOffset);
	Targets[RenderLastIndex++] = CalculateRenderTarget(Members[2], 100, 200, AtlasSize.w, AtlasSize.h, 1, Game, XOffset, YOffset);
	return RenderLastIndex;
}

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL, // handle to DLL module
	DWORD fdwReason,	// reason for calling function
	LPVOID lpReserved)	// reserved
	{
		return TRUE;
	}