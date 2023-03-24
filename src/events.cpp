#pragma once
#include "main.h"
#include "structs.cpp"


void ResetInputs()
{
    keys *K = &G->Keys;
    for (int i = 0; i < key_COUNT; i++)
    {
        K->K[i].up = false;
        K->K[i].dn = false;
    }
    K->ScrollYdiff = 0;
}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    keys *K = &G->Keys;

    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;
    switch (message)
    {
    case WM_DESTROY:    PostQuitMessage(0); Running = false; break;
    case WM_DROPFILES: 
    {
        if (!G->Loading_Droppedfile)
        {
            UINT length = DragQueryFile((HDROP)wParam, /*only first one is handled*/ 0, NULL, 0);
            TempPath = (wchar_t *)malloc((length + 1) * sizeof(wchar_t));
            DragQueryFileW((HDROP)wParam, /*only first one is handled*/ 0, TempPath, length + 1);
            int i = 0;
        }
        G->Droppedfile = true;
        break;
    }
    case WM_MOUSEMOVE:
    {
        int xPos = LOWORD(lParam);
        int yPos = HIWORD(lParam);
        K->Mouse.x  = xPos; K->Mouse.y  = yPos;
        break;
    }
    case WM_MOUSEWHEEL:
    {
        int scrollValue = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        K->ScrollYdiff = scrollValue;
        K->ScrollY    += scrollValue;
        break;
    }
    case WM_LBUTTONDOWN:    { K->K[MouseL].dn = true; K->K[MouseL].on = true;  break; }
    case WM_RBUTTONDOWN:    { K->K[MouseR].dn = true; K->K[MouseR].on = true;  break; }                                                                
    case WM_MBUTTONDOWN:    { K->K[MouseM].dn = true; K->K[MouseM].on = true;  break; }                                                            
    case WM_LBUTTONUP:      { K->K[MouseL].up = true; K->K[MouseL].on = false; break; }
    case WM_RBUTTONUP:      { K->K[MouseR].up = true; K->K[MouseR].on = false; break; }                                                                  
    case WM_MBUTTONUP:      { K->K[MouseM].up = true; K->K[MouseM].on = false; break; }   
    case WM_KEYUP:
    case WM_KEYDOWN:
    {
        int i = 0;
        switch(wParam)
        {
            // case VK_LBUTTON: i =  Key_Left;          // 	Left mouse button
            // case VK_RBUTTON: i = Key_Right;          // 	Right mouse button
            case VK_XBUTTON1:   i = MouseX1;        break; // 	X1 mouse button
            case VK_XBUTTON2:   i = MouseX2;        break; // 	X2 mouse button
            case VK_BACK:       i = Key_Backspace;  break; // 	BACKSPACE key
            case VK_TAB:        i = Key_Tab;        break; // 	TAB key
            case VK_RETURN:     i = Key_Enter;      break; // 	ENTER key
            case VK_SHIFT:      i = Key_Shift;      break; // 	SHIFT key
            case VK_CONTROL:    i = Key_Ctrl;       break; // 	CTRL key
            case VK_MENU:       i = Key_Alt;        break; // 	ALT key
            case VK_PAUSE:      i = Key_Pause;      break; // 	PAUSE key
            case VK_CAPITAL:    i = Key_CapsLock;   break; // 	CAPS LOCK key
            case VK_ESCAPE:     i = Key_Esc;        break; // 	ESC key
            case VK_SPACE:      i = Key_Space;      break; // 	SPACEBAR
            case VK_PRIOR:      i = Key_PgUp;       break; // 	PAGE UP key
            case VK_NEXT:       i = Key_PgDn;       break; // 	PAGE DOWN key
            case VK_END:        i = Key_End;        break; // 	END key
            case VK_HOME:       i = Key_Home;       break; // 	HOME key
            case VK_LEFT:       i = Key_Left;       break; // 	LEFT ARROW key
            case VK_UP:         i = Key_Up;         break; // 	UP ARROW key
            case VK_RIGHT:      i = Key_Right;      break; // 	RIGHT ARROW key
            case VK_DOWN:       i = Key_Down;       break; // 	DOWN ARROW key
            case VK_SELECT:     i = Key_Select;     break; // 	SELECT key
            case VK_PRINT:      i = Key_Print;      break; // 	PRINT key
            case VK_EXECUTE:    i = Key_Enter;      break; // 	EXECUTE key
            case VK_SNAPSHOT:   i = Key_Print;      break; // 	PRINT SCREEN key
            case VK_INSERT:     i = Key_Insert;     break; // 	INS key
            case VK_DELETE:     i = Key_Delete;     break; // 	DEL key
            case '0':           i = Key_0;          break; // 	0 key
            case '1':           i = Key_1;          break; // 	1 key
            case '2':           i = Key_2;          break; // 	2 key
            case '3':           i = Key_3;          break; // 	3 key
            case '4':           i = Key_4;          break; // 	4 key
            case '5':           i = Key_5;          break; // 	5 key
            case '6':           i = Key_6;          break; // 	6 key
            case '7':           i = Key_7;          break; // 	7 key
            case '8':           i = Key_8;          break; // 	8 key
            case '9':           i = Key_9;          break; // 	9 key
            case 'A':           i = Key_A;          break; // 	A key
            case 'B':           i = Key_B;          break; // 	B key
            case 'C':           i = Key_C;          break; // 	C key
            case 'D':           i = Key_D;          break; // 	D key
            case 'E':           i = Key_E;          break; // 	E key
            case 'F':           i = Key_F;          break; // 	F key
            case 'G':           i = Key_G;          break; // 	G key
            case 'H':           i = Key_H;          break; // 	H key
            case 'I':           i = Key_I;          break; // 	I key
            case 'J':           i = Key_J;          break; // 	J key
            case 'K':           i = Key_K;          break; // 	K key
            case 'L':           i = Key_L;          break; // 	L key
            case 'M':           i = Key_M;          break; // 	M key
            case 'N':           i = Key_N;          break; // 	N key
            case 'O':           i = Key_O;          break; // 	O key
            case 'P':           i = Key_P;          break; // 	P key
            case 'Q':           i = Key_Q;          break; // 	Q key
            case 'R':           i = Key_R;          break; // 	R key
            case 'S':           i = Key_S;          break; // 	S key
            case 'T':           i = Key_T;          break; // 	T key
            case 'U':           i = Key_U;          break; // 	U key
            case 'V':           i = Key_V;          break; // 	V key
            case 'W':           i = Key_W;          break; // 	W key
            case 'X':           i = Key_X;          break; // 	X key
            case 'Y':           i = Key_Y;          break; // 	Y key
            case 'Z':           i = Key_Z;          break; // 	Z key
            case VK_LWIN:       i = Key_LWIN;       break; // 	Left Windows key (Natural keyboard)
            case VK_RWIN:       i = Key_RWIN;       break; // 	Right Windows key (Natural keyboard)
            case VK_NUMPAD0:    i = Key_num_0;      break; // 	Numeric keypad 0 key
            case VK_NUMPAD1:    i = Key_num_1;      break; // 	Numeric keypad 1 key
            case VK_NUMPAD2:    i = Key_num_2;      break; // 	Numeric keypad 2 key
            case VK_NUMPAD3:    i = Key_num_3;      break; // 	Numeric keypad 3 key
            case VK_NUMPAD4:    i = Key_num_4;      break; // 	Numeric keypad 4 key
            case VK_NUMPAD5:    i = Key_num_5;      break; // 	Numeric keypad 5 key
            case VK_NUMPAD6:    i = Key_num_6;      break; // 	Numeric keypad 6 key
            case VK_NUMPAD7:    i = Key_num_7;      break; // 	Numeric keypad 7 key
            case VK_NUMPAD8:    i = Key_num_8;      break; // 	Numeric keypad 8 key
            case VK_NUMPAD9:    i = Key_num_9;      break; // 	Numeric keypad 9 key
            case VK_MULTIPLY:   i = Key_Multiply;   break; // 	Multiply key
            case VK_ADD:        i = Key_Plus;       break; // 	Add key
            case VK_SEPARATOR:  i = Key_BSlash;     break; // 	Separator key
            case VK_SUBTRACT:   i = Key_Minus;      break; // 	Subtract key
            case VK_DECIMAL:    i = Key_Comma;      break; // 	Decimal key
            case VK_DIVIDE:     i = Key_FSlash;     break; // 	Divide key
            case VK_F1:         i = Key_F1;         break; // 	F1 key
            case VK_F2:         i = Key_F2;         break; // 	F2 key
            case VK_F3:         i = Key_F3;         break; // 	F3 key
            case VK_F4:         i = Key_F4;         break; // 	F4 key
            case VK_F5:         i = Key_F5;         break; // 	F5 key
            case VK_F6:         i = Key_F6;         break; // 	F6 key
            case VK_F7:         i = Key_F7;         break; // 	F7 key
            case VK_F8:         i = Key_F8;         break; // 	F8 key
            case VK_F9:         i = Key_F9;         break; // 	F9 key
            case VK_F10:        i = Key_F10;        break; // 	F10 key
            case VK_F11:        i = Key_F11;        break; // 	F11 key
            case VK_F12:        i = Key_F12;        break; // 	F12 key
            case VK_NUMLOCK:    i = Key_NumLock;    break; // 	NUM LOCK key
            case VK_SCROLL:     i = Key_ScrollLock; break; // 	SCROLL LOCK key
            case VK_LSHIFT:     i = Key_LShift;     break; // 	Left SHIFT key
            case VK_RSHIFT:     i = Key_RShift;     break; // 	Right SHIFT key
            case VK_LCONTROL:   i = Key_LCtrl;      break; // 	Left CONTROL key
            case VK_RCONTROL:   i = Key_RCtrl;      break; // 	Right CONTROL key
            case VK_LMENU:      i = Key_LAlt;       break; // 	Left ALT key
            case VK_RMENU:      i = Key_RAlt;       break; // 	Right ALT key
            case VK_OEM_PLUS:   i = Key_Plus;       break; // 	For any country/region, the '+' key
            case VK_OEM_COMMA:  i = Key_Comma;      break; // 	For any country/region, the ',' key
            case VK_OEM_MINUS:  i = Key_Minus;      break; // 	For any country/region, the '-' key
            case VK_OEM_PERIOD: i = Key_Dot;        break; // 	For any country/region, the '.' key
            case VK_OEM_2:      i = Key_FSlash;     break; // 	Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
            case VK_OEM_3:      i = Key_Backquote;  break; // 	Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
            case VK_OEM_4:      i = Key_LBracket;   break; // 	Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
            case VK_OEM_5:      i = Key_BSlash;     break; // 	Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
            case VK_OEM_6:      i = Key_RBracket;   break; // 	Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
            case VK_OEM_7:      i = Key_Colon;      break; // 	Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
            // case VK_OEM_102:                         // 	The <> keys on the US standard keyboard, or the \\| key on the non-US 102-key keyboard
        }
        if (message == WM_KEYDOWN) { K->K[i].dn = true; K->K[i].on = true; }
        else                       { K->K[i].up = true; K->K[i].on = false;}
        if (wParam == VK_LCONTROL || wParam == VK_RCONTROL) K->K[Key_Ctrl]  = K->K[i];
        if (wParam == VK_LSHIFT   || wParam == VK_RSHIFT)   K->K[Key_Shift] = K->K[i];
        if (wParam == VK_LMENU    || wParam == VK_RMENU)    K->K[Key_Alt]   = K->K[i];
        break;
    }                                                          
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

void WIN_DPI_Point(LONG *xOut, LONG *yOut)
{
    *xOut = (*xOut * 96) / GetDpiForWindow(hwnd);
    *yOut = (*yOut * 96) / GetDpiForWindow(hwnd);
}

static void PollEvents()
{
    static MSG msg = {};
    keys *K = &G->Keys;
    while (PeekMessage(&msg, hwnd, 0, 0, PM_NOREMOVE))
    {
        GetMessage(&msg, nullptr, 0, 0);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT) Running = false;
    }

    static POINT prevMousePos = {0};
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(hwnd, &p);
    K->Mouse.x  = p.x; K->Mouse.y  = p.y;
    K->Mouse_rel.x = p.x - prevMousePos.x; 
    K->Mouse_rel.y = p.y - prevMousePos.y; 
    prevMousePos.x = p.x; prevMousePos.y = p.y;
}

inline bool keypress(int i)
{
    return G->Keys.K[i].on;
}
inline bool keyup(int i)
{
    return G->Keys.K[i].up;
}
inline bool keydn(int i)
{
    return G->Keys.K[i].dn;
}
inline void keyrelease(int i)
{
    G->Keys.K[i].on = false;
}