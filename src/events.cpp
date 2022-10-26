#pragma once
#include "main.h"
#include "structs.cpp"

static void PollEvents()
{
    keys *Keys = &G->Keys;

    Keys->MouseRightOnce = false;
    Keys->MouseLeftOnce = false;
    Keys->MouseLeftUp = false;
    Keys->MouseRightUp = false;
    Keys->MouseMiddleOnce = false;
    Keys->ScrollYdiff = 0;
    Keys->MouseLeft_Click = false;
    Keys->F1_Key = false;
    Keys->F2_Key = false;
    Keys->F3_Key = false;
    Keys->F4_Key = false;
    Keys->F5_Key = false;
    Keys->F6_Key = false;
    Keys->F7_Key = false;
    Keys->F8_Key = false;
    Keys->F9_Key = false;
    Keys->F10_Key = false;
    Keys->F11_Key = false;
    Keys->F12_Key = false;
    Keys->Backquote_Key = false;

    // Keys->UpButton = false;
    // Keys->DownButton = false;
    // Keys->LeftButton = false;
    // Keys->RightButton = false;

    // Keys->A_Key = false;
    // Keys->S_Key = false;
    // Keys->D_Key = false;
    // Keys->W_Key = false;

    Keys->B_Key = false;
    Keys->C_Key = false;
    Keys->E_Key = false;
    Keys->F_Key = false;
    Keys->G_Key = false;
    Keys->H_Key = false;
    Keys->I_Key = false;
    Keys->J_Key = false;
    Keys->K_Key = false;
    Keys->L_Key = false;
    Keys->M_Key = false;
    Keys->N_Key = false;
    Keys->O_Key = false;
    Keys->P_Key = false;
    Keys->Q_Key = false;
    Keys->R_Key = false;
    Keys->T_Key = false;
    Keys->U_Key = false;
    Keys->V_Key = false;
    Keys->X_Key = false;
    Keys->Y_Key = false;
    Keys->Z_Key = false;
    Keys->_0_Key = false;
    Keys->_1_Key = false;
    Keys->_2_Key = false;
    Keys->_3_Key = false;
    Keys->_4_Key = false;
    Keys->_5_Key = false;
    Keys->_6_Key = false;
    Keys->_7_Key = false;
    Keys->_8_Key = false;
    Keys->_9_Key = false;
    Keys->Quote_Key = false;
    Keys->Colon_Key = false;
    Keys->Comma_Key = false;
    Keys->Dot_Key = false;
    Keys->FSlash_Key = false;
    Keys->BSlash_Key = false;
    Keys->Return_Key = false;
    Keys->Minus_Key = false;
    Keys->Plus_Key = false;
    Keys->Equals_Key = false;
    Keys->Backspace_Key = false;
    // Keys->Space_Key = false;
    Keys->LBracket_Key = false;
    Keys->RBracket_Key = false;
    Keys->Delete_Key = false;
    Keys->Esc_Key = false;
    Keys->Enter = false;

    if (Keys->MouseLeft)
    {
        Keys->MouseLeftAlready = true;
    }
    if (Keys->MouseRight)
    {
        Keys->MouseRightAlready = true;
    }
    // cout << Keys->MouseLeftAlready << " ";
    SDL_GetRelativeMouseState(&Keys->xrel, &Keys->yrel);
    SDL_Event Ev = {0};
    SDL_Event *Event = &Ev;

    while (1)
    {
        if (G->GIF)
        {
            if (!SDL_PollEvent(Event))
                break;
        }
        else if (!SDL_WaitEvent(Event))
            break;
            
        bool should_update = ImGui_ImplSDL2_ProcessEvent(Event);
        if (G->GIF)
            should_update = true;

        // SDL_GetMouseState(&Keys->Mouse.x, &Keys->Mouse.y);
        SDL_GetMouseState(&Keys->Mouse.x, &Keys->Mouse.y);
        SDL_GetRelativeMouseState(&G->Keys.xrel, &G->Keys.yrel);

        if (Event->type == SDL_KEYDOWN)
        {
            if (Event->key.keysym.sym == SDLK_SPACE && Event->key.repeat == false)
            {
                Keys->Space_Key = true;
            }
            if (Event->key.keysym.sym == SDLK_TAB && Event->key.repeat == false)
            {
                Keys->Tab_Key = true;
            }
        }

        if (Event->type == SDL_KEYUP)
        {
            if (Event->key.keysym.sym == SDLK_SPACE)
            {
                Keys->Space_Key = false;
            }
        }

        if (Event->type == SDL_QUIT)
        {
            Running = false;
        }
        should_update |= Event->type == SDL_KEYDOWN;
        should_update |= Event->type == SDL_KEYUP;
        should_update |= Event->type == SDL_QUIT;

        // Mouse Events:
        {
            should_update |= Event->type == SDL_MOUSEMOTION;
            should_update |= Event->type == SDL_MOUSEBUTTONDOWN;
            should_update |= Event->type == SDL_MOUSEBUTTONUP;
            should_update |= Event->type == SDL_MOUSEWHEEL;

            if (Event->button.clicks == 1)
            {
                Keys->MouseLeft_Click = true;
            }
            if (Event->type == SDL_MOUSEBUTTONDOWN)
            {
                if (Event->button.button == SDL_BUTTON_RIGHT && Event->button.state == SDL_PRESSED)
                {

                    Keys->MouseRight = true;
                    Keys->MouseRightOnce = true;
                }
            }
            if (Event->type == SDL_MOUSEBUTTONUP && Event->button.button == SDL_BUTTON_RIGHT)
            {
                Keys->MouseRight = false;
                Keys->MouseRightUp = true;
                Keys->MouseRightAlready = false;
            }
            if (Event->type == SDL_MOUSEBUTTONDOWN)
            {
                if (Event->button.button == SDL_BUTTON_LEFT && Event->button.state == SDL_PRESSED)
                {

                    Keys->MouseLeft = true;
                    Keys->MouseLeftReleased = false;
                    Keys->MouseLeftOnce = true;
                }
            }
            if (Event->type == SDL_MOUSEBUTTONUP && Event->button.button == SDL_BUTTON_LEFT)
            {
                Keys->MouseLeft = false;
                Keys->MouseLeftUp = true;
                Keys->MouseLeftReleased = true;
                Keys->MouseLeftAlready = false;
            }
            if (Event->type == SDL_MOUSEBUTTONDOWN)
            {
                if (Event->button.button == SDL_BUTTON_MIDDLE && Event->button.state == SDL_PRESSED)
                {
                    Keys->MouseMiddle = true;
                    Keys->MouseMiddleOnce = true;
                }
            }
            if (Event->type == SDL_BUTTON_MIDDLE && Event->button.button == SDL_BUTTON_LEFT)
            {
                Keys->MouseMiddle = false;
            }
            if (Event->type == SDL_MOUSEWHEEL)
            {
                if (Event->wheel.y > 0) // scroll up
                {
                    Keys->ScrollY++;
                    Keys->ScrollYdiff++;
                }
                else if (Event->wheel.y < 0) // scroll down
                {
                    Keys->ScrollY--;
                    Keys->ScrollYdiff--;
                }
                // if (DEBUG_MODE)
                //     G->Camera.Scale *= (1.0f + Event->wheel.y * 0.05f);
            }
        }
        if (Event->type == SDL_KEYDOWN)
        {
            if (Event->key.repeat == false)
                switch (Event->key.keysym.sym) // press and hold once only
                {
                case SDLK_ESCAPE:
                    Keys->Esc_Key = true;
                    break;
                case SDLK_RETURN:
                    Keys->Enter = true;
                    break;
                case SDLK_KP_ENTER:
                    Keys->Enter = true;
                    break;
                case SDLK_F1:
                    Keys->F1_Key = true;
                    break;
                case SDLK_F2:
                    Keys->F2_Key = true;
                    break;
                case SDLK_F3:
                    Keys->F3_Key = true;
                    break;
                case SDLK_F4:
                    Keys->F4_Key = true;
                    break;
                case SDLK_F5:
                    Keys->F5_Key = true;
                    break;
                case SDLK_F6:
                    Keys->F6_Key = true;
                    break;
                case SDLK_F7:
                    Keys->F7_Key = true;
                    break;
                case SDLK_F8:
                    Keys->F8_Key = true;
                    break;
                case SDLK_F9:
                    Keys->F9_Key = true;
                    break;
                case SDLK_F10:
                    Keys->F10_Key = true;
                    break;
                case SDLK_F11:
                    Keys->F11_Key = true;
                    break;
                case SDLK_F12:
                    Keys->F12_Key = true;
                    break;
                case SDLK_BACKQUOTE:
                    Keys->Backquote_Key = true;
                    break;
                }
            bool KeyState = Event->type == SDL_KEYDOWN ? true : false;
            switch (Event->key.keysym.sym) // press and hold type style, continues after a period
            {
            case SDLK_BACKSPACE:
                Keys->Backspace_Key = true;
                break;
            case SDLK_DELETE:
                Keys->Delete_Key = KeyState;
                break;
            case SDLK_a:
                Keys->A_Key = KeyState;
                break;
            case SDLK_b:
                Keys->B_Key = KeyState;
                break;
            case SDLK_c:
                Keys->C_Key = KeyState;
                break;
            case SDLK_d:
                Keys->D_Key = KeyState;
                break;
            case SDLK_e:
                Keys->E_Key = KeyState;
                break;
            case SDLK_f:
                Keys->F_Key = KeyState;
                break;
            case SDLK_g:
                Keys->G_Key = KeyState;
                break;
            case SDLK_h:
                Keys->H_Key = KeyState;
                break;
            case SDLK_i:
                Keys->I_Key = KeyState;
                break;
            case SDLK_j:
                Keys->J_Key = KeyState;
                break;
            case SDLK_k:
                Keys->K_Key = KeyState;
                break;
            case SDLK_l:
                Keys->L_Key = KeyState;
                break;
            case SDLK_m:
                Keys->M_Key = KeyState;
                break;
            case SDLK_n:
                Keys->N_Key = KeyState;
                break;
            case SDLK_o:
                Keys->O_Key = KeyState;
                break;
            case SDLK_p:
                Keys->P_Key = KeyState;
                break;
            case SDLK_q:
                Keys->Q_Key = KeyState;
                break;
            case SDLK_r:
                Keys->R_Key = KeyState;
                break;
            case SDLK_s:
                Keys->S_Key = KeyState;
                break;
            case SDLK_t:
                Keys->T_Key = KeyState;
                break;
            case SDLK_u:
                Keys->U_Key = KeyState;
                break;
            case SDLK_v:
                Keys->V_Key = KeyState;
                break;
            case SDLK_w:
                Keys->W_Key = KeyState;
                break;
            case SDLK_x:
                Keys->X_Key = KeyState;
                break;
            case SDLK_y:
                Keys->Y_Key = KeyState;
                break;
            case SDLK_z:
                Keys->Z_Key = KeyState;
                break;
            case SDLK_0:
                Keys->_0_Key = KeyState;
                break;
            case SDLK_1:
                Keys->_1_Key = KeyState;
                break;
            case SDLK_2:
                Keys->_2_Key = KeyState;
                break;
            case SDLK_3:
                Keys->_3_Key = KeyState;
                break;
            case SDLK_4:
                Keys->_4_Key = KeyState;
                break;
            case SDLK_5:
                Keys->_5_Key = KeyState;
                break;
            case SDLK_6:
                Keys->_6_Key = KeyState;
                break;
            case SDLK_7:
                Keys->_7_Key = KeyState;
                break;
            case SDLK_8:
                Keys->_8_Key = KeyState;
                break;
            case SDLK_9:
                Keys->_9_Key = KeyState;
                break;
            case SDLK_KP_0:
                Keys->_0_Key = KeyState;
                break;
            case SDLK_KP_1:
                Keys->_1_Key = KeyState;
                break;
            case SDLK_KP_2:
                Keys->_2_Key = KeyState;
                break;
            case SDLK_KP_3:
                Keys->_3_Key = KeyState;
                break;
            case SDLK_KP_4:
                Keys->_4_Key = KeyState;
                break;
            case SDLK_KP_5:
                Keys->_5_Key = KeyState;
                break;
            case SDLK_KP_6:
                Keys->_6_Key = KeyState;
                break;
            case SDLK_KP_7:
                Keys->_7_Key = KeyState;
                break;
            case SDLK_KP_8:
                Keys->_8_Key = KeyState;
                break;
            case SDLK_KP_9:
                Keys->_9_Key = KeyState;
                break;
            case SDLK_QUOTE:
                Keys->Quote_Key = KeyState;
                break;
            case SDLK_COLON:
                Keys->Colon_Key = KeyState;
                break;
            case SDLK_COMMA:
                Keys->Comma_Key = KeyState;
                break;
            case SDLK_PERIOD:
                Keys->Dot_Key = KeyState;
                break;
            case SDLK_SLASH:
                Keys->FSlash_Key = KeyState;
                break;
            case SDLK_BACKSLASH:
                Keys->BSlash_Key = KeyState;
                break;
            case SDLK_RETURN:
                Keys->Return_Key = KeyState;
                break;
            case SDLK_KP_ENTER:
                Keys->Return_Key = KeyState;
                break;
            case SDLK_MINUS:
                Keys->Minus_Key = KeyState;
                break;
            case SDLK_KP_MINUS:
                Keys->Minus_Key = KeyState;
                break;
            case SDLK_KP_PLUS:
                Keys->Plus_Key = KeyState;
                break;
            case SDLK_EQUALS:
                Keys->Equals_Key = KeyState;
                break;
            case SDLK_LEFTBRACKET:
                Keys->LBracket_Key = KeyState;
                break;
            case SDLK_RIGHTBRACKET:
                Keys->RBracket_Key = KeyState;
                break;
            }
        }
        if (Event->type == SDL_KEYDOWN || Event->type == SDL_KEYUP)
        {
            bool KeyState = Event->type == SDL_KEYDOWN ? true : false;

            switch (Event->key.keysym.sym)
            {
            case SDLK_UP:
                Keys->UpButton = KeyState;
                break;
            case SDLK_DOWN:
                Keys->DownButton = KeyState;
                break;
            case SDLK_LEFT:
                Keys->LeftButton = KeyState;
                break;
            case SDLK_RIGHT:
                Keys->RightButton = KeyState;
                break;
            case SDLK_LSHIFT:
                Keys->Shift = KeyState;
                break;
            case SDLK_RSHIFT:
                Keys->Shift = KeyState;
                break;
            case SDLK_LCTRL:
                Keys->Ctrl_Key = KeyState;
                break;
            case SDLK_RCTRL:
                Keys->Ctrl_Key = KeyState;
                break;
            case SDLK_LALT:
                Keys->Alt_Key = KeyState;
                break;
            case SDLK_RALT:
                Keys->Alt_Key = KeyState;
                break;
            case SDLK_h:
                Keys->H_Key = KeyState;
                break;
            case SDLK_w:
                Keys->W_Key = KeyState;
                break;
            case SDLK_a:
                Keys->A_Key = KeyState;
                break;
            case SDLK_s:
                Keys->S_Key = KeyState;
                break;
            case SDLK_d:
                Keys->D_Key = KeyState;
                break;
            default:
                break;
            }
        }
        if (should_update)
            break;
    }
}