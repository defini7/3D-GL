#include <Windows.h>
#include <iostream>
#include <GL/gl.h>

#include <string>
#include <chrono>

#include <fstream>

#pragma comment(lib, "opengl32.lib")

#define MAX_NAME_LENGTH 256
#define HInstance() GetModuleHandle(NULL)

const double PI = 2.0 * acos(0.0);

int nScreenWidth = 800;
int nScreenHeight = 600;

float fMouseX = 0.0f;
float fMouseY = 0.0f;

#define MOUSE_LBUTTON 0
#define MOUSE_RBUTTON 1
#define MOUSE_WHEEL 3

struct MouseState
{
	bool bPressed;
	bool bReleased;
	bool bDblClick;

	float fWheelDelta;
};

struct KeyState
{
	bool bPressed;
	bool bReleased;
};

bool bShowCursor = true;

MouseState mouse[5];
KeyState keys[256];

LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	POINT cursor;
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		DestroyWindow(hWnd);
		return 0;

	case WM_MOUSEMOVE:
		GetCursorPos(&cursor);
		fMouseX = cursor.x;
		fMouseY = cursor.y;
		ShowCursor(bShowCursor);
		return 0;

	case WM_LBUTTONDOWN:
		mouse[0].bPressed = true;
		mouse[0].bReleased = false;
		return 0;

	case WM_LBUTTONUP:
		mouse[0].bPressed = false;
		mouse[0].bReleased = true;
		return 0;

	case WM_RBUTTONDOWN:
		mouse[1].bPressed = true;
		mouse[1].bReleased = false;
		return 0;

	case WM_RBUTTONUP:
		mouse[1].bPressed = false;
		mouse[1].bReleased = true;
		return 0;

	case WM_MOUSEWHEEL:
		mouse[3].fWheelDelta = HIWORD(wParam);
		return 0;

	case WM_KEYDOWN:
		keys[wParam].bPressed = true;
		keys[wParam].bReleased = false;
		return 0;

	case WM_KEYUP:
		keys[wParam].bPressed = false;
		keys[wParam].bReleased = true;
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

struct vf3d
{
	float x;
	float y;
	float z;
};

struct triangle
{
	vf3d v[3];
};

struct mesh
{
	std::vector<triangle> tris;

	bool LoadFile(std::string filename)
	{
		std::ifstream f(filename);

		std::vector<vf3d> verts;

		if (!f.is_open())
			return false;

		while (!f.eof())
		{
			char line[128];
			f.getline(line, 128);

			std::stringstream s;
			s << line;

			char junk;

			if (line[0] == 'v')
			{
				vf3d v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}

			if (line[0] == 'f')
			{
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
			}
		}

		return true;
	}
};

struct sCamera
{
	vf3d pos;
	vf3d rot;
};

sCamera vCamera = { 0, 0, -3 };

void RotateCamera(HWND* hWnd)
{
	auto rotate = [&](float xAngle, float zAngle)
	{
		vCamera.rot.x += xAngle;
		vCamera.rot.z += zAngle;

		if (vCamera.rot.x < 0.0f) vCamera.rot.x = 0.0f;
		if (vCamera.rot.x > 180.0f) vCamera.rot.z = 180.0f;

		if (vCamera.rot.z < 0.0f) vCamera.rot.z += 360.0f;
		if (vCamera.rot.z > 360.0f) vCamera.rot.z -= 360.0f;		
	};

	if (GetForegroundWindow() != *hWnd) return;

	static POINT base = { nScreenWidth / 2, nScreenHeight / 2 };
	rotate((base.y - fMouseY) / 5.0f, (base.x - fMouseX) / 5.0f);
	SetCursorPos(base.x, base.y);
}

mesh objFile;

bool OnUserCreate()
{
	bShowCursor = false;

	objFile.LoadFile("axis.obj");

	return true;
}

