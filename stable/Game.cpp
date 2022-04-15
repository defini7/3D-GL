#include "Poison.h"

#define BLOCK_SIZE 1.0f
#define PLAYER_HEIGHT 4.0f

#define OUTPUT_WIDTH 40
#define OUTPUT_HEIGHT 40

struct sCamera
{
	def::vf3d pos;
	def::vf3d rot;

	float speed;
	float vel;
};

class Example : public def::Poison
{
protected:
	bool Start() override
	{
		float fLightPos[] = { 0.0f, 0.0f, 10.0f, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, fLightPos);

		srand(time(NULL));

		for (int i = 0; i < OUTPUT_WIDTH; i++)
			for (int j = 0; j < OUTPUT_HEIGHT; j++)
				fPerlinSeed2D[i][j] = (float)rand() / (float)RAND_MAX;

		DoPerlinNoise2D(nOctaves, fScaleBias);

		ShowCursor(false);
		
		return true;
	}

	bool Update() override
	{
		glRotatef(-vCamera.rot.x, 1, 0, 0);
		glRotatef(-vCamera.rot.z, 0, 0, 1);
		glTranslatef(-vCamera.pos.x, -vCamera.pos.y, -vCamera.pos.z);

		RotateCamera();

		if (vCamera.vel > 0.2f)
			vCamera.vel = 0.2f;

		vCamera.pos.z -= vCamera.vel;

		for (int x = 0; x < OUTPUT_WIDTH; x++)
			for (int y = 0; y < OUTPUT_HEIGHT; y++)
				for (int z = 0; z < 64; z++)
				{
					if (bWorld[x][y][z])
					{
						if (x <= vCamera.pos.x && y <= vCamera.pos.y && vCamera.pos.x <= x + BLOCK_SIZE && vCamera.pos.y <= y + BLOCK_SIZE)
						{
							if (z + PLAYER_HEIGHT > vCamera.pos.z && vCamera.pos.z - PLAYER_HEIGHT / 2.0f > z)
							{
								vCamera.pos.z = z + PLAYER_HEIGHT;
								vCamera.vel = 0.0f;
								vCamera.speed = 0.0f;
								bJumpActive = false;
								bJumpEnds = false;
							}
						}						
						
						DrawCube(x, y, z);
					}
				}

		if (bJumpActive)
		{
			if (vCamera.vel > -0.3f && !bJumpEnds)
			{
				vCamera.speed = 0.05f;
				vCamera.vel -= 0.04f;
			}
			else
			{
				bJumpEnds = true;
				vCamera.vel += 0.03f;
			}
		}
		else
		{
			vCamera.speed = GetKey(VK_SHIFT).bHeld ? 0.05f : 0.0f;
			
			if (GetKey(VK_SPACE).bHeld)
			{
				vCamera.vel -= 0.1f;
				bJumpActive = true;
			}
		}

		vCamera.vel += 0.03f * !bJumpActive;
		
		return true;
	}

private:
	bool bWorld[OUTPUT_WIDTH][OUTPUT_HEIGHT][64];
	float fPerlinSeed2D[OUTPUT_WIDTH][OUTPUT_HEIGHT];

	const float fScaleBias = 1.0f;
	const float nOctaves = 12;

	sCamera vCamera = {
		{ 5.0f, 5.0f, 10.0f },
		{ 70.0f, 0.0f, -40.0f },
		0.0f, 0.0f
	};

	bool bJumpActive = false;
	bool bJumpEnds = false;

	void RotateCamera()
	{
		if (!IsFocused()) return;

		auto rotate = [&](float xAngle, float zAngle)
		{
			vCamera.rot.x += xAngle;
			vCamera.rot.z += zAngle;

			if (vCamera.rot.x < 0.0f) vCamera.rot.x = 0.0f;
			if (vCamera.rot.x > 180.0f) vCamera.rot.z = 180.0f;

			if (vCamera.rot.z < 0.0f) vCamera.rot.z += 360.0f;
			if (vCamera.rot.z > 360.0f) vCamera.rot.z -= 360.0f;
		};


		float fAngle = -vCamera.rot.z / 180.0f * def::PI;
		float fSpeed = 0.0f;

		if (GetKey(L'W').bHeld) fSpeed = 0.1f + vCamera.speed;
		if (GetKey(L'S').bHeld) fSpeed = -0.1f;
		if (GetKey(L'A').bHeld) { fSpeed = 0.1f; fAngle -= def::PI / 2.0f; }
		if (GetKey(L'D').bHeld) { fSpeed = 0.1f; fAngle += def::PI / 2.0f; }

		if (fSpeed != 0.0f)
		{
			float x = vCamera.pos.x + sinf(fAngle) * fSpeed;
			float y = vCamera.pos.y + cosf(fAngle) * fSpeed;
			float z = vCamera.pos.z;

			if (!bWorld[(int)x][(int)vCamera.pos.y][int(z - PLAYER_HEIGHT) + 1])
				vCamera.pos.x = x;

			if (!bWorld[(int)vCamera.pos.x][(int)y][int(z - PLAYER_HEIGHT) + 1])
				vCamera.pos.y = y;
		}

		static POINT base = { GetScreenWidth() / 2, GetScreenHeight() / 2 };
		rotate((base.y - GetMouseY()) / 5.0f, (base.x - GetMouseX()) / 5.0f);
		SetCursorPos(base.x, base.y);
	}

