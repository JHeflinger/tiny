/*
* author: Jason Heflinger
* description: Tiny GCC project manager
*/
#define VERSION 1
#define MAJOR_RELEASE 0
#define MINOR_RELEASE 0

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
	AUDIT = 1 << 1,
} BuildFlags;

typedef struct {
	char str[PATHLEN];
	void* next;
} PathList;

typedef struct {
	char header[PATHLEN];
	PathList* links;
	PathList* secondaries;
} HeaderLink;

typedef struct {
	HeaderLink* link;
	void* next;
} HeaderLinkList;

size_t s_start_time = 0;
BuildFlags s_flags = NONE;
char s_project_directory[PATHLEN];
char s_main_file_name[PATHLEN];
int s_found_main = 0;
int s_sources_up_to_date = 1;
int s_main_up_to_date = 1;
int s_vulnerabilities = 0;
char s_main_file_path[PATHLEN] = { 0 };
char s_cwd[PATHLEN] = { 0 };
PathList* s_includes = NULL;
PathList* s_links = NULL;
PathList* s_libs = NULL;
PathList* s_sources = NULL;
PathList* s_objects = NULL;
PathList* s_changed_headers = NULL;
HeaderLinkList* s_header_links = NULL;
HeaderLinkList* s_source_links = NULL;

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

void clean_header_links() {
	while (s_header_links) {
		pathlist_delete(s_header_links->link->links);
		pathlist_delete(s_header_links->link->secondaries);
		free(s_header_links->link);
		HeaderLinkList* old = s_header_links;
		s_header_links = (HeaderLinkList*)s_header_links->next;
		free(old);
	}
}

void clean_source_links() {
	while (s_source_links) {
		pathlist_delete(s_source_links->link->links);
		pathlist_delete(s_source_links->link->secondaries);
		free(s_source_links->link);
		HeaderLinkList* old = s_source_links;
		s_source_links = (HeaderLinkList*)s_source_links->next;
		free(old);
	}
}

int header_link_exists(const char* header) {
	HeaderLinkList* curr = s_header_links;
	while (curr) {
		if (strcmp(curr->link->header, header) == 0) {
			return 1;
		}
		curr = (HeaderLinkList*)curr->next;
	}
	return 0;
}

void add_header_link(const char* header, const char* link) {
	HeaderLinkList* curr = s_header_links;
	while (curr) {
		if (strcmp(curr->link->header, header) == 0) {
			pathlist_add(&(curr->link->links), link);
			return;
		}
		curr = (HeaderLinkList*)curr->next;
	}
	HeaderLinkList* new = calloc(1, sizeof(HeaderLinkList));
	new->link = calloc(1, sizeof(HeaderLink));
	strcpy(new->link->header, header);
	pathlist_add(&(new->link->links), link);
	new->next = s_header_links;
	s_header_links = new;
}

void add_source_link(const char* header, const char* link) {
	HeaderLinkList* curr = s_source_links;
	while (curr) {
		if (strcmp(curr->link->header, header) == 0) {
			pathlist_add(&(curr->link->links), link);
			return;
		}
		curr = (HeaderLinkList*)curr->next;
	}
	HeaderLinkList* new = calloc(1, sizeof(HeaderLinkList));
	new->link = calloc(1, sizeof(HeaderLink));
	strcpy(new->link->header, header);
	pathlist_add(&(new->link->links), link);
	new->next = s_source_links;
	s_source_links = new;
}

void add_header_secondary(const char* header, const char* link) {
	HeaderLinkList* curr = s_header_links;
	while (curr) {
		if (strcmp(curr->link->header, header) == 0) {
			pathlist_add(&(curr->link->secondaries), link);
			return;
		}
		curr = (HeaderLinkList*)curr->next;
	}
	HeaderLinkList* new = calloc(1, sizeof(HeaderLinkList));
	new->link = calloc(1, sizeof(HeaderLink));
	strcpy(new->link->header, header);
	pathlist_add(&(new->link->secondaries), link);
	new->next = s_header_links;
	s_header_links = new;
}

