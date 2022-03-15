#include "includes.h"

ID3DXLine* LineL;
ID3DXFont* Font;



void DrawFilledRect(int x, int y, int w, int h, D3DCOLOR color) {

	D3DRECT rect = { x,y,x + w,y + h };
	pDevice->Clear(1, &rect, D3DCLEAR_TARGET, color, 0, 0);

}

void DrawLine(int x1, int y1, int x2, int y2, int thickness, D3DCOLOR color) {
	
	if (!LineL) {
		D3DXCreateLine(pDevice, &LineL);
	}
	D3DXVECTOR2 Line[2];
	Line[0] = D3DXVECTOR2(x1, y1);
	Line[1] = D3DXVECTOR2(x2, y2);
	LineL->SetWidth(thickness);
	LineL->Draw(Line, 2, color);
}

void DrawLine2(Vec2 src, Vec2 dst, int thickness, D3DCOLOR color) {
	DrawLine(src.x, src.y, dst.x, dst.y, thickness, color);
}

void DrawEspBox2D(Vec2 top, Vec2 bot, int thickness, D3DCOLOR color) {
	int height = ABS(top.y - bot.y);
	Vec2 topLeft, topRight;
	topLeft.x = top.x - height / 4;
	topRight.x = top.x + height / 4;
	topLeft.y = topRight.y = top.y;
	Vec2 bottomLeft, bottomRight;
	bottomLeft.x = bot.x - height / 4;
	bottomRight.x = bot.x + height / 4;
	bottomLeft.y = bottomRight.y = bot.y;

	DrawLine2(topLeft, topRight, thickness, color);
	DrawLine2(bottomLeft, bottomRight, thickness, color);
	DrawLine2(topLeft, bottomLeft, thickness, color);
	DrawLine2(topRight, bottomRight, thickness, color);
}

void DrawEspBox3D(Vec3 top, Vec3 bot, float a, int width, int thickness, D3DCOLOR color) {
	int height3D = top.z - bot.z;
	Vec3 b1, b2, b3, b4, t1, t2, t3, t4;
	b1.z = b2.z = b3.z = b4.z = bot.z;
	b1.x = bot.x + (cos(TORAD(a + 45)) * width);
	b1.y = bot.y + (sin(TORAD(a + 45)) * width);

	b2.x = bot.x + (cos(TORAD(a + 135)) * width);
	b2.y = bot.y + (sin(TORAD(a + 135)) * width);

	b3.x = bot.x + (cos(TORAD(a + 225)) * width);
	b3.y = bot.y + (sin(TORAD(a + 225)) * width);

	b4.x = bot.x + (cos(TORAD(a + 315)) * width);
	b4.y = bot.y + (sin(TORAD(a + 315)) * width);

	t1.x = b1.x;
	t1.y = b1.y;
	t1.z = b1.z + height3D;

	t2.x = b2.x;
	t2.y = b2.y;
	t2.z = b2.z + height3D;

	t3.x = b3.x;
	t3.y = b3.y;
	t3.z = b3.z + height3D;

	t4.x = b4.x;
	t4.y = b4.y;
	t4.z = b4.z + height3D;

	Vec2 b1_2, b2_2, b3_2, b4_2, t1_2, t2_2, t3_2, t4_2;
	if (WorldToScreen(b1, b1_2) && WorldToScreen(b2, b2_2) && WorldToScreen(b3, b3_2) && WorldToScreen(b4, b4_2) && WorldToScreen(t1, t1_2) && WorldToScreen(t2, t2_2) && WorldToScreen(t3, t3_2) && WorldToScreen(t4, t4_2)) {
		// draw columns
		DrawLine2(t1_2, b1_2, thickness, color);
		DrawLine2(t2_2, b2_2, thickness, color);
		DrawLine2(t3_2, b3_2, thickness, color);
		DrawLine2(t4_2, b4_2, thickness, color);
		// draw top square
		DrawLine2(t1_2, t2_2, thickness, color);
		DrawLine2(t2_2, t3_2, thickness, color);
		DrawLine2(t3_2, t4_2, thickness, color);
		DrawLine2(t4_2, t1_2, thickness, color);
		// draw bottom square
		DrawLine2(b1_2, b2_2, thickness, color);
		DrawLine2(b2_2, b3_2, thickness, color);
		DrawLine2(b3_2, b4_2, thickness, color);
		DrawLine2(b4_2, b1_2, thickness, color);
	}
	
}

void TextToScreen(const char* text, float x, float y, D3DCOLOR color, int height, int width) {
	RECT rect;

	if (!Font) {
		D3DXCreateFontA(pDevice, height, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &Font);
	}
	SetRect(&rect, x + 1, y + 1, x + 1, y + 1);
	Font->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 0, 0));
	SetRect(&rect, x, y, x, y);
	Font->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_NOCLIP, color);
}