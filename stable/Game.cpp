#define _CRT_SECURE_NO_WARNINGS

#include "Engine.h"

#include <map>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

extern "C"
{
#include "lua542/include/lua.h"
#include "lua542/include/lauxlib.h"
#include "lua542/include/lualib.h"
}

#ifdef _WIN32
#pragma comment(lib, "lua542/liblua54.a")
#endif

struct sCamera
{
	def::vf3d pos;
	float scale;
};

struct sPlayer
{
	def::vf2d pos;
};

struct sTile
{
	bool isEmpty;
	int height;

	int textureIndex;
	GLuint texture;
	float textureAngle; // in degrees
};

struct sImage
{
	unsigned char* data;

	int width;
	int height;
	int channels;
};

enum TILES
{
	GRASS,
	ROAD_TILE,
	ROAD,
	ROAD_CROSSWALK,
	INTERSECTION,
	RIGHT_ROAD,
	CAR // keep it always at the bottom
};

class Example : public def::Poison
{
public:
	Example(lua_State* state, int world_width, int world_height)
	{
		L = state;
		
		nWorldWidth = (world_width < 30) ? 30 : world_width;
		nWorldHeight = (world_height < 15) ? 15 : world_height;
	}

protected:
	bool Start() override
	{
		for (int i = -nWorldWidth * nWorldHeight; i < nWorldWidth * nWorldHeight; i++)
			tWorld[i] = { true, 0, 0, GRASS, 0.0f };

		textures[GRASS] = LoadTexture("sprites/grass.png");
		textures[ROAD_TILE] = LoadTexture("sprites/road_tile.jpg");
		textures[ROAD] = LoadTexture("sprites/horizontal.jpg");
		textures[ROAD_CROSSWALK] = LoadTexture("sprites/horizontal_crosswalk.jpg");
		textures[INTERSECTION] = LoadTexture("sprites/intersection.jpg");
		textures[RIGHT_ROAD] = LoadTexture("sprites/right.jpg");
		textures[CAR] = LoadTexture("sprites/car_top1.png", true);

		// LUA STUFF

		LoadLevel();

		// END LUA STUFF

		return true;
	}

	bool Update() override
	{
		glScalef(vCamera.scale, vCamera.scale, 1.0f);
		glTranslatef(-vCamera.pos.x, -vCamera.pos.y, -vCamera.pos.z);

		if (GetKey(VK_LBUTTON).bPressed)
		{
			int p = vSelectedArea.y * nWorldWidth + vSelectedArea.x;

			tWorld[p].isEmpty = false;
			tWorld[p].height++;

			if (tWorld[p].height > 4)
				tWorld[p].height = 4;
		}

		if (GetKey(VK_RBUTTON).bPressed)
		{
			int p = vSelectedArea.y * nWorldWidth + vSelectedArea.x;

			tWorld[p].height--;

			if (tWorld[p].height <= 0)
			{
				tWorld[p].isEmpty = true;
				tWorld[p].height = 0;
			}
		}

		if (GetKey(L'E').bPressed)
		{
			int p = vSelectedArea.y * nWorldWidth + vSelectedArea.x;

			if (tWorld[p].textureIndex < 5)
				tWorld[p].texture = textures[++tWorld[p].textureIndex];
		}

		if (GetKey(L'Q').bPressed)
		{
			int p = vSelectedArea.y * nWorldWidth + vSelectedArea.x;

			if (tWorld[p].textureIndex > 0)
				tWorld[p].texture = textures[--tWorld[p].textureIndex];
		}

		if (GetKey(L'O').bPressed)
		{
			int p = vSelectedArea.y * nWorldWidth + vSelectedArea.x;
			tWorld[p].textureAngle -= 90.0f;
		}

		if (GetKey(L'P').bPressed)
		{
			int p = vSelectedArea.y * nWorldWidth + vSelectedArea.x;
			tWorld[p].textureAngle += 90.0f;
		}

		if (GetKey(L'Z').bHeld)
			vCamera.scale += 0.1f;

		if (GetKey(L'X').bHeld)
			vCamera.scale -= 0.1f;

		if (vCamera.scale < 0.1f)
			vCamera.scale = 0.1f;

		if (GetKey(VK_LEFT).bHeld)
			vCamera.pos.x -= 0.001f * nWorldWidth;

		if (GetKey(VK_RIGHT).bHeld)
			vCamera.pos.x += 0.001f * nWorldWidth;

		if (GetKey(VK_DOWN).bHeld)
			vCamera.pos.y -= 0.001f * nWorldHeight;

		if (GetKey(VK_UP).bHeld)
			vCamera.pos.y += 0.001f * nWorldHeight;

		if (GetKey(L'A').bPressed)
			vSelectedArea.x -= 1;

		if (GetKey(L'D').bPressed)
			vSelectedArea.x += 1;

		if (GetKey(L'S').bPressed)
			vSelectedArea.y -= 1;

		if (GetKey(L'W').bPressed)
			vSelectedArea.y += 1;

		DrawSelectedArea();

		for (int i = -nWorldWidth / 2; i < nWorldWidth / 2; i++)
			for (int j = -nWorldHeight / 2; j < nWorldHeight / 2; j++)
			{
				int p = j * nWorldWidth + i;

				if (!tWorld[p].isEmpty)
				{
					for (int k = 0; k < tWorld[p].height; k++)
						DrawCube(i, j, k);
				}
				
				DrawTile(i, j, textures[tWorld[p].texture - 1], tWorld[p].textureAngle);
			}

		glPushMatrix();
			glScalef(1.0f, 0.5f, 1.0f);
			glTranslatef(0.0f, 0.5f, 0.0f);
			DrawTile(player.pos.x, player.pos.y, textures[CAR], 0.0f);
		glPopMatrix();

		return true;
	}

