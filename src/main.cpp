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

const char* getStringFromGlobal(lua_State* state, const char* global) {
	lua_getglobal(state, global);
	int result = lua_isnil(state, -1);
	if (result != 0) {
		throw std::runtime_error("build_config file was missing the ProjectName variable");
	}
	result = lua_isstring(state, -1);
	if (result == 0) {
		throw std::runtime_error("ProjectName in build_config must be a string value");
	}
	const char* retVal = lua_tostring(state, -1);
	lua_pop(state, 1);
	return retVal;
}

class CompileLua {
private:
	static CompileLua* instance;
public:
	static CompileLua& Instance();
	char* exeDir;
	const char* projectName;
	char* outPath;
	int scriptIndex = 0;
	const char* createLuaJITString() {
		scriptIndex++;
		const char* retVal;
		char* input = new char[strlen(exeDir) + strlen(outPath) + 64];
#if defined(_WIN32) || defined(_WIN64)
		const char* slash = "\\";
#else
		const char* slash = "/";
#endif
		sprintf(input, "%sbin%sluajit -b \"%s\" \"%slib%slua%sscripts%s%09i.raw\"", exeDir, slash, "%s", outPath, slash, slash, slash, scriptIndex);
		retVal = input;

		return retVal;
	}

	//    friend CompileLua& CompileLua_Instance();
};

CompileLua* CompileLua::instance;

CompileLua& CompileLua::Instance() {
	if (CompileLua::instance == nullptr) {
		CompileLua::instance = new CompileLua();
	}
	return *CompileLua::instance;
}

