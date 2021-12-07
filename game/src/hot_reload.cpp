#include "hot_reload.h"


global_var rectangle Atlas;
global_var vp_game VPGame;
global_var int32 OffsetX;
global_var int32 OffsetY;

float NormalizeCoordinate(float X, float MaxX, float MinX, int From, int To)
{
	float Result;
	Result = ((To-From)*( (X-MinX)/(MaxX-MinX) ))+From;
	return Result;
}

m2 CalculateAtlasPos(atlas_member Member)
{
	m2 AtlasPos = {};
	AtlasPos.x1 = NormalizeCoordinate(Member.x, Atlas.w, 0, 0, 1);
	AtlasPos.y1 = NormalizeCoordinate(Member.y, Atlas.h, 0, 0, 1);
	AtlasPos.x2 = NormalizeCoordinate(Member.x + Member.w, Atlas.w, 0, 0, 1);
	AtlasPos.y2 = NormalizeCoordinate(Member.y + Member.h, Atlas.h, 0, 0, 1);
	return AtlasPos;
}

m2 CalculateWorldPos(float X, float Y, float Width, float Height)
{
	m2 Pos;
	Pos.x1 = NormalizeCoordinate(X, VPGame.config.w, 0, 0, 10);
	Pos.y1 = NormalizeCoordinate(Y, VPGame.config.h, 0, 0, 10);
	Pos.x2 = NormalizeCoordinate(X + Width, VPGame.config.w, 0, 0, 10);
	Pos.y2 = NormalizeCoordinate(Y + Height, VPGame.config.h, 0, 0, 10);
	return Pos;
}

vp_render_target
CalculateRenderTarget(atlas_member Member, float X, float Y, float Width, float Height, int LayerID)
{


	vp_render_target Result = {};
	Result.layer_id = LayerID;
	Result.world_position = (m2){X, Y, X+Width, Y+Height};
//	Result.world_position = CalculateWorldPos(X-OffsetX, Y-OffsetY, Width, Height);
	Result.texture_position = CalculateAtlasPos(Member);
	return Result;
}


extern "C" __declspec(dllexport) GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
	OffsetX = XOffset;
	OffsetY = YOffset;
	Atlas.w = AtlasSize.w;
	Atlas.h = AtlasSize.h;
	VPGame = Game;

	int RenderLastIndex = 0;
	Targets[RenderLastIndex++] = CalculateRenderTarget(Members[0], PlayerX, PlayerY, 0.4f, 1.74f, 2);
	Targets[RenderLastIndex++] = CalculateRenderTarget(Members[1], 2, 2, 0.4f, 1.74f, 1);
	return RenderLastIndex;
}

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL, // handle to DLL module
	DWORD fdwReason,	// reason for calling function
	LPVOID lpReserved)	// reserved
{
	return TRUE;
}