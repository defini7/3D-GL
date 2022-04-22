#include "Poison.h"

#include <map>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define BLOCK_SIZE 1.0f
#define PLAYER_HEIGHT 4.0f

#define OUTPUT_WIDTH 30
#define OUTPUT_HEIGHT 15

struct sCamera
{
	def::vf3d pos;
	float scale;
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
	RIGHT_ROAD
};

class Example : public def::Poison
{
protected:
	bool Start() override
	{
		for (int i = -OUTPUT_WIDTH * OUTPUT_HEIGHT; i < OUTPUT_WIDTH * OUTPUT_HEIGHT; i++)
			tWorld[i] = { true, 0, -1, GRASS, 0.0f };

		textures[GRASS] = LoadTexture("sprites/grass.jpg");
		textures[ROAD_TILE] = LoadTexture("sprites/road_tile.jpg");
		textures[ROAD] = LoadTexture("sprites/horizontal.jpg");
		textures[ROAD_CROSSWALK] = LoadTexture("sprites/horizontal_crosswalk.jpg");
		textures[INTERSECTION] = LoadTexture("sprites/intersection.jpg");
		textures[RIGHT_ROAD] = LoadTexture("sprites/right.jpg");

		LoadLevel("1.lvl");

		return true;
	}

	bool Update() override
	{
		glScalef(vCamera.scale, vCamera.scale, 1.0f);
		glTranslatef(-vCamera.pos.x, -vCamera.pos.y, -vCamera.pos.z);

		if (GetKey(VK_LBUTTON).bPressed)
		{
			int p = vSelectedArea.y * OUTPUT_WIDTH + vSelectedArea.x;

			tWorld[p].isEmpty = false;
			tWorld[p].height++;

			if (tWorld[p].height > 4)
				tWorld[p].height = 4;
		}

		if (GetKey(VK_RBUTTON).bPressed)
		{
			int p = vSelectedArea.y * OUTPUT_WIDTH + vSelectedArea.x;

			tWorld[p].height--;

			if (tWorld[p].height <= 0)
			{
				tWorld[p].isEmpty = true;
				tWorld[p].height = 0;
			}
		}

		if (GetKey(L'E').bPressed)
		{
			int p = vSelectedArea.y * OUTPUT_WIDTH + vSelectedArea.x;

			if (tWorld[p].textureIndex < 4)
			{
				tWorld[p].textureIndex++;
				tWorld[p].texture = textures[tWorld[p].textureIndex];
			}
		}

		if (GetKey(L'Q').bPressed)
		{
			int p = vSelectedArea.y * OUTPUT_WIDTH + vSelectedArea.x;

			if (tWorld[p].textureIndex > -1)
			{
				tWorld[p].textureIndex--;
				tWorld[p].texture = textures[tWorld[p].textureIndex];
			}
		}

		if (GetKey(L'O').bPressed)
		{
			int p = vSelectedArea.y * OUTPUT_WIDTH + vSelectedArea.x;
			tWorld[p].textureAngle -= 90.0f;
		}

		if (GetKey(L'P').bPressed)
		{
			int p = vSelectedArea.y * OUTPUT_WIDTH + vSelectedArea.x;
			tWorld[p].textureAngle += 90.0f;
		}

		if (GetKey(L'Z').bHeld)
			vCamera.scale += 0.1f;

		if (GetKey(L'X').bHeld)
			vCamera.scale -= 0.1f;

		if (vCamera.scale < 0.1f)
			vCamera.scale = 0.1f;

		if (GetKey(VK_LEFT).bHeld)
			vCamera.pos.x -= 0.001f * OUTPUT_WIDTH;

		if (GetKey(VK_RIGHT).bHeld)
			vCamera.pos.x += 0.001f * OUTPUT_WIDTH;

		if (GetKey(VK_DOWN).bHeld)
			vCamera.pos.y -= 0.001f * OUTPUT_HEIGHT;

		if (GetKey(VK_UP).bHeld)
			vCamera.pos.y += 0.001f * OUTPUT_HEIGHT;

		if (GetKey(L'A').bPressed)
			vSelectedArea.x -= 1;

		if (GetKey(L'D').bPressed)
			vSelectedArea.x += 1;

		if (GetKey(L'S').bPressed)
			vSelectedArea.y -= 1;

		if (GetKey(L'W').bPressed)
			vSelectedArea.y += 1;

		DrawSelectedArea();

		for (int i = -OUTPUT_WIDTH / 2; i < OUTPUT_WIDTH / 2; i++)
			for (int j = -OUTPUT_HEIGHT / 2; j < OUTPUT_HEIGHT / 2; j++)
			{
				if (!tWorld[j * OUTPUT_WIDTH + i].isEmpty)
				{
					for (int k = 0; k < tWorld[j * OUTPUT_WIDTH + i].height; k++)
						DrawCube(i, j, k);
				}

				DrawTile(i, j, textures[tWorld[j * OUTPUT_WIDTH + i].texture], tWorld[j * OUTPUT_WIDTH + i].textureAngle);
			}

		return true;
	}

