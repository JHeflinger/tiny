#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>

#define print(...) {printf(__VA_ARGS__);printf("\n");}
#define crash(...) {printf("\033[31m[CRITICAL FAILURE]\033[0m "); print(__VA_ARGS__);exit(1);}
#define warn(...) {printf("\033[33m[WARNING]\033[0m "); print(__VA_ARGS__);}
#define PATHLEN 4096

typedef void (*FileHandler)(const char*);

#ifdef __linux__
	#include <unistd.h>
	#include <dirent.h>
	#include <sys/time.h>

	#define cwd(buffer) getcwd(buffer, sizeof(buffer))
	#define makedir(dir) (!mkdir(dir, 0755))

	int dexists(const char* dir) {
		struct stat statbuf;
		if (stat(dir, &statbuf) != 0) {
			return 0;
		}
		return S_ISDIR(statbuf.st_mode);
	}

	int fexists(const char* file) {
		struct stat statbuf;
		if (stat(file, &statbuf) != 0) {
			return 0;
		}
		return !S_ISDIR(statbuf.st_mode);
	}

	void walkdir(const char* path, FileHandler func) {
		DIR *dir = opendir(path);
		if (!dir) crash("Unable to open directory \"%s\"", path);
		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
			char full_path[PATH_MAX];
			snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
	        struct stat statbuf;
			if (stat(full_path, &statbuf) != 0) crash("Stat call failed");
			if (S_ISDIR(statbuf.st_mode)) {
				func(full_path);
				walkdir(full_path, func);
			}
		}
		closedir(dir);
	}

	void walkfiles(const char* path, FileHandler func) {
		DIR *dir = opendir(path);
		if (!dir) crash("Unable to open directory \"%s\"", path);
		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
			char full_path[PATH_MAX];
			snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
	        struct stat statbuf;
			if (stat(full_path, &statbuf) != 0) crash("Stat call failed");
			if (!S_ISDIR(statbuf.st_mode)) {
				func(full_path);
			} else {
				walkfiles(full_path, func);
			}
		}
		closedir(dir);
	}

	uint64_t mtime() {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return (uint64_t)(tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
	}
#elifdef __WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
	#define NOVIRTUALKEYCODES // VK_*
	#define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
	#define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
	#define NOSYSMETRICS      // SM_*
	#define NOMENUS           // MF_*
	#define NOICONS           // IDI_*
	#define NOKEYSTATES       // MK_*
	#define NOSYSCOMMANDS     // SC_*
	#define NORASTEROPS       // Binary and Tertiary raster ops
	#define NOSHOWWINDOW      // SW_*
	#define OEMRESOURCE       // OEM Resource values
	#define NOATOM            // Atom Manager routines
	#define NOCLIPBOARD       // Clipboard routines
	#define NOCOLOR           // Screen colors
	#define NOCTLMGR          // Control and Dialog routines
	#define NODRAWTEXT        // DrawText() and DT_*
	#define NOGDI             // All GDI defines and routines
	#define NOKERNEL          // All KERNEL defines and routines
	#define NOUSER            // All USER defines and routines
	#define NOMB              // MB_* and MessageBox()
	#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
	#define NOMETAFILE        // typedef METAFILEPICT
	#define NOMSG             // typedef MSG and associated routines
	#define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
	#define NOSCROLL          // SB_* and scrolling routines
	#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
	#define NOSOUND           // Sound driver routines
	#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
	#define NOWH              // SetWindowsHook and WH_*
	#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
	#define NOCOMM            // COMM driver routines
	#define NOKANJI           // Kanji support stuff.
	#define NOHELP            // Help engine interface.
	#define NOPROFILER        // Profiler interface.
	#define NODEFERWINDOWPOS  // DeferWindowPos routines
	#define NOMCX             // Modem Configuration Extensions

	#include <direct.h>
	#include <windows.h>

	#define cwd(buffer) _getcwd(buffer, sizeof(buffer))
	#define makedir(dir) (!_mkdir(dir))

	int dexists(const char* dir) {
		struct _stat statbuf;
		if (_stat(dir, &statbuf) != 0) {
		    return 0;
		}
		return (statbuf.st_mode & _S_IFDIR) != 0;
	}

	int fexists(const char* file) {
		struct _stat statbuf;
		if (_stat(file, &statbuf) != 0) {
		    return 0;
		}
		return (statbuf.st_mode & _S_IFDIR) == 0;
	}

	void walkdir(const char* path) {
		char search_path[MAX_PATH];
		snprintf(search_path, MAX_PATH, "%s/*", path);
		WIN32_FIND_DATAA find_data;
		HANDLE hFind = FindFirstFileA(search_path, &find_data);
		if (hFind == INVALID_HANDLE_VALUE) crash("Unable to open directory \"%s\"", path);
		do {
			const char *name = find_data.cFileName;
			if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
			char full_path[MAX_PATH];
			snprintf(full_path, MAX_PATH, "%s/%s", path, name);
			if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				func(full_path);
				walkdir(full_path, func);
			}
		} while (FindNextFileA(hFind, &find_data) != 0);
		FindClose(hFind);
	}

	void walkfiles(const char* path) {
		char search_path[MAX_PATH];
		snprintf(search_path, MAX_PATH, "%s/*", path);
		WIN32_FIND_DATAA find_data;
		HANDLE hFind = FindFirstFileA(search_path, &find_data);
		if (hFind == INVALID_HANDLE_VALUE) crash("Unable to open directory \"%s\"", path);
		do {
			const char *name = find_data.cFileName;
			if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
			char full_path[MAX_PATH];
			snprintf(full_path, MAX_PATH, "%s/%s", path, name);
			if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				func(full_path);
			} else {
				walkfiles(full_path, func);
			}
		} while (FindNextFileA(hFind, &find_data) != 0);
		FindClose(hFind);
	}

	uint64_t mtime() {
		return (uint64_t)GetTickCount64();
	}
