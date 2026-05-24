#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>

#include "fileio.h"
#include "log.h"

bool file_exists(const char* path) {
    return (access(path, F_OK) == 0);
}

bool dir_exists(const char* path) { 
    struct stat sb;
    if(lstat(path, &sb) != 0) {
        return false;
    }
    return (sb.st_mode & S_IFDIR);
}


bool mkdir_p(const char* path, mode_t perm) {
    bool result = false;
    
    if(!path) {
        goto out;
    }

    const size_t path_length = strlen(path);

    char buffer[255] = { 0 };
    size_t buffer_idx = 0;

    for(size_t i = 0; i < path_length; i++) {
        char ch = path[i];

        
        buffer[buffer_idx++] = ch;
        if(buffer_idx >= sizeof(buffer)) {
            fprintf(stderr, "%s: The path is too long\n", __func__);
            goto out;
        }

        if(((i > 0) && (ch == '/')) 
        || (i+1 >= path_length)) {
            if(!dir_exists(buffer)) {
                if(mkdir(buffer, perm) != 0) {
                    fprintf(stderr, "%s: \"%s\" %s\n", __func__, buffer, strerror(errno));
                    goto out;
                }
            }
        }
    }
    
    result = true;

out:
    return result;
}

ssize_t file_size(const char* path) {
    struct stat sb;
    if(lstat(path, &sb) < 0) {
        fprintf(stderr, "%s: lstat() | %s\n", __func__, strerror(errno));
        return -1;
    }

    return sb.st_size;
}

bool map_file(const char* path, int prot, char** out, size_t* out_size) {
    bool result = false;
        
    int mmap_flags = 0;
    int open_flags = 0;

    if((prot & PROT_WRITE) && !(prot & PROT_READ)) {
        open_flags = O_RDWR;
        mmap_flags = MAP_SHARED;
    }
    else
    if((prot & PROT_READ) && !(prot & PROT_WRITE)) {
        open_flags = O_RDONLY;
        mmap_flags = MAP_PRIVATE;
    }
    else
    if((prot & PROT_WRITE) && (prot & PROT_READ)) {
        open_flags = O_RDWR;
        mmap_flags = MAP_SHARED;
    }

    int fd = open(path, open_flags);
    struct stat sb;


    if(fd < 0) {
        fprintf(stderr, "%s: open() | %s\n", __func__, strerror(errno));
        goto out;
    }

    if(fstat(fd, &sb) < 0) {
        fprintf(stderr, "%s: fstat() | %s\n", __func__, strerror(errno));
        goto out;
    }
    
    *out_size = sb.st_size;

    if(sb.st_size == 0) {
        fprintf(stderr, "%s: Not mapping empty file \"%s\"\n", __func__, path);
        goto out;
    }

    if(out) {
        *out = mmap(NULL, sb.st_size, prot, mmap_flags, fd, 0);
        if(*out == MAP_FAILED) {
            fprintf(stderr, "%s: mmap() | %s\n", __func__, strerror(errno));
            goto out;
        }
    }

    result = true;

out:

    if(fd > 0) {
        close(fd);
    }

    return result;
}


bool write_file(const char* path, void* data, size_t size) {
    bool result = false;
    if(!file_exists(path)) {
        goto out;
    }

    int fd = open(path, O_WRONLY | O_TRUNC);
    if(fd < 0) {
        fprintf(stderr, "%s: open() | %s\n", __func__, strerror(errno));
        goto out;
    }

    if(write(fd, data, size) < 0) {
        fprintf(stderr, "%s: write() | %s\n", __func__, strerror(errno));
        goto close_and_out;
    }


    result = true;

close_and_out:
    close(fd);
out:
    return result;
}

char* file_magic_bytes(const char* path, size_t num_read_bytes) {
    char* magic_bytes = NULL;

    int fd = open(path, O_RDONLY);
    if(fd < 0) {
        fprintf(stderr, "%s: open() | %s\n", __func__, strerror(errno));
        goto out;
    }

    magic_bytes = malloc(num_read_bytes * sizeof *magic_bytes);
    read(fd, magic_bytes, num_read_bytes * sizeof *magic_bytes);

    close(fd);
out:
    return magic_bytes;
}

FileInfo* nmt_list_files(const char* dirpath, size_t* num_files) {
    assert(dirpath != NULL);
    assert(num_files != NULL);
    
    size_t dirpath_len = strlen(dirpath);
    assert(dirpath_len != 0);

    FileInfo* files = NULL;
   

    DIR* dir = opendir(dirpath);
    if(!dir) {
        logprintf(LOG_ERROR, "Could not open '%s' | %s", dirpath, strerror(errno));
        return NULL;
    }

    size_t files_allocated = 3;
    files = malloc(sizeof *files * files_allocated);
    *num_files = 0;

    struct dirent* ent = NULL;
    while((ent = readdir(dir))) {

        if(*num_files + 1 >= files_allocated) {
            files_allocated = *num_files + 8;
            files = realloc
            (
                files,
                files_allocated * sizeof *files
            );
        }
        


        FileInfo* finfo = &files[*num_files];
        *num_files += 1;
    
        memset(finfo->name, 0, sizeof(finfo->name));
        memset(finfo->full_path, 0, sizeof(finfo->full_path));

        size_t name_len = strlen(ent->d_name);
        size_t full_path_len  = dirpath_len;
        
        memcpy(finfo->name, ent->d_name, name_len);
        memcpy(finfo->full_path, dirpath, dirpath_len);

        if(finfo->full_path[ full_path_len-1 ] != '/') {
            finfo->full_path[ full_path_len ] = '/';
            full_path_len++;
        }
        memcpy(finfo->full_path + full_path_len, ent->d_name, name_len);
        
        stat(finfo->full_path, &finfo->sb);
    }

    closedir(dir);
    return files;
}


