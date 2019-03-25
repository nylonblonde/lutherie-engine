#define LUTHERIE_VULKAN

#if defined(LUTHERIE_MAC)
#include <Cocoa/Cocoa.h>
#include <unistd.h>
#elif defined(_WIN32) || defined(_WIN64)
//#include<io.h>
#include <shobjidl.h>
#endif

#include "lutherie.hpp"

enum option { closeApp = 0, openProj = 1 };

int main(int carg, char* args[]) {

	std::cout << args[0] << std::endl;
	char* path = 0;

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
			std::cout << carg << std::endl;

			bool gui = i > 0 && tty;

			if (!gui) {
#if defined(LUTHERIE_MAC)
				gui = (isatty(0) == 0);
#elif defined(_WIN32) || defined(_WIN64)
				//gui = (_isatty(0) == 0);
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
							path = const_cast<char*>(std::string([[nsurl path] UTF8String]).c_str());
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
				return 0;
			}
		}

		if (strcmp(args[i], "-o") == 0 || strcmp(args[i], "--open") == 0) {
			if (opt == option::openProj) {
				std::cout << "Project already chosen, ignoring open argument" << std::endl;
				continue;
			}
			else {
				std::cout << args[i] << " " << args[i + 1] << std::endl;
				path = args[i + 1];

				opt = option::openProj;
				continue;
			}
		}

		if (tty) {
			i--;
		}
	} //for loop

	switch (opt)
	{
	case option::closeApp:
		return 0;
	case option::openProj:
		char scriptsDir[10] = "";
		char resDir[12] = "";
		char libDir[7] = "";

		fs::addOSSlash(scriptsDir);
		fs::addOSSlash(resDir);
		fs::addOSSlash(libDir);

		strcat(scriptsDir, "scripts");
		strcat(resDir, "resources");
		strcat(libDir, "libs");

		fs::addOSSlash(scriptsDir);
		fs::addOSSlash(resDir);
		fs::addOSSlash(libDir);

#if defined (LUTHERIE_MAC) || defined (__unix__)
		char newPath[strlen(path)];

		if (strncmp(path, "~", 1) == 0) {
			char* home = getenv("HOME");
			strcpy(newPath, home);
			strcat(newPath, &path[1]);

			path = newPath;

		}

		std::cout << path << std::endl;

		if (fs::makePath(path)) {

			char scriptsDir[] = "scripts/";
			char resDir[] = "resources/";
			char libDir[] = "libs/";

			if (fs::addSubDirectory(path, scriptsDir) && fs::addSubDirectory(path, resDir) && fs::addSubDirectory(path, libDir)) {
				char scriptsPath[strlen(path) + strlen(scriptsDir) + 1];
				char resPath[strlen(path) + strlen(resDir) + 1];
				char libPath[strlen(path) + strlen(libDir) + 1];

				strcpy(scriptsPath, path);
				strcpy(resPath, path);
				strcpy(libPath, path);

				fs::addOSSlash(scriptsPath);
				fs::addOSSlash(resPath);
				fs::addOSSlash(libPath);

				strcat(scriptsPath, scriptsDir);
				strcat(resPath, resDir);
				strcat(libPath, libDir);

				//                     World& world = World::createWorld<MySystem>();

				//                    auto start = std::chrono::high_resolution_clock::now();
				//                    for(int i = 0; i < 10000; i++){
				//                        Entity entity = world.createEntity();
				//                        world.setComponent(entity, new MyComponent());
				//                    }
				//                    auto end = std::chrono::high_resolution_clock::now();
				//                    double duration = std::chrono::duration_cast<std::chrono::duration<double>>(end-start).count();
				//                    std::cout << "C++ elapsed: " << duration << std::endl;

				char exeDir[strlen(args[0])];
				strcpy(exeDir, args[0]);

				for (int c = strlen(args[0]); c > 0; c--) {
					if (exeDir[c] == '/') {
						exeDir[c + 1] = 0;
						break;
					}
				}

				Lutherie lutherie = Lutherie(exeDir, scriptsPath, resPath, libPath);
			}
		}

#else // not LUTHERIE_MAC or __unix__
		std::string scriptsPath = std::string(path) + std::string(scriptsDir);
		std::string resPath = std::string(path) + std::string(resDir);
		std::string libPath = std::string(path) + std::string(libDir);

		std::filesystem::create_directories(scriptsPath);
		std::filesystem::create_directories(resPath);
		std::filesystem::create_directories(libPath);

		std::cout << scriptsPath << std::endl << resPath << std::endl << libPath << std::endl;
		if (std::filesystem::exists(scriptsPath) && std::filesystem::exists(resPath) && std::filesystem::exists(libPath)) {

			std::string exeDir = std::string(args[0]);

#if defined(_WIN32) || defined(_WIN64)
			if (exeDir.find_last_of('\\') != std::string::npos) {
				exeDir = exeDir.substr(0, exeDir.find_last_of('\\'));
			}
			else {
				exeDir = "";
			}
#else
			exeDir.erase(exeDir.find_last_of('/'), std::string::npos);
#endif

			Lutherie lutherie = Lutherie(exeDir.c_str(), scriptsPath.c_str(), resPath.c_str(), libPath.c_str());
		}

#endif

		break;
	}

	return 0;
}