	void Destroy() override
	{
		glDisable(GL_TEXTURE_2D);
	}

private:
	lua_State* L = nullptr;

	std::map<int, sTile> tWorld;

	GLuint textures[7];

	sCamera vCamera = {
		{ 0.0f, 0.0f, 10.0f },
		1.0f
	};

	sPlayer player = { 0.0f, 0.0f };

	def::vi2d vSelectedArea = { 0, 0 };

	int nWorldWidth;
	int nWorldHeight;

	bool LoadLevel()
	{
		lua_getglobal(L, "WorldMap");

		if (lua_istable(L, -1))
		{
			int x = -nWorldWidth / 2;
			int y = -nWorldHeight / 2;

			for (int k = nWorldHeight / 2; k > -nWorldHeight / 2; k--)
			{
				lua_pushnumber(L, k);
				lua_gettable(L, -2);
				char* pMapLine = (char*)lua_tostring(L, -1);
				lua_pop(L, 1);

				for (int i = 0; i < nWorldWidth; i++, x++)
				{
					int p = y * nWorldWidth + x;
					switch (pMapLine[i])
					{
					case '-':
						tWorld[p].texture = textures[ROAD];
						tWorld[p].textureIndex = ROAD;
						break;

					case '|':
						tWorld[p].texture = textures[ROAD];
						tWorld[p].textureAngle = -90.0f;
						tWorld[p].textureIndex = ROAD;
						break;

					case '@':
						tWorld[p].texture = textures[ROAD_TILE];
						tWorld[p].textureIndex = ROAD_TILE;
						break;

					case '=':
						tWorld[p].texture = textures[ROAD_CROSSWALK];
						tWorld[p].textureIndex = ROAD_CROSSWALK;
						break;

					case '"':
						tWorld[p].texture = textures[ROAD_CROSSWALK];
						tWorld[p].textureAngle = -90.0f;
						tWorld[p].textureIndex = ROAD_CROSSWALK;
						break;

					case '>':
						tWorld[p].texture = textures[RIGHT_ROAD];
						tWorld[p].textureIndex = RIGHT_ROAD;
						break;

					case '<':
						tWorld[p].texture = textures[RIGHT_ROAD];
						tWorld[p].textureAngle = -180.0f;
						tWorld[p].textureIndex = RIGHT_ROAD;
						break;

					case '+':
						tWorld[p].texture = textures[INTERSECTION];
						tWorld[p].textureIndex = INTERSECTION;
						break;

					case '#':
						tWorld[p].texture = textures[GRASS];
						tWorld[p].textureIndex = GRASS;
						break;

					case '\0':
						break;

					default:
						if (std::isdigit(pMapLine[i]))
						{
							tWorld[p].height = (int)pMapLine[i] - 48;
							tWorld[p].isEmpty = false;
						}
						else
							return true;
					}

				}

				x = -nWorldWidth / 2;
				if (++y == nWorldHeight / 2)
					return true;
			}
		}
		
		return true;
	}

	GLuint LoadTexture(const char* sFilename, bool bClamp = false)
	{
		int nWidth, nHeight, nChannels;

		stbi_set_flip_vertically_on_load(1);
		unsigned char* data = stbi_load(sFilename, &nWidth, &nHeight, &nChannels, 0);

		if (!data)
			throw std::exception(stbi_failure_reason());

		GLuint nTextureId = 0;
		GLenum format = 0;

		switch (nChannels)
		{
		case 1: format = GL_RED; break;
		case 3: format = GL_RGB; break;
		case 4: format = GL_RGBA; break;
		default: throw std::exception("Unsupported color format.");
		}

		glGenTextures(1, &nTextureId);
		glBindTexture(GL_TEXTURE_2D, nTextureId);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			format,
			nWidth, 
			nHeight,
			0,
			format,
			GL_UNSIGNED_BYTE,
			data
		);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		if (bClamp)
		{
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		
		stbi_image_free(data);

		return nTextureId;
	}

