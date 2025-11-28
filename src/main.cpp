#include "main.h"
#include "source.cpp"

int wmain(int argc, wchar_t **argv) {

#ifdef _WIN32
#include <fileapi.h>
    char *appdata = getenv("APPDATA");
    APPDATA_FOLDER = (char *)malloc(strlen(appdata) + 15);
    sprintf(APPDATA_FOLDER, "%s\\CactusViewer", appdata);
    CreateDirectoryA(APPDATA_FOLDER, NULL);
    
    // Store exe parent folder.
    wchar_t exe_path[256];
    GetModuleFileNameW(0, exe_path, sizeof(exe_path));
    wchar_t *one_past_slash = exe_path;
    wchar_t *exe            = exe_path;
    while (*exe)  {
        if (*exe++ == L'\\') one_past_slash = exe;
    }
    memmove(EXE_FOLDER, exe_path, (one_past_slash - 1 - exe_path) * sizeof(wchar_t));
    
    // This tells us where user executed CactusViewer.exe from.
    GetCurrentDirectoryW(sizeof(CURRENT_FOLDER), CURRENT_FOLDER);
#else
    APPDATA_FOLDER = "./";
    // TODO(): Store exe folder and current working directory for other platforms.
#endif
	Loader_Thread_Inputs inputs;

	G->main_loop_fiber =  ConvertThreadToFiber(NULL);
	G->message_loop_fiber = CreateFiber(0, poll_events, NULL);

    init_all();
	if (G->settings_start_in_fullscreen)
		enter_fullscreen(hwnd);
	{
		// Use immediate loading for startup - load the image first, then scan folder in background
		if (argc > 1 && !PathIsDirectoryW(argv[1])) {
			if (!load_image_immediate(argv[1])) {
				push_alert("File type not supported");
			}
		} else {
			// Directory or no argument - use the old flow
			int scan = scan_folder(argv[1]);
			if (argc > 1 && scan != SCAN_FAILED) {
				inputs = { argv[1], G->current_file_index, &G->files[G->current_file_index] };
				CreateThread(NULL, 0, loader_thread, (LPVOID) & inputs, 0, NULL);
			}
		}
	}

    while (Running) {
        bool gifmode = false;
		if (G->files.Count > 0) gifmode = G->files[G->current_file_index].type == TYPE_GIF || G->files[G->current_file_index].type == TYPE_WEBP_ANIM;
        mouse_detection = WH - 140 - 60 * (gifmode) - (3 + THUMBS_DIM) * G->settings_preview_thumbs;
		SwitchToFiber(G->message_loop_fiber);
		UI_begin_frame(G->ui, 60);
		UI_check_mouse();
        G->show_gui = should_show_gui();
		G->ui_mouse_hit_test = false;

        if (G->dropped_file) {
            G->loading_dropped_file = true;
			// Use immediate loading for dropped files - load image first, scan folder in background
			if (!PathIsDirectoryW(global_temp_path)) {
				if (!load_image_immediate(global_temp_path)) {
					push_alert("File type not supported");
					G->loading_dropped_file = false;
				}
			} else {
				// Directory dropped - use old flow
				int scan = scan_folder(global_temp_path);
				if (scan != SCAN_FAILED) {
					G->loaded = false;
					inputs = { G->files[0].file.path, 0, &G->files[0], true };
					CreateThread(NULL, 0, loader_thread, (LPVOID) & inputs, 0, NULL);
				}
			}
			G->dropped_file = false;
        } 

        get_window_size();

		// Start deferred folder scan once the immediate load completes
		if (G->pending_folder_scan && G->loaded) {
			G->pending_folder_scan = false;
			start_deferred_folder_scan(G->pending_folder_scan_path);
		}

        update_gui();
		update_logic();
        render();

        if (G->files.Count > 0) {
            if ((!G->sorting && !G->scanning_folder && (keyup(Key_Right) || keyup(MouseFr))) || G->signals.next_image) {
                G->signals.next_image = false;

                if (G->current_file_index < G->files.Count - 1) {
                    G->current_file_index++;
                    G->loaded = false;
                    inputs = {G->files[G->current_file_index].file.path, G->current_file_index, &G->files[G->current_file_index], false};
                    CreateThread(NULL, 0, loader_thread, (LPVOID)&inputs, 0, NULL);
                }
            }
            if ((!G->sorting && !G->scanning_folder && (keyup(Key_Left) || keyup(MouseBk)))|| G->signals.prev_image) {
                G->signals.prev_image = false;

                if (G->current_file_index > 0) {
                    G->current_file_index--;
                    G->loaded = false;
					inputs = {G->files[G->current_file_index].file.path, G->current_file_index, &G->files[G->current_file_index], false};
                    CreateThread(NULL, 0, loader_thread, (LPVOID)&inputs, 0, NULL);
                }
            }
			if (G->signals.reload_file) {
				G->current_file_index = G->req_file_index;
				G->signals.reload_file = false;
				inputs = {G->files[G->current_file_index].file.path, G->current_file_index, &G->files[G->current_file_index], false};
				CreateThread(NULL, 0, loader_thread, (LPVOID)&inputs, 0, NULL);
			}
        }
        reset_inputs();
		if (keyup(MouseL))
			G->mouse_dn_hash = 0;
    }
    save_settings();
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    if(AttachConsole (ATTACH_PARENT_PROCESS) != 0) {
        FILE *  fpstdin = stdin, *fpstdout = stdout, *fpstderr = stderr;  
        freopen_s (&fpstdin,  "CONIN$",  "r", stdin);  
        freopen_s (&fpstdout, "CONOUT$", "w", stdout);  
        freopen_s (&fpstderr, "CONOUT$", "w", stderr);  
    } 
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);
    int result = wmain(argc, argv);
    return result;
}