int main(int carg, char* args[]) {

	std::cout << args[0] << std::endl;
	char* path = 0;
	char* outPath = 0;

	char* exeDir = new char[strlen(args[0]) + 1];
	strcpy(exeDir, args[0]);

	char* scriptsPath = 0;
	char* libPath = 0;
	char* resPath = 0;

	bool found = false;
	for (size_t c = strlen(args[0]); c > 0; c--) {
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
	if (!found) {
		exeDir[0] = 0;
	}

	std::unordered_map<std::string, const char *> buildConfigVars = {
		{"ProjectName", 0},
		{"Identifier", "com.Unidentified.Untitled"},
		{"Version", "1.0"},
		{"ExecutableName", "build"},
		{"VulkanSDKPath", getenv("VULKAN_SDK")},
	};

	option opt = option::closeApp;

	try {

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
					if (opt != option::openProj) {

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
											path = new char[wcslen(pFilePath) + 1];
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
				}
				else { // gui
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
					if (i + 1 < carg) {
						path = new char[strlen(args[i + 1]) + 1];
						strcpy(path, args[i + 1]);

						opt = option::openProj;
						continue;
					}
					else {
						throw std::runtime_error("No path provided for opening");
					}
				}
			}

			if (strcmp(args[i], "-b") == 0 || strcmp(args[i], "--build") == 0) {
				if (i + 2 < carg) {
					opt = option::buildProj;
					path = new char[strlen(args[i + 1]) + 1];
					outPath = new char[strlen(args[i + 2]) + 1];

					strcpy(path, args[i + 1]);
					strcpy(outPath, args[i + 2]);
					break;
				}
				else {
					throw std::runtime_error("Building requires an input and an output path");
				}
			}

		} //for loop


		switch (opt)
		{
		case option::closeApp:
			return 0;
		case option::buildProj: {

			scriptsPath = new char[strlen(path) + 10];
			resPath = new char[strlen(path) + 14];
			libPath = new char[strlen(path) + 12];

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

			std::cout << scriptsPath << std::endl << resPath << std::endl << libPath << std::endl;

			lua_State* state = luaL_newstate();
			char* buildConfigPath = new char[strlen(path) + 18];
			strcpy(buildConfigPath, path);
			fs::addOSSlash(buildConfigPath);
			strcat(buildConfigPath, "build_config.lua");
			std::cout << buildConfigPath << std::endl;

			int result = luaL_dofile(state, buildConfigPath);

			if (result != 0) {
				auto message = lua_tostring(state, -1);
				puts(message);
				lua_pop(state, 1);
				throw std::runtime_error("Error loading build_config.lua file, open your project to regenerate the buildConfig file if there are nil fields.");
			}

			const char* executable = getStringFromGlobal(state, "ExecutableName");
			const char* projectName = getStringFromGlobal(state, "ProjectName");
			const char* identifier = getStringFromGlobal(state, "Identifier");
			const char* version = getStringFromGlobal(state, "Version");
			char* _outPath = new char[strlen(outPath) + strlen(projectName) + 24];

#if defined(_WIN32) || defined(_WIN64)
			sprintf(_outPath, "%s\\%s\\", outPath, projectName);

			std::string makeDir = std::string(_outPath) + std::string("/lib/lua/scripts");
			std::filesystem::create_directories(makeDir);

			if (!std::filesystem::exists(makeDir)) {
				throw std::runtime_error("Failed to make build directory!");
			}
			std::filesystem::copy(std::string(exeDir) + std::string("/lib/lua"), std::string(_outPath) + std::string("/lib/lua"), std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive );

#elif defined(LUTHERIE_MAC)
			sprintf(_outPath, "%s/%s.app/Contents/MacOS/", outPath, projectName);

			char* makeDir = new char[strlen(_outPath) * 2 + strlen(exeDir) + 1024];
			sprintf(makeDir, "mkdir -p %slib/lua/scripts && cp -r %slib/lua/ %slib/lua/", _outPath, exeDir, _outPath);
			system(makeDir);

            delete[] makeDir;
            
			char* compileCommand = new char[strlen(outPath) + strlen(projectName) + strlen(executable) + strlen(exeDir) + 1024];
			sprintf(compileCommand, "g++ -std=c++17 -pagezero_size 10000 -image_base 100000000 -o %s%s %smodules/main.cpp -framework Cocoa -framework IOKit -framework CoreFoundation -framework CoreVideo -L%slib/static -llutherie -lECS -lECSlua -lglfw3 -L%slib -lluajit -I%s/include", _outPath, executable, exeDir, exeDir, exeDir, exeDir);
			printf("%s \n", compileCommand);
			result = system(compileCommand);
			if (result < 0) {
				throw std::runtime_error("Project failed to compile");
			}
			delete[] compileCommand;
			std::cout << std::string(_outPath) + std::string("../Info.plist") << std::endl;
			std::fstream infoPlist;
			infoPlist.open(std::string(_outPath) + std::string("../Info.plist"), std::ios::in | std::ios::out | std::ios::trunc);
			infoPlist << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
			infoPlist << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" << std::endl;
			infoPlist << "<plist version=\"1.0\">" << std::endl;
			infoPlist << "<dict>" << std::endl;
			infoPlist << "\t<key>CFBundleDevelopmentRegion</key>" << std::endl;
			infoPlist << "\t<string>en</string>" << std::endl;
			infoPlist << "\t<key>CFBundleExecutable</key>" << std::endl;
			infoPlist << "\t<string>" << executable << "</string>" << std::endl;
			infoPlist << "\t<key>CFBundleIdentifier</key>" << std::endl;
			infoPlist << "\t<string>" << identifier << "</string>" << std::endl;
			infoPlist << "\t<key>CFBundleInfoDictionaryVersion</key>" << std::endl;
			infoPlist << "\t<string>6.0</string>" << std::endl;
			infoPlist << "\t<key>CFBundleName</key>" << std::endl;
			infoPlist << "\t<string>" << projectName << "</string>" << std::endl;
			infoPlist << "\t<key>CFBundlePackageType</key>" << std::endl;
			infoPlist << "\t<string>APPL</string>" << std::endl;
			infoPlist << "\t<key>CFBundleSupportedPlatforms</key>" << std::endl;
			infoPlist << "\t<array>" << std::endl;
			infoPlist << "\t\t<string>MacOSX</string>" << std::endl;
			infoPlist << "\t</array>" << std::endl;
			infoPlist << "\t<key>CFBundleDisplayName</key>" << std::endl;
			infoPlist << "\t<string>" << projectName << "</string>" << std::endl;
			infoPlist << "\t<key>CFBundleVersion</key>" << std::endl;
			infoPlist << "\t<string>" << version << "</string>" << std::endl;
			infoPlist << "\t<key>NSPrincipalClass</key>" << std::endl;
			infoPlist << "\t<string>NSApplication</string>" << std::endl;
			infoPlist << "</dict>" << std::endl;
			infoPlist << "</plist>" << std::endl;
			infoPlist.close();

#endif    

			CompileLua& clObj = CompileLua::Instance();
			clObj.exeDir = exeDir;
			clObj.outPath = _outPath;
			clObj.projectName = projectName;
			void(*f)(const char*) = [](const char* p) -> void {
				const char* luaJITCmd = CompileLua::Instance().createLuaJITString();
				std::cout << luaJITCmd << std::endl;

				char* compileLua = new char[strlen(luaJITCmd) + strlen(p)];
				sprintf(compileLua, luaJITCmd, p);
				std::cout << compileLua << std::endl;
				int result = system(compileLua);
				if (result < 0) {
					throw std::runtime_error(
						std::string("LuaJIT failed to get bytecode for ") + std::string(p)
					);
				}
				delete[] compileLua;
			};
			std::cout << scriptsPath << std::endl;
#if defined(_WIN32) || defined(_WIN64)
			for (std::filesystem::directory_entry it : std::filesystem::recursive_directory_iterator(scriptsPath)) {
				(*f)(it.path().string().c_str());
			}
#else
			fs::doOnFilesInDir(scriptsPath, f);
#endif
			delete &clObj;

			lua_close(state);
			delete[] _outPath;

			delete[] exeDir;
			delete[] scriptsPath;
			delete[] resPath;
			delete[] libPath;
			delete[] path;
			delete[] outPath;

			return 0;
		}
		case option::openProj: {

			scriptsPath = new char[strlen(path) + 10];
			resPath = new char[strlen(path) + 14];
			libPath = new char[strlen(path) + 12];

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
<<<<<<< HEAD
			char newPath = new char[strlen(path)+1];
=======
			char* newPath = new char[strlen(path)+1];
>>>>>>> 6bb57f78cb71319c166d057817ceba6be67fa6a4

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

			char* parentPath = new char[strlen(path) + 1];
			strcpy(parentPath, path);
#if defined(_WIN32) || defined(_WIN64)
			const char* slash = "\\";
#else
			const char* slash = "/";
#endif

			char* token = strtok(parentPath, slash);
			char* parentFolder = 0;

			while (token) {
				parentFolder = token;
				token = strtok(NULL, slash);
			}

			lua_State* state = luaL_newstate();
			char* buildConfigPath = new char[strlen(path) + 18];
			strcpy(buildConfigPath, path);
			fs::addOSSlash(buildConfigPath);
			strcat(buildConfigPath, "build_config.lua");

			int result = luaL_dofile(state, buildConfigPath);

			if (result != 0) {
				auto message = lua_tostring(state, -1);
				puts(message);
			}
			for (auto it = buildConfigVars.begin(); it != buildConfigVars.end(); ++it) {

				lua_getglobal(state, it->first.c_str());
				int result = lua_isnil(state, -1);
				if (result == 0) {
					result = lua_isstring(state, -1);
					//std::cout << result << std::endl;
					if (result == 1) {
						it->second = lua_tostring(state, -1);
						std::cout << it->first << " " << it->second << std::endl;

					}
				}
			}

			if (buildConfigVars["ProjectName"] == NULL) {
				buildConfigVars["ProjectName"] = parentFolder;
			}

			std::fstream buildConfig;
			buildConfig.open(buildConfigPath, std::ios::in | std::ios::out | std::ios::trunc);
			for (auto it = buildConfigVars.begin(); it != buildConfigVars.end(); ++it) {
				std::string string = it->second;
				size_t n = string.find("\\");
				while (n != std::string::npos) {
					string.replace(n, 1, "\\\\");
					n = string.find("\\", n + 2);
				}
				buildConfig << it->first << " = \"" << string << "\"" << std::endl;
			}
			buildConfig.close();

			delete[] buildConfigPath;
			delete[] parentPath;

			lua_close(state);
			Lutherie lutherie = Lutherie(exeDir, scriptsPath, resPath, libPath);

			delete[] exeDir;
			delete[] scriptsPath;
			delete[] resPath;
			delete[] libPath;
			delete[] path;

			break; //option::openProj
		}
		}
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}