#else
	#error "Unsupported operating system detected!"
#endif

typedef enum {
	NONE = 0,
	PROD = 1 << 0,
} BuildFlags;

typedef struct {
	char str[PATHLEN];
	void* next;
} PathList;

size_t s_start_time = 0;
BuildFlags s_flags = NONE;
char s_project_directory[PATHLEN];
char s_main_file_name[PATHLEN];
int s_found_main = 0;
int s_sources_up_to_date = 1;
char s_main_file_path[PATHLEN] = { 0 };
char s_cwd[PATHLEN] = { 0 };
PathList* s_includes = NULL;
PathList* s_links = NULL;
PathList* s_libs = NULL;
PathList* s_sources = NULL;
PathList* s_objects = NULL;
PathList* s_changed_headers = NULL;

void copyfile(const char *src, const char *dst) {
    FILE *srcFile = fopen(src, "rb");
    if (!srcFile) {
        crash("Failed to open source file");
    }
    FILE *dstFile = fopen(dst, "wb");
    if (!dstFile) {
        fclose(srcFile);
        crash("Failed to open destination file");
    }
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), srcFile)) > 0) {
        if (fwrite(buffer, 1, bytes, dstFile) != bytes) {
            fclose(srcFile);
            fclose(dstFile);
            crash("Critical write error during copying");
        }
    }
    fclose(srcFile);
    fclose(dstFile);
}

int filecmp(const char *path1, const char *path2) {
    FILE *f1 = fopen(path1, "rb");
    FILE *f2 = fopen(path2, "rb");
    if (!f1 || !f2) {
        fclose(f1);
        fclose(f2);
		crash("Error opening files");
    }
    struct stat stat1, stat2;
    if (stat(path1, &stat1) != 0 || stat(path2, &stat2) != 0) {
        fclose(f1);
        fclose(f2);
		crash("Error stating files");
    }
    if (stat1.st_size != stat2.st_size) {
        fclose(f1);
        fclose(f2);
        return 0;
    }
    char buf1[4096], buf2[4096];
    size_t bytes1, bytes2;
    int equal = 1;
    while ((bytes1 = fread(buf1, 1, 4096, f1)) > 0 &&
           (bytes2 = fread(buf2, 1, 4096, f2)) > 0) {
        if (bytes1 != bytes2 || memcmp(buf1, buf2, bytes1) != 0) {
            equal = 0;
            break;
        }
    }
    fclose(f1);
    fclose(f2);
    return equal;
}

