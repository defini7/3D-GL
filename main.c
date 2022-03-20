#include <windows.h>
#include <gl/gl.h>
#include <math.h>

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

float fCube[] = { 0,0,0, 0,1,0, 1,1,0, 1,0,0, 0,0,1, 0,1,1, 1,1,1, 1,0,1 };
GLuint nCubeVertices[] = { 0,1,2, 2,3,0, 4,5,6, 6,7,4, 3,2,5, 6,7,3, 0,1,5, 5,4,0, 1,2,6, 6,5,1, 0,3,7, 7,4,0 };

typedef struct {
    float r;
    float g;
    float b;
} TColor;

typedef struct {
    TColor col;
} TCell;

typedef struct {
    float x;
    float y;
    float z;
    float xRot;
    float zRot;
} TCamera;

TCamera camera = { 0.0f, 0.0f, 1.7f, 70.0f, -40.0f };

#define MAP_WIDTH 40
#define MAP_HEIGHT 40

TCell map[MAP_WIDTH * MAP_HEIGHT];

void SetCell(int x, int y, float r, float g, float b)
{
    TColor c;
    c.r = r;
    c.g = g;
    c.b = b;

    map[y * MAP_HEIGHT + x].col = c;
}

TColor GetCell(int x, int y)
{
    return map[y * MAP_HEIGHT + x].col;
}

void CameraApply()
{
    glRotatef(-camera.xRot, 1.0f, 0.0f, 0.0f);
    glRotatef(-camera.zRot, 0.0f, 0.0f, 1.0f);
    glTranslatef(-camera.x, -camera.y, -camera.z);
}

void CameraRotate(float xAlpha, float zAlpha)
{
    camera.zRot += zAlpha;
    camera.xRot += xAlpha;

    if (camera.zRot < 0.0f) camera.zRot += 360.0f;
    if (camera.zRot > 360.0f) camera.zRot -= 360.0f;

    if (camera.xRot < 0.0f) camera.xRot = 0.0f;
    if (camera.xRot > 180.0f) camera.xRot = 180.0f;
}

void PlayerMove(HWND* hwnd)
{
    if (GetForegroundWindow() != *hwnd) return;

    float fAngle = -camera.zRot / 180.0f * M_PI;
    float fSpeed = 0.0f;

    if (GetKeyState('W') < 0) fSpeed = 0.1f;
    if (GetKeyState('S') < 0) fSpeed = -0.1f;
    if (GetKeyState('A') < 0) { fSpeed = 0.1f; fAngle -= M_PI / 2.0f; }
    if (GetKeyState('D') < 0) { fSpeed = 0.1f; fAngle += M_PI / 2.0f; }

    if (fSpeed != 0.0f)
    {
        camera.x += sin(fAngle) * fSpeed;
        camera.y += cos(fAngle) * fSpeed;
    }

    POINT cursor;
    static POINT base = { 400, 300 };

    GetCursorPos(&cursor);
    CameraRotate((base.y - cursor.y) / 5.0f, (base.x - cursor.x) / 5.0f);
    SetCursorPos(base.x, base.y);
}

void MapInit()
{
    for (int i = 0; i < MAP_WIDTH; i++)
        for (int j = 0; j < MAP_HEIGHT; j++)
        {
            float dc = (rand() % 20) * 0.01f;

            SetCell(i, j, 0.31f + dc, 0.5f + dc, 0.13f + dc);
        }
}

void WndResize(double x, double y)
{
    glViewport(0, 0, x, y);
    double k = x / y;
    double sz = 0.1f;
    glLoadIdentity();
    glFrustum(-k*sz, k*sz, -sz, sz, sz*2.0, 100.0);
}

void GameInit(HWND* hwnd)
{
    glEnable(GL_DEPTH_TEST);
    MapInit();

    RECT rect;
    GetClientRect(hwnd, &rect);
    WndResize(rect.right, rect.bottom);
}

void GameMove(HWND* hwnd)
{
    PlayerMove(hwnd);
}

void GameShow()
{
    glClearColor(0.6f, 0.8f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
        CameraApply();
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, fCube);
        for (int i = 0; i < MAP_WIDTH; i++)
            for (int j = 0; j < MAP_HEIGHT; j++)
            {
                glPushMatrix();
                    glTranslatef(i, j, 0);
                    TColor c = GetCell(i, j);
                    glColor3f(c.r, c.g, c.b);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nCubeVertices);
                glPopMatrix();
            }

        glDisableClientState(GL_VERTEX_ARRAY);
    glPopMatrix();
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
    float theta = 0.0f;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "OpenGL Sample",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          1024,
                          768,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);

    GameInit(&hwnd);

    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */

            GameMove(&hwnd);
            GameShow();

            SwapBuffers(hDC);

            theta += 1.0f;
            Sleep (1);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_SIZE:
            WndResize(LOWORD(lParam), HIWORD(lParam));
        break;

        case WM_SETCURSOR:
            ShowCursor(FALSE);
        break;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

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

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}
