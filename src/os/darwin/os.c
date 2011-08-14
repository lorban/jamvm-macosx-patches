/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2009
 * Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/sysctl.h>
#include <pthread.h>
#include <Carbon/Carbon.h>

#include "../../jam.h"

void *nativeStackBase() {
    return pthread_get_stackaddr_np(pthread_self());
}

int nativeAvailableProcessors() {
    int processors, mib[2];
    size_t len = sizeof(processors);

    mib[0] = CTL_HW;
    mib[1] = HW_AVAILCPU;

    if(sysctl(mib, 2, &processors, &len, NULL, 0) == -1)
        return 1;
    else
        return processors;
}

char *nativeLibError() {
    return dlerror();
}

char *nativeLibPath() {
    return getenv("DYLD_LIBRARY_PATH");
}

/* GNU Classpath's libraries end in .dylib because it
   uses libtool, but JNI libraries normally end in
   .jnilib under Mac OS X.  We try both.

   On Mac OS X/Intel libtool seems to use a .so ending.
   This is wrong, but a workaround for now is to _also_
   try .so!

   OpenJDK seems to already apppend .dylib to the path name
   so we also try without prefix
*/

void *nativeLibOpen(char *path) {
    void *handle;

    if((handle = dlopen(path, RTLD_LAZY)) == NULL) {
        int len = strlen(path);
        char buff[len + sizeof(".jnilib") + 1];
     
        strcpy(buff, path);
        strcpy(buff + len, ".dylib");

        if((handle = dlopen(buff, RTLD_LAZY)) == NULL) {
            strcpy(buff + len, ".jnilib");

            if((handle = dlopen(buff, RTLD_LAZY)) == NULL) {
                strcpy(buff + len, ".so");

                handle = dlopen(buff, RTLD_LAZY);
            }
        }
    }
    
    return handle;
}

void nativeLibClose(void *handle) {
    dlclose(handle);
}

void *nativeLibSym(void *handle, char *symbol) {
    return dlsym(handle, symbol);
}

char *nativeLibMapName(char *name) {
   char *buff = sysMalloc(strlen(name) + sizeof("lib") + 1);

   sprintf(buff, "lib%s", name);
   return buff;
}

char *nativeJVMPath() {
    Dl_info info;
    char *path;

    if(dladdr(nativeJVMPath, &info) == 0) {
        printf("Error: dladdr failed.  Aborting VM\n");
        exitVM(1);
    }

    path = sysMalloc(strlen(info.dli_fname) + 1);
    strcpy(path, info.dli_fname);

    return path;
}


long long getPhysicalMemory() {
    /* Long longs are used here because with PAE, a 32-bit
       machine can have more than 4GB of physical memory */

    // On Mac OS X, sysctl with selectors CTL_HW, HW_PHYSMEM returns only a 
    // 4-byte value, even if passed an 8-byte buffer, and limits the returned 
    // value to 2GB when the actual RAM size is > 2GB.  The Gestalt selector 
    // gestaltPhysicalRAMSizeInMegabytes is available starting with OS 10.3.0.
    long mem_size;
    if (Gestalt(gestaltPhysicalRAMSizeInMegabytes, &mem_size)) {
        perror("Error: Couldn't determine physical RAM size.  Aborting VM\n");
        exitVM(1);
    }
    return ((long long)(1024 * 1024)) * mem_size;
}