int functionline(const char* line) {
	int sfound = 0;
	int typefound = 0;
	int p1found = 0;
	int p2found = 0;
	int i = 0;
	while (1) {
		if (line[i] == '\0') return 0;
		if (!sfound && line[i] != ' ') sfound = 1;
		if (sfound && !typefound && line[i] == ' ') typefound = 1;
		else if (typefound && line[i] == '(' && !p1found) p1found = 1;
		else if (p1found && line[i] == '(') return 0;
		else if (p1found && line[i] == ')') p2found = 1;
		else if (p2found && line[i] == ';') return 1;
		else if (p2found) return 0;
		i++;
	}
	return 0;
}

void syntax_audit(const char* file) {
	int slen = strlen(file);
	int header = (slen > 2 && (file[slen - 1] == 'h' && file[slen - 2] == '.'));
	int source = (slen > 2 && (file[slen - 1] == 'c' && file[slen - 2] == '.'));
	if (!header && !source) {
		print("Detected abnormal file type in project: \"%s\"", file);
		s_vulnerabilities++;
		return;
	}
	int basename_ptr = 0;
	for (int i = slen; i > 0; i--) {
		if (file[i] == '/' || file[i] == '\\') {
			basename_ptr = i + 1;
			break;
		}
	}
	FILE* fp = fopen(file, "r");
	if (!fp) crash("Failed to open file");
	char line[PATHLEN * 2] = { 0 };
	int linecount = 0;
	int prev_empty = 0;
	int header_closed = 0;
	while (fgets(line, sizeof(line), fp)) {
		linecount++;
		int linelen = strlen(line);
		if (linelen >= PATHLEN) {
			print("Detected excessively long line in \"%s\" on line %d", file, linecount);
			s_vulnerabilities++;
		}
		int empty_line = 1;
		for (int i = 0; i < linelen; i++) {
			if (line[i] != '\n' && line[i] != ' ' && line[i] != '\t' && line[i] != '\r') {
				empty_line = 0;
				break;
			}
		}
		if (empty_line && prev_empty) {
			print("Detected excessive whitespace in \"%s\" on line %d", file, linecount);
			s_vulnerabilities++;
			prev_empty = 0;
		} else if (empty_line) {
			prev_empty = 1;
		} else {
			prev_empty = 0;
		}
		if (linecount == 1 && header) {
			char hbuf[PATHLEN] = { 0 };
			snprintf(hbuf, PATHLEN, "#ifndef %s", file + basename_ptr);
			int hlen = strlen(hbuf);
			for (int i = 8; i < hlen; i++) {
				if (hbuf[i] >= 'a' && hbuf[i] <= 'z') hbuf[i] = hbuf[i] + ('A' - 'a');
				else if (hbuf[i] == '.') hbuf[i] = '_';
			}
			char t = line[hlen];
			line[hlen] = '\0';
			if (strcmp(hbuf, line) != 0) {
				print("Detected missing or incorrect header guard in \"%s\" on line %d", file, linecount);
				s_vulnerabilities++;
			}
			line[hlen] = t;
		} else if (linecount == 2 && header) {
			char hbuf[PATHLEN] = { 0 };
			snprintf(hbuf, PATHLEN, "#define %s", file + basename_ptr);
			int hlen = strlen(hbuf);
			for (int i = 8; i < hlen; i++) {
				if (hbuf[i] >= 'a' && hbuf[i] <= 'z') hbuf[i] = hbuf[i] + ('A' - 'a');
				else if (hbuf[i] == '.') hbuf[i] = '_';
			}
			char t = line[hlen];
			line[hlen] = '\0';
			if (strcmp(hbuf, line) != 0) {
				print("Detected missing or incorrect header guard in \"%s\" on line %d", file, linecount);
				s_vulnerabilities++;
			}
			line[hlen] = t;
		}
		if (header) {
			char endbuf[7] = { 0 };
			if (linelen >= 6) memcpy(endbuf, line, 6);
			if (linelen >= 6 && strcmp("#endif", endbuf) == 0) header_closed = 1;
			else if (!empty_line) header_closed = 0;
		}
		if (strstr(line,"malloc(") || strstr(line, "calloc(") || strstr(line, "free(")) {
			print("Detected unmonitored memory operation in \"%s\" on line %d", file, linecount);
			s_vulnerabilities++;
		}
		if (header) {
			if (functionline(line)) {
				char implbuf[PATHLEN] = { 0 };
				char badimplbuf[PATHLEN] = { 0 };
				for (int i = 0; i < PATHLEN; i++) {
					if (line[i] == ';') {
						implbuf[i] = ' ';
						implbuf[i + 1] = '{';
						implbuf[i + 2] = '\0';
						badimplbuf[i] = '{';
						badimplbuf[i + 1] = '\0';
						break;
					} else {
						implbuf[i] = line[i];
						badimplbuf[i] = line[i];
					}
				}
				int implfound = 0;
				int badimplfound = 0;
				char srcfilepath[PATHLEN] = { 0 };
				strcpy(srcfilepath, file);
				srcfilepath[slen - 1] = 'c';
				if (fexists(srcfilepath)) {
					FILE* sfp = fopen(srcfilepath, "r");
					if (!sfp) crash("Unable to open file");
					char srcline[PATHLEN * 2] = { 0 };
					int srclc = 0;
					while (fgets(srcline, sizeof(srcline), sfp)) {
						srclc++;
						if (strstr(srcline, implbuf)) implfound = 1;
						if (strstr(srcline, badimplbuf)) badimplfound = 1;
					}
					if (!implfound) {
						if (badimplfound) {
							print("The function implementation in \"%s\" on line %d is improperly formatted - please have a space in between the function and the curly brace.", srcfilepath, srclc);
							s_vulnerabilities++;
						} else {
							print("Unable to find an implementation for the function on line %d of header \"%s\" in the corresponding source file", linecount, file);
							s_vulnerabilities++;
						}
					}
					fclose(sfp);
				} else {
					print("Unable to find a corresponding source for the header \"%s\"", file);
					s_vulnerabilities++;
				}
			}
		}
		if (header) {
			if (strstr(line, "#include")) {
				char ibuf[PATHLEN] = { 0 };
				int ind = 0;
				int toggle = 0;
				for (int i = 0; i < linelen; i++) {
					if (line[i] == '\"' || line[i] == '<' || line[i] == '>') toggle = !toggle;
					else if (toggle) {
						ibuf[ind] = line[i];
						ind++;
						if (line[i] == '/' || line[i] == '\\') {
							ind = 0;
							memset(ibuf, 0, PATHLEN);
						}
					}
				}
				add_header_link(file + basename_ptr, ibuf);
			}
		}
		if (source) {
			if (strstr(line, "#include")) {
				char ibuf[PATHLEN] = { 0 };
				int ind = 0;
				int toggle = 0;
				for (int i = 0; i < linelen; i++) {
					if (line[i] == '\"' || line[i] == '<' || line[i] == '>') toggle = !toggle;
					else if (toggle) {
						ibuf[ind] = line[i];
						ind++;
						if (line[i] == '/' || line[i] == '\\') {
							ind = 0;
							memset(ibuf, 0, PATHLEN);
						}
					}
				}
				add_source_link(file + basename_ptr, ibuf);
			}
		}
	}
	if (header && !header_closed) {
		print("Detected missing #endif to close header guard in \"%s\"", file);
		s_vulnerabilities++;
	}
	if (linecount == 0) {
		print("Detected empty file \"%s\"", file);
		s_vulnerabilities++;
	}
	fclose(fp);
}