bool OnUserUpdate(HWND* hWnd)
{
	glTranslatef(vCamera.pos.x, vCamera.pos.y, vCamera.pos.z);
	glRotatef(-vCamera.rot.x, 1, 0, 0);
	glRotatef(-vCamera.rot.z, 0, 0, 1);

	RotateCamera(hWnd);

	glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 1.0f, 1.0f);
		for (const auto t : objFile.tris)
		{
			glVertex3f(t.v[0].x, t.v[0].y, t.v[0].z);
			glVertex3f(t.v[1].x, t.v[1].y, t.v[1].z);
			glVertex3f(t.v[2].x, t.v[2].y, t.v[2].z);
		}
	glEnd();

	if (keys[VK_LEFT].bPressed)
		vCamera.pos.x += 0.1f;

	if (keys[VK_RIGHT].bPressed)
		vCamera.pos.x -= 0.1f;

	if (keys[VK_UP].bPressed)
		vCamera.pos.y -= 0.1f;

	if (keys[VK_DOWN].bPressed)
		vCamera.pos.y += 0.1f;

	if (keys[L'W'].bPressed)
		vCamera.pos.z += 0.1f;

	if (keys[L'S'].bPressed)
		vCamera.pos.z -= 0.1f;
	
	return true;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
	PIXELFORMATDESCRIPTOR pfd;

	/* get the device context (DC) */
	*hDC = GetDC(hwnd);

	/* set the pixel format for the DC */
	ZeroMemory(&pfd, sizeof(pfd));

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;

	SetPixelFormat(*hDC, ChoosePixelFormat(*hDC, &pfd), &pfd);

	/* create and enable the render context (RC) */
	*hRC = wglCreateContext(*hDC);

	wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hwnd, hDC);
}

int main(HINSTANCE, HINSTANCE, LPSTR, INT)
{
	std::wstring sTitle = L"Example";

	bool bFullScreen = false;

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;

	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);

	wc.hIcon = LoadIcon(HInstance(), nullptr);

	wc.lpszClassName = sTitle.c_str();
	wc.lpszMenuName = nullptr;

	wc.hInstance = HInstance();

	wc.lpfnWndProc = WinProc;

	RegisterClass(&wc);

	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_THICKFRAME;

	HDC hDC;
	HGLRC hRC;

	float fTopLeftX = CW_USEDEFAULT;
	float fTopLeftY = CW_USEDEFAULT;

	HWND hWnd = nullptr;

	if (bFullScreen)
	{
		dwExStyle = 0;
		dwStyle = WS_VISIBLE | WS_POPUP;

		HMONITOR hmon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };

		if (!GetMonitorInfo(hmon, &mi)) return 1;

		nScreenWidth = mi.rcMonitor.right;
		nScreenHeight = mi.rcMonitor.bottom;

		fTopLeftX = 0.0f;
		fTopLeftY = 0.0f;
	}

	hWnd = CreateWindowEx(dwExStyle, sTitle.c_str(), sTitle.c_str(), dwStyle,
		fTopLeftX, fTopLeftY, nScreenWidth, nScreenHeight, nullptr, nullptr, HInstance(), nullptr);

	if (!hWnd)
	{
		MessageBox(0, L"Failed To Create Window!", 0, 0);
		return false;
	}

	ShowWindow(hWnd, SW_SHOW);
	ShowWindow(GetConsoleWindow(), SW_SHOW);

	EnableOpenGL(hWnd, &hDC, &hRC);

	glEnable(GL_DEPTH_TEST);

	//glOrtho(0, nScreenWidth, nScreenHeight, 0, 0, 1);
	glFrustum(-1, 1, -1, 1, 2, 80);

	if (!OnUserCreate())
		return true;

	for (int i = 0; i < 5; i++)
		mouse[0] = { false, false, true, 0 };

	for (int i = 0; i < 256; i++)
		keys[i] = { false, true };

	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glPushMatrix();

			SetWindowText(hWnd, LPWSTR(sTitle.c_str()));

			if (!OnUserUpdate(&hWnd))
				return 0;

			glPopMatrix();

			SwapBuffers(hDC);

			Sleep(1);
		}
	}

	return 0;
}
