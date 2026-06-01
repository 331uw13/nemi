/* License: zlib

   Copyright (C) 2026 (331uw13/eeiuwie)

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
*/
#ifndef FILEIO_H
#define FILEIO_H

#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <limits.h>

bool file_exists(const char* path);
bool dir_exists(const char* path);

// Behaves similarly to command "mkdir -p"
// If the parent directories do not exists for 'path'
// they will be created with 'perm' permissions. <-  see "open" manual pages under O_CREAT for modes.
bool mkdir_p(const char* path, mode_t perm); 

// Maps file into memory with 'prot' access.
// 'prot' can one or both flags: PROT_READ, PROT_WRITE
//
// Remember to synchronize the file with 'msync' if modifying the buffer
// to avoid undefined reads in the future for the file.
bool map_file(const char* path, int prot, char** out, size_t* out_size);

bool write_file(const char* path, void* data, size_t size);

// On error returns -1 otherwise the file size.
ssize_t file_size(const char* path);

// 'NULL' is returned if no such file was opened succesfully.
// Otherwise returned address must be freed.
char* file_magic_bytes(const char* path, size_t num_read_bytes);


typedef struct FileInfo_t {
    char name [256];
    char full_path [PATH_MAX * 2];
    struct stat sb;
}
FileInfo;

FileInfo* nmt_list_files(const char* dirpath, size_t* num_files);



#endif