void pathlist_add(PathList** list, const char* path) {
	PathList* new = calloc(1, sizeof(PathList));
	new->next = NULL;
	strncpy(new->str, path, PATHLEN);
	if (*list == NULL) {
		*list = new;
		return;
	}
	PathList* current = *list;
	new->next = current;
	*list = new;
}

void pathlist_delete(PathList* list) {
	if (list == NULL) return;
	PathList* n = (PathList*)list->next;
	free(list);
	pathlist_delete(n);
}

size_t pathlist_len(PathList* list) {
	if (list == NULL) return 0;
	return 1 + pathlist_len(list->next);
}

void pathlist_construct(PathList* list, char* output) {
	size_t outcounter = 0;
	PathList* curr = list;
	while (curr != NULL) {
		strcpy(output + outcounter, curr->str);
		outcounter += strlen(curr->str);
		output[outcounter] = ' ';
		outcounter++;
		output[outcounter] = '\0';
		curr = (PathList*)curr->next;
	}
}

void affirmdir(const char* dir) {
	if (!dexists(dir))
		if (!makedir(dir)) crash("Unable to affirm directory %s", dir);
}

void affirm_to_cache(const char* dir) {
	char buffer[PATHLEN] = { 0 };
	snprintf(buffer, PATHLEN, "build/cache/%s", dir);
	affirmdir(buffer);
}

void add_to_sources(const char* file) {
	size_t slen = strlen(file);
	if (slen > 2 && file[slen - 1] == 'c' && file[slen - 2] == '.')
		pathlist_add(&s_sources, file);
}

void verify_header(const char* file) {
	size_t slen = strlen(file);
	if (slen > 2 && (file[slen - 1] != 'h' || file[slen - 2] != '.')) return;
	int basename_ptr = 0;
	for (int i = slen; i > 0; i--) {
		if (file[i] == '/' || file[i] == '\\') {
			basename_ptr = i + 1;
			break;
		}
	}
	char destination[PATHLEN] = { 0 };
	snprintf(destination, PATHLEN, "build/cache/%s", file);
	if (!fexists(destination)) {
		pathlist_add(&s_changed_headers, file + basename_ptr);
		copyfile(file, destination);
	} else {
		if (!filecmp(file, destination)) {
			pathlist_add(&s_changed_headers, file + basename_ptr);
			copyfile(file, destination);
		}
	}
}

void accumulate_header(const char* file) {
	size_t slen = strlen(file);
	if (slen > 2 && (file[slen - 1] != 'h' || file[slen - 2] != '.')) return;
	int basename_ptr = 0;
	for (int i = slen; i > 0; i--) {
		if (file[i] == '/' || file[i] == '\\') {
			basename_ptr = i + 1;
			break;
		}
	}
	PathList* curr = s_changed_headers;
	while (curr != NULL) {
		if (strcmp(file + basename_ptr, curr->str) == 0) return;
		curr = (PathList*)curr->next;
	}
	FILE* fp = fopen(file, "r");
	if (!fp) crash("Unable to open file \"%s\"", file);
	char line[PATHLEN * 2] = { 0 };
	while (fgets(line, sizeof(line), fp)) {
		if (strstr(line, "#include") != NULL) {
			curr = s_changed_headers;
			while (curr != NULL) {
				if (strstr(line, curr->str) != NULL) {
					pathlist_add(&s_changed_headers, file + basename_ptr);
					fclose(fp);
					return;
				}
				curr = (PathList*)curr->next;
			}
		}
	}
	fclose(fp);
}

