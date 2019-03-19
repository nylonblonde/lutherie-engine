#ifndef fs_h
#define fs_h

#include <stdio.h>
#include <fstream>
#include <cstring>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>

namespace fs {

    static int addOSSlash(char* string){
#if defined(_WIN32) || defined(_WIN64)
        if(string[strlen(string)-1] != '\\') { strcat(string, "\\"); }
#else
        if(string[strlen(string)-1] != '/') { strcat(string, "/"); }
#endif
        return 0;
    }
    
#if defined(__APPLE__) && defined(__MACH__)
    static bool pathExists(const char* path){
        struct stat info;
        
        if(stat(path, &info) != 0){
            return false;
        }else if(info.st_mode & S_IFDIR){
            return true;
        }
        
        return false;
    }

    static bool makePath(const char* path){
        
        char newPath[strlen(path)];
        strcpy(newPath, path);
        
        for(int i = strlen(path)-1; i > 0; --i){
            if(path[i] == '\0'){
                continue;
            }
            
            int dashed = (path[i] == '/');
            
            if(dashed){
                if(i == strlen(path)){
                    continue;
                }
                
                if(mkdir(newPath, 0777) == 0){
                    makePath(path);
                    break;
                }else if(errno == EEXIST) {
                    break;
                }
                
                strncpy(newPath, path, i);
                newPath[i] = '\0';
            }
        }
        
        if(pathExists(path)){
            return true;
        }
        
        return false;
    }
    
    static bool addSubDirectory(const char* path, const char* subdir){
        char subdirPath[strlen(path)+strlen(subdir)+1];
        
        int needsSlash = path[strlen(path)] != '/' && subdir[0] != '/';
        
        strcpy(subdirPath, path);
        
        if(needsSlash == 1){
            addOSSlash(subdirPath);
        }
        
        strcat(subdirPath, subdir);
        
        printf("%s\n", subdirPath);
        
        return makePath(subdirPath);
    }
#endif // defined(__APPLE__) && defined(__MACH__)

}
#endif /* fs_hpp */
