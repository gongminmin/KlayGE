void Transform2D(float4 position : POSITION, int halfWidth, int halfHeight,

			out float4 oPosition : POSITION)
{
	oPosition.x = (position.x - halfWidth) / halfWidth;
	oPosition.y = (halfHeight - position.y) / halfHeight;
	oPosition.zw = position.zw;
}