void compile_source(const char* file) {
	size_t slen = strlen(file);
	if (slen > 2 && (file[slen - 1] != 'c' || file[slen - 2] != '.')) return;
	int basename_ptr = 0;
	for (int i = slen; i > 0; i--) {
		if (file[i] == '/' || file[i] == '\\') {
			basename_ptr = i + 1;
			break;
		}
	}
	if (strcmp(file + basename_ptr, s_main_file_name) == 0) {
		if (s_found_main) {
			print("\033[31mError\033[0m: another main file detected: %s", file);
			exit(1);
		}
		s_found_main = 1;
		strcpy(s_main_file_path, file);
	} else {
		char destination[PATHLEN] = { 0 };
		snprintf(destination, PATHLEN, "build/cache/%s", file);
		if (fexists(destination)) {	
			FILE* fp = fopen(file, "r");
			if (!fp) crash("Unable to open file \"%s\"", file);
			char line[PATHLEN * 2] = { 0 };
			while (fgets(line, sizeof(line), fp)) {
				if (strstr(line, "#include") != NULL) {
					PathList* curr = s_changed_headers;
					int breakagain = 0;
					while (curr != NULL) {
						if (strstr(line, curr->str) != NULL) {
							remove(destination);
							breakagain = 1;
							break;
						}
						curr = (PathList*)curr->next;
					}
					if (breakagain) break;
				}
			}
			fclose(fp);
		}
		char* incbuf = calloc(pathlist_len(s_includes), PATHLEN);
		char* linkbuf = calloc(pathlist_len(s_links), PATHLEN);
		char* libbuf = calloc(pathlist_len(s_libs), PATHLEN);
		pathlist_construct(s_includes, incbuf);
		pathlist_construct(s_links, linkbuf);
		pathlist_construct(s_libs, libbuf);
		char* commandbuf = calloc(strlen(incbuf) + strlen(linkbuf) + strlen(libbuf) + PATHLEN, sizeof(char));
		sprintf(
			commandbuf,
			"gcc -Wall -Wextra -Wno-unused-parameter -c %s %s%s%s-o %s.o %s",
			file,
			incbuf,
			libbuf,
			linkbuf,
			destination,
			s_flags & PROD ? "-O3 -DPROD_BUILD" : "");
		if (!fexists(destination) || !filecmp(file, destination)) {
			s_sources_up_to_date = 0;
			print("- [%s] \033[33m(compiling...)\033[0m", file + basename_ptr);
			int result = system(commandbuf);
			if (result == 0) {
				print("\033[1A\033[0K- [%s] \033[32mOK\033[0m", file + basename_ptr);
				copyfile(file, destination);
			} else {
				print("Building source \"%s\" \033[31mfailed\033[0m", file + basename_ptr);
				exit(1);
			}
		}
		free(commandbuf);
		free(incbuf);
		free(linkbuf);
		free(libbuf);
		char finalbuf[PATHLEN + 2] = { 0 };
		snprintf(finalbuf, PATHLEN + 2, "%s.o", destination);
		pathlist_add(&s_objects, finalbuf);
	}
}

void initialize(int argc, char* argv[]) {
	// initialize timer
	s_start_time = mtime();

	// parse flags
	for (int i = 1; i < argc; i++) {
		if (strcmp("-p", argv[i]) == 0 || strcmp("-prod", argv[i]) == 0) {
			print("Optimizing for production build...");
			s_flags |= PROD;
		}
	}

	// set up cwd
	if (cwd(s_cwd) == NULL) {
		crash("Unable to find the current working directory");
	}

	// initialize project and main dir and file
	strcpy(s_project_directory, "src");
	strcpy(s_main_file_name, "main.c");
	if (fexists(".tinyconf")) {
		FILE* file = fopen(".tinyconf", "r");
		if (!file) crash("Unable to open configuration file");
		char line[PATHLEN * 2] = { 0 };
		char precursor[PATHLEN] = { 0 };
		while (fgets(line, sizeof(line), file)) {
			for (int i = strlen(line) - 1; i >= 0; i--) {
				if (line[i] == '\n' || line[i] == '\r') {
					line[i] = '\0';
				} else break;
			}
			size_t postcursor = 0;
			for (size_t i = 0; i < strlen(line); i++) {
				if (line[i] == ' ') {
					precursor[i] = '\0';
					postcursor = i + 1;
					break;
				} else {
					precursor[i] = line[i];
				}
			}
			if (strcmp(precursor, "PROJECT") == 0) {
				strcpy(s_project_directory, line + postcursor);
			} else if (strcmp(precursor, "MAIN") == 0) {
				strcpy(s_main_file_name, line + postcursor);
			}
		}
		fclose(file);
	}

	// set up build directories
	affirmdir("build");
	affirmdir("build/cache");
	affirmdir("build/vendor");
	char tbuf[PATHLEN + 12] = { 0 };
	snprintf(tbuf, PATHLEN + 12, "build/cache/%s", s_project_directory);
	affirmdir(tbuf);

	// set up cache folders
	walkdir(s_project_directory, affirm_to_cache);

	// set include dir
	snprintf(tbuf, PATHLEN + 12, "-I\"%s\"", s_project_directory);
	pathlist_add(&s_includes, tbuf);
}