	void DrawTile(float x, float y, GLuint texture, float angle)
	{
		glTranslatef(x, y, 0.0f);
		glRotatef(angle, 0.0f, 0.0f, 1.0f);	

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);

		glPushMatrix();
			glColor3f(1.0f, 1.0f, 1.0f);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glBindTexture(GL_TEXTURE_2D, texture);

			glBegin(GL_QUADS);
				glTexCoord3f(-1.0f, 1.0f, 0.0f);  glVertex3f(-1, 1, 0);
				glTexCoord3f(1.0f, 1.0f, 0.0f);  glVertex3f(1, 1, 0);
				glTexCoord3f(1.0f, -1.0f, 0.0f);  glVertex3f(1, -1, 0);
				glTexCoord3f(-1.0f, -1.0f, 0.0f);  glVertex3f(-1, -1, 0);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, 0);

		glPopMatrix();

		glRotatef(-angle, 0.0f, 0.0f, 1.0f);
		glTranslatef(-x, -y, 0.0f);
	}

	void DrawCube(float x, float y, float z)
	{
		glTranslatef(x, y, z);

		glBegin(GL_TRIANGLES);

		glColor3f(0.5f, 0.5f, 0.5f);

		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, 0.0f, 0.0f);

		glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, 0.0f, 0.0f);

		glColor3f(0.1f, 0.5f, 0.5f);

		glVertex3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, 1.0f, 1.0f);
		glVertex3f(1.0f, 1.0f, 0.0f);

		glVertex3f(0.0f, 1.0f, 1.0f);
		glVertex3f(1.0f, 1.0f, 1.0f);
		glVertex3f(1.0f, 1.0f, 0.0f);

		glColor3f(0.1f, 0.2f, 0.5f);

		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 1.0f, 0.0f);

		glVertex3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, 1.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);

		glColor3f(0.3f, 0.2f, 0.1f);

		glVertex3f(1.0f, 0.0f, 0.0f);
		glVertex3f(1.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, 1.0f, 0.0f);

		glVertex3f(1.0f, 1.0f, 0.0f);
		glVertex3f(1.0f, 1.0f, 1.0f);
		glVertex3f(1.0f, 0.0f, 1.0f);

		glColor3f(0.5f, 0.7f, 0.6f);

		glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 1.0f, 1.0f);
		glVertex3f(1.0f, 0.0f, 1.0f);

		glVertex3f(1.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, 1.0f, 1.0f);
		glVertex3f(0.0f, 1.0f, 1.0f);

		glEnd();

		glTranslatef(-x, -y, -z);
	}

	void DrawSelectedArea()
	{
		glTranslatef(vSelectedArea.x, vSelectedArea.y, 0.0f);

		glBegin(GL_LINES);
		
		glColor3f(1.0f, 0.0f, 0.0f);

		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 1.0f, 0.0f);

		glVertex3f(0.0f, 1.0f, 0.0f);
		glVertex3f(1.0f, 1.0f, 0.0f);

		glVertex3f(1.0f, 1.0f, 0.0f);
		glVertex3f(1.0f, 0.0f, 0.0f);

		glVertex3f(1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);

		glEnd();

		glTranslatef(-vSelectedArea.x, -vSelectedArea.y, 0.0f);
	}
};

int main()
{
	lua_State* L = luaL_newstate();

	int rcode = luaL_dofile(L, "config.lua");

	if (rcode == LUA_OK)
	{
		lua_getglobal(L, "WorldWidth");
		int nWorldWidth = lua_tonumber(L, -1);

		lua_getglobal(L, "WorldHeight");
		int nWorldHeight = lua_tonumber(L, -1);

		Example demo(L, nWorldWidth, nWorldHeight);

		lua_getglobal(L, "ScreenWidth");
		int nScreenWidth = lua_tonumber(L, -1);

		lua_getglobal(L, "ScreenHeight");
		int nScreenHeight = lua_tonumber(L, -1);

		if (nScreenWidth < 800)
			nScreenWidth = 800;

		if (nScreenWidth < 600)
			nScreenWidth = 600;

		if (!demo.Run(nScreenWidth, nScreenHeight, L"3D Game"))
			return 1;
	}
	else
	{
		const char* msg = lua_tostring(L, -1);
		std::cerr << msg << "\n";
		return 1;
	}

	return 0;
}
