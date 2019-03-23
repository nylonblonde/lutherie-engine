#define LUTHERIE_VULKAN

#include "lutherie.hpp"

using namespace ECS;

struct MyComponent : Component {
    int Value;
};

class MySystem : public System {

public:
    ComponentGroup group = ComponentGroup::createComponentGroup<MyComponent>(*this);

    MySystem(World& w) : System(w) {
        
    }
    
    virtual void OnUpdate(){
//        std::cout << group.size() << std::endl;
    }
    
};

int main(int carg, char* args[]){
    
    std::cout << args[0] << std::endl;
    
    if(carg < 2){
        std::cout << "Usage: Lutherie [options [args]]" << std::endl;
        std::cout << "Available options:" << std::endl;
        std::cout << "-o | --open   <path-to-project>   Opens a Lutherie project at path destination or creates one if directory doesn't contain one" << std::endl;
        return 0;
    }
    
    for(int i = 1; i < carg; i+=2){
        if(strcmp(args[i], "-o") == 0 || strcmp(args[i], "--open") == 0){
			char* path = args[i + 1];

			char scriptsDir[9] = "scripts";
			char resDir[11] = "resources";
			char libDir[6] = "libs";

			fs::addOSSlash(scriptsDir);
			fs::addOSSlash(resDir);
			fs::addOSSlash(libDir);

#if defined (LUTHERIE_MAC) || defined (__unix__)
            char newPath[strlen(path)];
            
            if(strncmp(path, "~", 1) == 0){
                char* home = getenv("HOME");
                strcpy(newPath, home);
                strcat(newPath, &path[1]);
                
                path = newPath;
                
            }
            
            std::cout << path << std::endl;
            
            if(fs::makePath(path)){
                
                char scriptsDir[] = "scripts/";
                char resDir[] = "resources/";
                char libDir[] = "libs/";
                
                if(fs::addSubDirectory(path, scriptsDir) && fs::addSubDirectory(path, resDir) && fs::addSubDirectory(path, libDir)){
                    char scriptsPath[strlen(path)+strlen(scriptsDir)+1];
                    char resPath[strlen(path)+strlen(resDir)+1];
                    char libPath[strlen(path)+strlen(libDir)+1];

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
                    
                    for(int c = strlen(args[0]); c > 0; c--){
                        if(exeDir[c] == '/'){
                            exeDir[c+1] = 0;
                            break;
                        }
                    }
                    
                    Lutherie lutherie = Lutherie(exeDir, scriptsPath, resPath, libPath);
                }
            }

#else // not LUTHERIE_MAC or __unix__
			fs::addOSSlash(path);
			std::string scriptsPath = std::string(path) + std::string(scriptsDir);
			std::string resPath = std::string(path) + std::string(resDir);
			std::string libPath = std::string(path) + std::string(libDir);

			std::filesystem::create_directories(scriptsPath);
			std::filesystem::create_directories(resPath);
			std::filesystem::create_directories(libPath);

			std::cout << scriptsPath << std::endl << resPath << std::endl << libPath << std::endl;
			if (std::filesystem::exists(scriptsPath) &&	std::filesystem::exists(resPath) &&	std::filesystem::exists(libPath)) {
				
                std::string exeDir = std::string(args[0]);
                
            #if defined(_WIN32) || defined(_WIN64)
                exeDir.erase(path.find_last_of('\\'), string::string::npos);
            #else
                exeDir.erase(path.find_last_of('/'), string::string::npos);
            #endif
                
				Lutherie lutherie = Lutherie(exeDir, scriptsPath.c_str(), resPath.c_str(), libPath.c_str());
			}
			
#endif
			
            return 0;
        }
    }

    
    
    return 0;
}