void add_vendors() {
	if (!fexists(".tinyconf")) return;
	FILE* file = fopen(".tinyconf", "r");
	if (!file) crash("Unable to open configuration file");
	char line[PATHLEN * 2] = { 0 };
	char precursor[PATHLEN] = { 0 };
	char workbuffer[PATHLEN] = { 0 };
	int linecount = 0;
	while (fgets(line, sizeof(line), file)) {
		linecount++;
		for (int i = strlen(line) - 1; i >= 0; i--) {
			if (line[i] == '\n' || line[i] == '\r') {
				line[i] = '\0';
			} else break;
		}
		size_t postcursor = 0;
		for (size_t i = 0; i < strlen(line); i++) {
			if (line[i] == ' ') {
				precursor[i] = '\0';
				postcursor = i + 1;
				break;
			} else {
				precursor[i] = line[i];
			}
		}
		if (strcmp(precursor, "INCLUDE") == 0) {
			snprintf(workbuffer, PATHLEN, "-I\"%s\"", line + postcursor);
			pathlist_add(&s_includes, workbuffer);
		} else if (strcmp(precursor, "LINK") == 0) {
			snprintf(workbuffer, PATHLEN, "-l%s", line + postcursor);
			pathlist_add(&s_links, workbuffer);
		} else if (strcmp(precursor, "LIB") == 0) {
			snprintf(workbuffer, PATHLEN, "-L\"%s\"", line + postcursor);
			pathlist_add(&s_libs, workbuffer);
		} else if (strcmp(precursor, "SOURCE") == 0) {
			if (dexists(line + postcursor)) {
				walkfiles(line + postcursor, add_to_sources);	
			} else {
				pathlist_add(&s_sources, line + postcursor);
			}
		} else if (strcmp(precursor, "PROJECT") != 0 && strcmp(precursor, "MAIN") != 0) {
			warn("Unknown precursor \"%s\" detected on line %d of \".tinyconf\" - skipping", precursor, linecount);
		}
	}
	fclose(file);
}

void compile_vendors() {
	if (s_sources == NULL) return;
	if (!fexists("build/vendor/vendor.o")) {
		print("Compiling vendors...");
		FILE* file = fopen("build/vendor/tiny_merged_vendors.c", "w");
		if (!file) crash("Unable to consolidate vendor sources");
		PathList* pl = s_sources;
		while (pl != NULL) {
			char buffer[PATHLEN + 18] = { 0 };
			snprintf(buffer, PATHLEN + 18, "#include \"../../%s\"\n", pl->str);
			fprintf(file, buffer);
			pl = (PathList*)pl->next;
		}
		fclose(file);
		char* incbuf = calloc(pathlist_len(s_includes), PATHLEN);
		char* linkbuf = calloc(pathlist_len(s_links), PATHLEN);
		char* libbuf = calloc(pathlist_len(s_libs), PATHLEN);
		pathlist_construct(s_includes, incbuf);
		pathlist_construct(s_links, linkbuf);
		pathlist_construct(s_libs, libbuf);
		char* commandbuf = calloc(strlen(incbuf) + strlen(linkbuf) + strlen(libbuf) + PATHLEN, sizeof(char));
		sprintf(
			commandbuf,
			"gcc -Wall -Wextra -Wno-unused-parameter -c build/vendor/tiny_merged_vendors.c %s%s%s-o build/vendor/vendor.o %s",
			incbuf,
			libbuf,
			linkbuf,
			s_flags & PROD ? "-O3 -DPROD_BUILD" : "");
		uint64_t timer = mtime();
		int result = system(commandbuf);
		timer = mtime() - timer;
		if (result == 0) {
			int hours = (int)(timer / 3600000);
			int minutes = (int)((timer - (hours * 3600000)) / 60000);
			float seconds = (((float)timer) - (hours * 3600000) - (minutes * 60000)) / 1000.0f;
			print("\033[32mFinished\033[0m compiling vendors in %d:%d:%.3f", hours, minutes, seconds);
		} else {
			print("Building vendors \033[31mfailed\033[0m");
		}
		free(commandbuf);
		free(incbuf);
		free(linkbuf);
		free(libbuf);
	}
	pathlist_add(&s_objects, "build/vendor/vendor.o");
}

