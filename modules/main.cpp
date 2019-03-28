#include <lutherie.hpp>
#include <fs.h>

int main(int carg, char* args[]){
    
    char* exeDir = new char[strlen(args[0])+1];
    strcpy(exeDir, args[0]);
    char* scriptsPath = 0;
    char* libPath = 0;
    char* resPath = 0;

    bool found = false;
    for (int c = strlen(args[0]); c > 0; c--) {
        #if defined(_WIN32) || defined(_WIN64)
        if (exeDir[c] == '\\') {
        #else
        if (exeDir[c] == '/') {
        #endif
            exeDir[c + 1] = 0;
            found = true;
            break;
        #if defined(_WIN32) || defined(_WIN64)
        }
        #else
        }
        #endif
    }
    if(!found) {
        exeDir[0] = 0;
    }

    scriptsPath = new char[strlen(exeDir)+10];
    strcpy(scriptsPath, exeDir);
    fs::addOSSlash(scriptsPath);
    strcat(scriptsPath, "scripts");
    fs::addOSSlash(scriptsPath);

    libPath = new char[strlen(exeDir)+6];
    strcpy(libPath, exeDir);
    fs::addOSSlash(libPath);
    strcat(libPath, "lib");
    fs::addOSSlash(libPath);
    
    resPath = new char[strlen(exeDir)+12];
    strcpy(resPath, exeDir);
    fs::addOSSlash(resPath);
    strcat(resPath, "resources");
    fs::addOSSlash(resPath);
    
    Lutherie lutherie = Lutherie(exeDir, scriptsPath, resPath, libPath);

	delete[] exeDir;
	delete[] scriptsPath;
	delete[] resPath;
	delete[] libPath;

}