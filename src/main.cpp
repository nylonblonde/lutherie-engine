#define LUTHERIE_VULKAN

#if defined(LUTHERIE_MAC)
    #include <Cocoa/Cocoa.h>
    #include <unistd.h>
#elif defined(_WIN32) || defined(_WIN64)
    #include <shobjidl.h>
#endif

#include<luajit-2.0/lua.hpp>
#include<fstream>
#include<unordered_map>

#include <lutherie.hpp>

enum option { closeApp = 0, openProj = 1, buildProj = 2 };

const char* getStringFromGlobal(lua_State* state, const char* global){
    lua_getglobal(state, global);
    int result = lua_isnil(state, -1);
    if(result != 0){
        throw std::runtime_error("buildConfig file was missing the ProjectName variable");
    }
    result = lua_isstring(state, -1);
    if(result == 0){
        throw std::runtime_error("ProjectName in buildConfig must be a string value");
    }
    const char* retVal = lua_tostring(state, -1);
//    char* retVal = new char[strlen(str)];
//    strcpy(retVal, str);
    lua_pop(state, 1);
    return retVal;
}

int main(int carg, char* args[]) {
    
	std::cout << args[0] << std::endl;
	char* path = 0;
    char* outPath = 0;
    
    char* exeDir = new char[strlen(args[0])];
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
    
    std::unordered_map<const char *, const char *> buildConfigVars = {
        {"ProjectName", 0},
        {"VersionNumber", "1.0"},
        {"Identifier", "com.Unidentified.Untitled"},
        {"BuildVersion", "1"},
        {"ExecutableName", "build"},
        {"VulkanSDKPath", std::getenv("VULKAN_SDK")},
    };
    
	option opt = option::closeApp;

	for (int i = 0; i < carg; i += 2) {
		//goofy but we gotta do this so that we get the carg < 2 so that the open dialog can open with zero args
		if (i == 0) {
			i++;
		}

		bool tty = false;
		if (i < carg) {
			tty = strcmp(args[i], "-t") == 0 || strcmp(args[i], "--tty") == 0;
		}
		if (carg < 2 || tty) {

			bool gui = i > 0 && tty;

			if (!gui) {
#if defined(LUTHERIE_MAC)
				gui = (isatty(0) == 0);
#elif defined(_WIN32) || defined(_WIN64)
				DWORD pids[2];
				DWORD num_pids = GetConsoleProcessList(pids, sizeof(pids) / sizeof(*pids));
				gui = num_pids <= 1;
#endif
			}
			if (gui) {
				if(opt != option::openProj) {

				#if defined(LUTHERIE_MAC)
					NSOpenPanel *op = [NSOpenPanel openPanel];
					[op setCanChooseDirectories : YES];
					[op setCanChooseFiles : NO];
					[op setAllowsMultipleSelection : NO];
					[op setResolvesAliases : NO];
					[op setCanCreateDirectories : YES];
					[op setPrompt : @"Open Project Directory"];

                    if ([op runModal] == NSModalResponseOK) {
                        NSURL *nsurl = [[op URLs] objectAtIndex:0];
                        path = new char[strlen([[nsurl path] UTF8String])];
                        strcpy(path, [[nsurl path] UTF8String]);
                        opt = option::openProj;
                    }
				#elif defined(_WIN32) || defined(_WIN64)

					HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
					if (SUCCEEDED(hr)) {
						IFileOpenDialog *pOpen;

						hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pOpen));
						if (SUCCEEDED(hr)) {
							DWORD options = 0;
							pOpen->GetOptions(&options);
							pOpen->SetOptions(options | FOS_PICKFOLDERS);
							hr = pOpen->Show(NULL);

							if (SUCCEEDED(hr)) {
								IShellItem *pItem;
								hr = pOpen->GetFolder(&pItem);

								if (SUCCEEDED(hr)) {
									LPWSTR pFilePath;
									hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pFilePath);
									if (SUCCEEDED(hr)) {
										path = new char[wcslen(pFilePath)];
										wcstombs(path, pFilePath, strlen(path));
										opt = option::openProj;
										CoTaskMemFree(pFilePath);
									}
								}
								pItem->Release();
							}
						}
						pOpen->Release();
					}
					CoUninitialize();
				#endif
					continue;
				}
			} else { // gui
				std::cout << "Usage: lutherie [options [arg]]" << std::endl;
				std::cout << "Available options:" << std::endl;
				std::cout << "-o | --open	<path-to-project>	Opens a Lutherie project at path destination or creates one if directory doesn't contain one" << std::endl;
				std::cout << "-t | --tty	Runs the GUI version of the Lutherie Engine from the command line" << std::endl;
				std::cout << "-b | --build	<path-to-project> <output>	Builds the project as a standalone executable to the output path" << std::endl;

				return 0;
			}
            
            if (tty) {
                i--;
            }
		}

		if (strcmp(args[i], "-o") == 0 || strcmp(args[i], "--open") == 0) {
			if (opt == option::openProj) {
				std::cout << "Project already chosen, ignoring open argument" << std::endl;
				continue;
			}
			else {
				std::cout << args[i] << " " << args[i + 1] << std::endl;
                if(i + 1 < carg){
                    path = args[i + 1];

                    opt = option::openProj;
    				continue;
                } else {
                    throw std::runtime_error("No path provided for opening");
                }
			}
		}

        if(strcmp(args[i], "-b") == 0 || strcmp(args[i], "--build") == 0) {
            if(i + 2 < carg){
                opt = option::buildProj;
                path = args[i + 1];
                outPath = args[i + 2];
                break;
            } else {
                throw std::runtime_error("Building requires an input and an output path");
            }
        }
		
	} //for loop

    
	switch (opt)
	{
        case option::closeApp:
            return 0;
        case option::buildProj: {

            char* compileCommand = 0;    

            lua_State* state = luaL_newstate();
            char* buildConfigPath = new char[strlen(path)+12];
            strcpy(buildConfigPath, path);
            fs::addOSSlash(buildConfigPath);
            strcat(buildConfigPath, "buildConfig");
            int result = luaL_dofile(state, buildConfigPath);
            
            if(result != 0){
                throw std::runtime_error("Error loading buildConfig file, open your project to regenerate the buildConfig file");
            }
            
            
            const char* executable = getStringFromGlobal(state, "ExecutableName");

            const char* projectName = getStringFromGlobal(state, "ProjectName");            

        #if defined(LUTHERIE_MAC)
            compileCommand = new char[strlen(outPath) + strlen(executable) + strlen(exeDir) + 1024];

            sprintf(compileCommand, "g++ -std=c++17 -pagezero_size 10000 -image_base 100000000 -o %s/%s.app/Contents/MacOS/%s %smodules/main.cpp -framework Cocoa -framework IOKit -framework CoreFoundation -framework CoreVideo -L%slib/static -llutherie -lECS -lECSlua -lglfw3 -L%slib -lluajit -I%s/include", outPath, projectName, executable, exeDir, exeDir, exeDir, exeDir);
            printf("%s \n", compileCommand);
            
            char* makeDir = new char[strlen(outPath) + strlen(projectName) + 27];
            sprintf(makeDir, "mkdir -p %s/%s.app/Contents/MacOS", outPath, projectName);
            system(makeDir);
            delete[] makeDir;
        #endif    
            
            system(compileCommand);
            lua_close(state);

            return 0;
        }
        case option::openProj: {

            scriptsPath = new char[strlen(path) + 9];
            resPath = new char[strlen(path) + 13];
            libPath = new char[strlen(path) + 11];

            strcpy(scriptsPath, path);
            strcpy(resPath, path);
            strcpy(libPath, path);

            fs::addOSSlash(scriptsPath);
            fs::addOSSlash(resPath);
            fs::addOSSlash(libPath);

            strcat(scriptsPath, "scripts");
            strcat(resPath, "resources");
            strcat(libPath, "lib");

            fs::addOSSlash(scriptsPath);
            fs::addOSSlash(resPath);
            fs::addOSSlash(libPath);


    #if defined (LUTHERIE_MAC) || defined (__unix__)
            char newPath[strlen(path)];

            if (strncmp(path, "~", 1) == 0) {
                char* home = getenv("HOME");
                strcpy(newPath, home);
                strcat(newPath, &path[1]);

                path = newPath;
            }

            std::cout << path << std::endl;

            if (!fs::makePath(scriptsPath) || !fs::makePath(libPath) || !fs::makePath(resPath)) {
                throw std::runtime_error("Failed to make a path to the supplied directory");
            }

    #else // not LUTHERIE_MAC or __unix__

            std::filesystem::create_directories(scriptsPath);
            std::filesystem::create_directories(resPath);
            std::filesystem::create_directories(libPath);

            if (!std::filesystem::exists(scriptsPath) || !std::filesystem::exists(resPath) || !std::filesystem::exists(libPath)) {
                throw std::runtime_error("Error retrieving project subdirectories!");
            }

    #endif

            char* parentPath = new char[strlen(path)];
            strcpy(parentPath, path);
        #if defined(_WIN32) || defined(_WIN64)
			const char* slash = "\\";
        #else
            const char* slash = "/";
        #endif
            
            char* token = strtok(parentPath, slash);
            char* parentFolder = 0;
            
            while(token){
                parentFolder = token;
                token = strtok(NULL, slash);
            }
            
            if(buildConfigVars["ProjectName"] == NULL){
                buildConfigVars["ProjectName"] = parentFolder;
            }
            
            std::cout << parentFolder << std::endl;
            
            lua_State* state = luaL_newstate();
            char* buildConfigPath = new char[strlen(path)+12];
            strcpy(buildConfigPath, path);
            fs::addOSSlash(buildConfigPath);
            strcat(buildConfigPath, "buildConfig");

            int result = luaL_dofile(state, buildConfigPath);  
            if(result == 0) {
                for(auto& it : buildConfigVars){
                    lua_getglobal(state, it.first);
                    result = lua_isnil(state, -1);
                    if(result == 0){
                        result = lua_isstring(state, -1);
                        if(result == 1){
                            it.second = lua_tostring(state, -1);
                        }
                    }
                }
            }

            std::fstream buildConfig;
            buildConfig.open(buildConfigPath, std::ios::in | std::ios::out | std::ios::trunc);
            for(auto& it : buildConfigVars){
                buildConfig << it.first << " = \"" << it.second << "\"" << std::endl;
            }
            buildConfig.close();
            lua_close(state); 
            Lutherie lutherie = Lutherie(exeDir, scriptsPath, resPath, libPath);

            break; //option::openProj
        }
	}

	return 0;
}
