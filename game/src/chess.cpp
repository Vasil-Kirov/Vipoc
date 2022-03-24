#include <chess.h>



void
DrawChessBoard(i32 CubeIndex)
{

#if 0	
	if(CubeIndex < 0)
	{
		VP_ERROR("Invalid cube index: ", CubeIndex);
		return;
	}
	for(i32 Y = 0; Y < 8; ++Y)
	{
		for(i32 X = 0; X < 8; ++X)
		{
			u32 Color;
			if((X + Y) % 2 == 0) Color = 0xF5E7C0FF;
			// else Color = 0x232323FF;
			else Color = 0x66433BFF;
			v3 SquarePosition = {X*20.0f, 0.0f, Y*20.0f};
			//vp_object_pushback(CubeIndex, Color, SquarePosition, true, true);
		}
	}
	#endif

}