	void Destroy() override
	{
		glDisable(GL_TEXTURE_2D);
	}

private:
	std::map<int, sTile> tWorld;

	sCamera vCamera = {
		{ 0.0f, 0.0f, 10.0f },
		1.0f
	};

	def::vi2d vSelectedArea = { 0, 0 };

	GLuint textures[6];

	void LoadLevel(const char* filename)
	{
		std::ifstream file;
		file.open(filename, std::ios::in);

		if (!file.is_open())
		{
			file.close();
			return;
		}

		int x = -OUTPUT_WIDTH / 2;
		int y = -OUTPUT_HEIGHT / 2;

		while (!file.eof())
		{
			if (file.bad())
			{
				file.close();
				return;
			}

			char s[OUTPUT_WIDTH];
			file.ignore();
			file.getline(s, OUTPUT_WIDTH);

			for (int i = 0; i < OUTPUT_WIDTH; i++, x++)
			{
				int p = y * OUTPUT_WIDTH + x;
				switch (s[i])
				{
				case '-':
					tWorld[p].texture = textures[ROAD - 1];
					tWorld[p].textureIndex = ROAD - 1;
					break;

				case '|':
					tWorld[p].texture = textures[ROAD - 1];
					tWorld[p].textureAngle = -90.0f;
					tWorld[p].textureIndex = ROAD - 1;
					break;

				case '@':
					tWorld[p].texture = textures[ROAD_TILE - 1];
					tWorld[p].textureIndex = ROAD_TILE - 1;
					break;

				case '=':
					tWorld[p].texture = textures[ROAD_CROSSWALK - 1];
					tWorld[p].textureIndex = ROAD_CROSSWALK - 1;
					break;

				case '"':
					tWorld[p].texture = textures[ROAD_CROSSWALK - 1];
					tWorld[p].textureAngle = -90.0f;
					tWorld[p].textureIndex = ROAD_CROSSWALK - 1;
					break;

				case '>':
					tWorld[p].texture = textures[RIGHT_ROAD - 1];
					tWorld[p].textureIndex = RIGHT_ROAD - 1;
					break;

				case '<':
					tWorld[p].texture = textures[RIGHT_ROAD - 1];
					tWorld[p].textureAngle = -180.0f;
					tWorld[p].textureIndex = RIGHT_ROAD - 1;
					break;

				case '+':
					tWorld[p].texture = textures[INTERSECTION - 1];
					tWorld[p].textureIndex = INTERSECTION - 1;
					break;
				
				case '#':
					tWorld[p].texture = textures[GRASS - 1];
					tWorld[p].textureIndex = GRASS - 1;
					break;

				case '\0':
					break;

				default:
					if (std::isdigit(s[i]))
					{
						tWorld[p].height = (int)s[i] - 48;
						tWorld[p].isEmpty = false;
					}
					else
					{
						file.close();
						return;
					}
				}
			}
		
			x = -OUTPUT_WIDTH / 2;
			if (++y == OUTPUT_HEIGHT / 2)
			{
				file.close();
				return;
			}
		}
		
		return;
	}

	GLuint LoadTexture(const char* filename)
	{
		int nWidth, nHeight, nChannels;

		stbi_set_flip_vertically_on_load(1);
		unsigned char* data = stbi_load(filename, &nWidth, &nHeight, &nChannels, 0);

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

		glBindTexture(GL_TEXTURE_2D, 0);
		
		stbi_image_free(data);

		return nTextureId;
	}

	void DrawTile(float x, float y, GLuint texture, float angle)
	{
		glTranslatef(x, y, 0.0f);
		glRotatef(angle, 0.0f, 0.0f, 1.0f);	

		glEnable(GL_TEXTURE_2D);

		glPushMatrix();
			glColor3f(1.0f, 1.0f, 1.0f);

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
	Example demo;

	if (!demo.Run(1024, 768, L"Hello, World!"))
		return 1;

	return 0;
}