	void DrawCube(float x, float y, float z)
	{
		glTranslatef(x, y, z);

		glBegin(GL_TRIANGLES);

		glColor3f(0.5f, 0.5f, 0.5f);

		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 1);
		glVertex3f(1, 0, 0);

		glVertex3f(0, 0, 1);
		glVertex3f(1, 0, 1);
		glVertex3f(1, 0, 0);

		glColor3f(0.1f, 0.5f, 0.5f);

		glVertex3f(0, 1, 0);
		glVertex3f(0, 1, 1);
		glVertex3f(1, 1, 0);

		glVertex3f(0, 1, 1);
		glVertex3f(1, 1, 1);
		glVertex3f(1, 1, 0);

		glColor3f(0.1f, 0.2f, 0.5f);

		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 1);
		glVertex3f(0, 1, 0);

		glVertex3f(0, 1, 0);
		glVertex3f(0, 1, 1);
		glVertex3f(0, 0, 1);

		glColor3f(0.3f, 0.2f, 0.1f);

		glVertex3f(1, 0, 0);
		glVertex3f(1, 0, 1);
		glVertex3f(1, 1, 0);

		glVertex3f(1, 1, 0);
		glVertex3f(1, 1, 1);
		glVertex3f(1, 0, 1);

		glColor3f(0.5f, 0.7f, 0.6f);

		glVertex3f(0, 0, 1);
		glVertex3f(0, 1, 1);
		glVertex3f(1, 0, 1);

		glVertex3f(1, 0, 1);
		glVertex3f(1, 1, 1);
		glVertex3f(0, 1, 1);

		glColor3f(0.75f, 0.1f, 0.3f);

		glVertex3f(0, 0, 0);
		glVertex3f(0, 1, 0);
		glVertex3f(1, 0, 0);

		glVertex3f(1, 0, 0);
		glVertex3f(1, 1, 0);
		glVertex3f(0, 1, 0);

		glEnd();

		glTranslatef(-x, -y, -z);
	}

	void DoPerlinNoise2D(int nOctaves, float fBias)
	{
		for (int x = 0; x < OUTPUT_WIDTH; x++)
			for (int y = 0; y < OUTPUT_HEIGHT; y++)
			{
				float fNoise = 0.0f;
				float fScale = 1.0f;
				float fScaleAccumulator = 0.0f;

				for (int o = 0; o < nOctaves; o++)
				{
					int nPitch = OUTPUT_WIDTH >> o;

					if (nPitch != 0) {
						int nSampleX1 = (x / nPitch) * nPitch;
						int nSampleY1 = (y / nPitch) * nPitch;

						int nSampleX2 = (nSampleX1 + nPitch) % OUTPUT_WIDTH;
						int nSampleY2 = (nSampleY1 + nPitch) % OUTPUT_WIDTH;

						float fBlendX = (float)(x - nSampleX1) / (float)nPitch;
						float fBlendY = (float)(y - nSampleY1) / (float)nPitch;

						float fSampleT = (1.0f - fBlendX) * fPerlinSeed2D[nSampleX1][nSampleY1] + fBlendX * fPerlinSeed2D[nSampleX2][nSampleY1];
						float fSampleB = (1.0f - fBlendX) * fPerlinSeed2D[nSampleX1][nSampleY2] + fBlendX * fPerlinSeed2D[nSampleX2][nSampleY2];

						fNoise += (fBlendY * (fSampleB - fSampleT) + fSampleT) * fScale;
						fScaleAccumulator += fScale;
						fScale /= fBias;
					}
				}

				bWorld[x][y][int(fNoise / fScaleAccumulator * 10) / 2] = true;
			}
	}
};

int main()
{
	Example demo;

	if (!demo.Run(1024, 768, L"Hello, World!"))
		return 1;

	return 0;
}