void copyfile(const char* src, const char* dst) {
    FILE* srcFile = fopen(src, "rb");
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

int filecmp(const char* path1, const char* path2) {
    FILE* f1 = fopen(path1, "rb");
    FILE* f2 = fopen(path2, "rb");
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
	char destination[PATHLEN] = { 0 };
	snprintf(destination, PATHLEN, "build/cache/%s", file);
	if (strcmp(file + basename_ptr, s_main_file_name) == 0) {
		if (s_found_main) {
			print("\033[31mError\033[0m: another main file detected: %s", file);
			exit(1);
		}
		s_found_main = 1;
		strcpy(s_main_file_path, file);
		if (!fexists(destination) || !filecmp(destination, file)) {
			copyfile(file, destination);
			s_main_up_to_date = 0;
		}
	} else {
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
		if (strcmp("-a", argv[i]) == 0 || strcmp("-audit", argv[i]) == 0) {
			s_flags |= AUDIT;
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

	// ensure project directory exists
	if (!dexists(s_project_directory)) {
		print("\033[31mError\033[0m: Project directory \"%s/\" does not exist - if this is not your desired location, please specify a different one in \".tinyconf\"", s_project_directory);
		exit(1);
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

void get_in_depth_headers(const char* dive_header, HeaderLinkList* update_header) {
	HeaderLinkList* header = s_header_links;
	while (header) {
		if (strcmp(header->link->header, dive_header) == 0) {
			PathList* secondary = header->link->links;
			while (secondary) {
				if (strcmp(secondary->str, update_header->link->header) == 0) {
					print("\033[31mCritical error\033[0m: recursive include detected from \"%s\" in \"%s\"! Aborting build...", update_header->link->header, dive_header);
					exit(1);
				}
				add_header_secondary(update_header->link->header, secondary->str);
				if (header_link_exists(secondary->str)) {
					get_in_depth_headers(secondary->str, update_header);
				}
				secondary = (PathList*)secondary->next;
			}
			return;
		}
		header = (HeaderLinkList*)header->next;
	}
}

void audit() {
	print("Auditing project...")
	uint64_t timer = mtime();
	walkfiles(s_project_directory, syntax_audit);

	HeaderLinkList* header = s_header_links;
	while (header) {
		PathList* primary = header->link->links;
		while (primary) {
			if (header_link_exists(primary->str)) {
				get_in_depth_headers(primary->str, header);
			}
			primary = (PathList*)primary->next;
		}
		header = (HeaderLinkList*)header->next;
	}
	header = s_header_links;
	while (header) {
		PathList* primary = header->link->links;
		while (primary) {
			PathList* secondary = header->link->secondaries;
			while (secondary) {
				if (strcmp(primary->str, secondary->str) == 0) {
					if (header_link_exists(primary->str)) {
						print("Useless include detected from \"%s\" - \"%s\" is not needed", header->link->header, primary->str);
					} else {
						print("Useless include detected from \"%s\" - <%s> is not needed", header->link->header, primary->str);
					}
					s_vulnerabilities++;
					break;
				}
				secondary = (PathList*)secondary->next;
			}
			primary = (PathList*)primary->next;
		}
		header = (HeaderLinkList*)header->next;
	}
	HeaderLinkList* source = s_source_links;
	while (source) {
		char buff[PATHLEN] = { 0 };
		strcpy(buff, source->link->header);
		buff[strlen(source->link->header) - 1] = 'h';
		PathList* inc = source->link->links;
		while (inc) {
			HeaderLinkList* curr = s_header_links;
			while (curr) {
				if (strcmp(curr->link->header, buff) == 0) {
					PathList* secondary = curr->link->secondaries;
					while (secondary) {
						if (strcmp(inc->str, secondary->str)) {
							if (header_link_exists(inc->str)) {
								print("Useless include detected from \"%s\" - \"%s\" is not needed", source->link->header, inc->str);
							} else {
								print("Useless include detected from \"%s\" - <%s> is not needed", source->link->header, inc->str);
							}
							s_vulnerabilities++;
							break;
						}
						secondary = (PathList*)secondary->next;
					}
				}
				curr = (HeaderLinkList*)curr->next;
			}
			inc = (PathList*)inc->next;
		}
		source = (HeaderLinkList*)source->next;
	}

	timer = mtime() - timer;
	int hours = (int)(timer / 3600000);
	int minutes = (int)((timer - (hours * 3600000)) / 60000);
	float seconds = (((float)timer) - (hours * 3600000) - (minutes * 60000)) / 1000.0f;
	print("Finished audit in %d:%d:%.3f - detected %s%d\033[0m vulnerabilities", 
	   hours, minutes, seconds, 
	   s_vulnerabilities == 0 ? "\033[32m" : s_vulnerabilities <= 10 ? "\033[33m" : "\033[31m", 
	   s_vulnerabilities);
	clean_header_links();
	clean_source_links();
}

int main(int argc, char* argv[]) {
	if (argc == 2 && strcmp(argv[1], "-v") == 0) {
		print("TINY BUILDER \033[32mv%d.%d.%d\033[0m authored by Jason Heflinger (https://github.com/JHeflinger)", VERSION, MAJOR_RELEASE, MINOR_RELEASE);
		return 0;
	}
	initialize(argc, argv);
	add_vendors();
	if (s_flags & AUDIT) audit();
	compile_vendors();
	calculate_dependencies();
	compile_objects();
	if (!s_sources_up_to_date || !fexists("build/bin.exe") || !s_main_up_to_date) {
		compile_executable();
	} else {
		print("Current build is \033[32mup to date\033[0m, no need to build executable");
	}
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