void calculate_dependencies() {
	print("Calculating dependency tree...");
	uint64_t timer = mtime();
	walkfiles(s_project_directory, verify_header);
	if (s_changed_headers == NULL) {
		print("\033[1A\033[0KHeaders are currently \033[32mup to date\033[0m");
		return;
	}
	while (1) {
		PathList* current = s_changed_headers;
		walkfiles(s_project_directory, accumulate_header);
		if (current == s_changed_headers) break;
	}
	timer = mtime() - timer;
	int hours = (int)(timer / 3600000);
	int minutes = (int)((timer - (hours * 3600000)) / 60000);
	float seconds = (((float)timer) - (hours * 3600000) - (minutes * 60000)) / 1000.0f;
	print("\033[32mFinished\033[0m calculating depdencies in %d:%d:%.3f", hours, minutes, seconds);
}

void compile_objects() {
	print("Compiling sources...");
	uint64_t timer = mtime();
	walkfiles(s_project_directory, compile_source);
	if (s_sources_up_to_date) {
		print("\033[1A\033[0KSources are currently \033[32mup to date\033[0m");
	} else {
		timer = mtime() - timer;
		int hours = (int)(timer / 3600000);
		int minutes = (int)((timer - (hours * 3600000)) / 60000);
		float seconds = (((float)timer) - (hours * 3600000) - (minutes * 60000)) / 1000.0f;
		print("\033[32mFinished\033[0m compiling vendors in %d:%d:%.3f", hours, minutes, seconds);
	}
	if (!s_found_main) {
		print("\033[31mError\033[0m: unable to compile without a detected \"%s\" file", s_main_file_name);
		exit(1);
	}
}

void compile_executable() {
	print("Building executable...");
	uint64_t timer = mtime();
	char* incbuf = calloc(pathlist_len(s_includes), PATHLEN);
	char* linkbuf = calloc(pathlist_len(s_links), PATHLEN);
	char* libbuf = calloc(pathlist_len(s_libs), PATHLEN);
	char* objbuf = calloc(pathlist_len(s_objects), PATHLEN);
	pathlist_construct(s_includes, incbuf);
	pathlist_construct(s_links, linkbuf);
	pathlist_construct(s_libs, libbuf);
	pathlist_construct(s_objects, objbuf);
	char* commandbuf = calloc(strlen(incbuf) + strlen(linkbuf) + strlen(libbuf) + strlen(objbuf) + PATHLEN, sizeof(char));
	sprintf(
		commandbuf,
		"gcc -Wall -Wextra -Wno-unused-parameter %s %s%s%s%s-o build/bin.exe %s",
		s_main_file_path,
		objbuf,
		incbuf,
		libbuf,
		linkbuf,
		s_flags & PROD ? "-O3 -DPROD_BUILD" : "");
	int result = system(commandbuf);
	timer = mtime() - timer;
	if (result == 0) {
		int hours = (int)(timer / 3600000);
		int minutes = (int)((timer - (hours * 3600000)) / 60000);
		float seconds = (((float)timer) - (hours * 3600000) - (minutes * 60000)) / 1000.0f;
		print("\033[32mFinished\033[0m compiling executable in %d:%d:%.3f", hours, minutes, seconds);
	} else {
		print("Building executable \033[31mfailed\033[0m");
		exit(1);
	}
	free(commandbuf);
	free(objbuf);
	free(incbuf);
	free(linkbuf);
	free(libbuf);
}

int main(int argc, char* argv[]) {
	initialize(argc, argv);
	add_vendors();
	compile_vendors();
	calculate_dependencies();
	compile_objects();
	compile_executable();	
	pathlist_delete(s_sources);
	pathlist_delete(s_includes);
	pathlist_delete(s_links);
	pathlist_delete(s_libs);
	s_start_time = mtime() - s_start_time;
	int hours = (int)(s_start_time / 3600000);
	int minutes = (int)((s_start_time - (hours * 3600000)) / 60000);
	float seconds = (((float)s_start_time) - (hours * 3600000) - (minutes * 60000)) / 1000.0f;
	print("\033[32mFinished\033[0m total build in %d:%d:%.3f", hours, minutes, seconds);
	return 0;
}
