

float4 vs_main(uint id: SV_VertexID) : SV_Position
{
	float4 positions[3] = {
	float4(-0.5, -0.5, 0, 1),
	float4(0, 0.5, 0, 1),
	float4(0.5, -0.5, 0, 1)
};
	return positions[id];
}

float4 ps_main() : SV_Target
{
	return float4(1, 1, 1, 1);
}