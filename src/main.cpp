#include "main.h"
#include "source.cpp"

int wmain(int argc, wchar_t **argv) {

#ifdef _WIN32
#include <fileapi.h>
    char *appdata = getenv("APPDATA");
    APPDATA_FOLDER = (char *)malloc(strlen(appdata) + 15);
    sprintf(APPDATA_FOLDER, "%s\\CactusViewer", appdata);
    CreateDirectoryA(APPDATA_FOLDER, NULL);
    
    // Initialize debug logging
    debug_log_init();
    DLOG_INIT("CactusViewer starting up");
    DLOG_INIT("APPDATA_FOLDER: %s", APPDATA_FOLDER);
    
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

	DLOG_INIT("Converting thread to fiber");
	G->main_loop_fiber =  ConvertThreadToFiber(NULL);
	G->message_loop_fiber = CreateFiber(0, poll_events, NULL);

    DLOG_INIT("Calling init_all()");
    init_all();
    DLOG_INIT("init_all() completed");
	if (G->settings_start_in_fullscreen) {
		DLOG_INIT("Entering fullscreen mode (from settings)");
		enter_fullscreen(hwnd);
	}
	{
		DLOG_INIT("Processing command line arguments, argc=%d", argc);
		// Use immediate loading for startup - load the image first, then scan folder in background
		if (argc > 1 && !PathIsDirectoryW(argv[1])) {
			DLOG_FILE("Attempting immediate load of file from argv[1]");
			debug_log_wstr("FILE", "Path", argv[1]);
			if (!load_image_immediate(argv[1])) {
				DLOG_ERROR("load_image_immediate failed - file type not supported");
				push_alert("File type not supported");
			} else {
				DLOG_FILE("load_image_immediate succeeded");
			}
		} else {
			// Directory or no argument - use the old flow
			DLOG_SCAN("Using old flow (directory or no argument)");
			int scan = scan_folder(argv[1]);
			DLOG_SCAN("scan_folder returned %d", scan);
			if (argc > 1 && scan != SCAN_FAILED) {
				DLOG_LOADER("Creating loader thread for initial file, index=%u", G->current_file_index);
				Loader_Thread_Inputs *heap_inputs = create_loader_inputs(argv[1], G->current_file_index, &G->files[G->current_file_index], false);
				CreateThread(NULL, 0, loader_thread, (LPVOID)heap_inputs, 0, NULL);
			}
		}
	}

    while (Running) {
        bool gifmode = false;
		if (G->files.Count > 0 && G->current_file_index < G->files.Count) gifmode = G->files[G->current_file_index].type == TYPE_GIF || G->files[G->current_file_index].type == TYPE_WEBP_ANIM;
        mouse_detection = WH - 170 - 60 * (gifmode) - (3 + THUMBS_DIM) * G->settings_preview_thumbs;
		SwitchToFiber(G->message_loop_fiber);
		UI_begin_frame(G->ui, 60);
		UI_check_mouse();
        G->show_gui = should_show_gui();
		G->ui_mouse_hit_test = false;

        if (G->dropped_file) {
            DLOG_FILE("Processing dropped file");
            debug_log_wstr("FILE", "Dropped path", global_temp_path);
            G->loading_dropped_file = true;
			// Use immediate loading for dropped files - load image first, scan folder in background
			if (!PathIsDirectoryW(global_temp_path)) {
				DLOG_FILE("Dropped item is a file, using immediate load");
				if (!load_image_immediate(global_temp_path)) {
					DLOG_ERROR("load_image_immediate failed for dropped file");
					push_alert("File type not supported");
					G->loading_dropped_file = false;
				} else {
					DLOG_FILE("Immediate load succeeded for dropped file");
				}
			} else {
				// Directory dropped - use old flow
				DLOG_SCAN("Dropped item is a directory, using scan_folder");
				int scan = scan_folder(global_temp_path);
				DLOG_SCAN("scan_folder returned %d", scan);
				if (scan != SCAN_FAILED) {
					G->loaded = false;
					DLOG_LOADER("Creating loader thread for first file in dropped directory");
					Loader_Thread_Inputs *heap_inputs = create_loader_inputs(G->files[0].file.path, 0, &G->files[0], true);
					CreateThread(NULL, 0, loader_thread, (LPVOID)heap_inputs, 0, NULL);
				}
			}
			G->dropped_file = false;
        } 

        get_window_size();

		// Start deferred folder scan once the immediate load completes
		if (G->pending_folder_scan && G->loaded) {
			DLOG_SCAN("Starting deferred folder scan");
			debug_log_wstr("SCAN", "Deferred scan path", G->pending_folder_scan_path);
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
                    DLOG_NAV("Navigating to NEXT image, current=%u, new=%u, total=%u", 
                             G->current_file_index, G->current_file_index + 1, G->files.Count);
                    G->current_file_index++;
                    G->loaded = false;
                    debug_log_wstr("NAV", "Loading file", G->files[G->current_file_index].file.path);
                    Loader_Thread_Inputs *heap_inputs = create_loader_inputs(G->files[G->current_file_index].file.path, G->current_file_index, &G->files[G->current_file_index], false);
                    CreateThread(NULL, 0, loader_thread, (LPVOID)heap_inputs, 0, NULL);
                }
            }
            if ((!G->sorting && !G->scanning_folder && (keyup(Key_Left) || keyup(MouseBk)))|| G->signals.prev_image) {
                G->signals.prev_image = false;

                if (G->current_file_index > 0) {
                    DLOG_NAV("Navigating to PREV image, current=%u, new=%u, total=%u", 
                             G->current_file_index, G->current_file_index - 1, G->files.Count);
                    G->current_file_index--;
                    G->loaded = false;
                    debug_log_wstr("NAV", "Loading file", G->files[G->current_file_index].file.path);
                    Loader_Thread_Inputs *heap_inputs = create_loader_inputs(G->files[G->current_file_index].file.path, G->current_file_index, &G->files[G->current_file_index], false);
                    CreateThread(NULL, 0, loader_thread, (LPVOID)heap_inputs, 0, NULL);
                }
            }
			if (G->signals.reload_file) {
				DLOG_NAV("Reload file signal, req_index=%u", G->req_file_index);
				G->current_file_index = G->req_file_index;
				G->signals.reload_file = false;
				debug_log_wstr("NAV", "Reloading file", G->files[G->current_file_index].file.path);
				Loader_Thread_Inputs *heap_inputs = create_loader_inputs(G->files[G->current_file_index].file.path, G->current_file_index, &G->files[G->current_file_index], false);
				CreateThread(NULL, 0, loader_thread, (LPVOID)heap_inputs, 0, NULL);
			}
			if (G->signals.delete_current_image) {
				G->signals.delete_current_image = false;
				wchar_t current_path[CUTE_FILES_MAX_PATH];
				wcscpy(current_path, G->files[G->current_file_index].file.path);
				
				// Determine which file to show after deletion
				u32 next_index = G->current_file_index;
				bool has_next = G->current_file_index < G->files.Count - 1;
				bool has_prev = G->current_file_index > 0;
				
				if (delete_current_image()) {
					// After successful deletion, rescan folder and navigate
					if (has_next || has_prev) {
						// Jump to next file, or previous if we were at the end
						if (!has_next && has_prev) {
							next_index = G->current_file_index - 1;
						}
						// Rescan folder to update file list
						wchar_t *target_path = nullptr;
						if (has_next && G->current_file_index + 1 < G->files.Count) {
							target_path = G->files[G->current_file_index + 1].file.path;
						} else if (has_prev && G->current_file_index > 0) {
							target_path = G->files[G->current_file_index - 1].file.path;
						}
						if (target_path) {
							scan_folder(target_path);
							if (G->files.Count > 0) {
								G->loaded = false;
								Loader_Thread_Inputs *heap_inputs = create_loader_inputs(G->files[G->current_file_index].file.path, G->current_file_index, &G->files[G->current_file_index], false);
								CreateThread(NULL, 0, loader_thread, (LPVOID)heap_inputs, 0, NULL);
							}
						}
					} else {
						// Last file in folder was deleted
						G->files.reset_count();
						set_to_no_file();
						SetWindowTextW(hwnd, L"CactusViewer");
					}
				}
			}
        }
        reset_inputs();
		if (keyup(MouseL))
			G->mouse_dn_hash = 0;
    }
    DLOG_INIT("Main loop ended, saving settings");
    save_settings();
    debug_log_close();
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