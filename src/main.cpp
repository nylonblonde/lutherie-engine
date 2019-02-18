#define LUTHERIE_VULKAN

#include "lutherie.hpp"
#include <fstream>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

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
        std::cout << group.size() << std::endl;
    }
    
};

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

    #if defined(_WIN32) || defined(_WIN64)
        int dashed = (path[i] == '\\');
    #else
        int dashed = (path[i] == '/');
    #endif

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
    
#if defined(_WIN32) || defined(_WIN64)
    int needsSlash = path[strlen(path)] != '\\' && subdir[0] != '\\';
#else
    int needsSlash = path[strlen(path)] != '/' && subdir[0] != '/';
#endif
    std::cout << needsSlash << std::endl;

    strcpy(subdirPath, path);

    if(needsSlash == 1){
#if defined(_WIN32) || defined(_WIN64)
        strcat(subdirPath, "\\");
#else
        strcat(subdirPath, "/");
#endif
    }

    strcat(subdirPath, subdir);
    
    std::cout << subdirPath << std::endl;
    
    return makePath(subdirPath);
}

int main(int carg, char* args[]){
    
    if(carg < 2){
        std::cout << "Usage: Lutherie [options [args]]" << std::endl;
        std::cout << "Available options:" << std::endl;
        std::cout << "-o | --open   <path-to-project>   Opens a Lutherie project at path destination or creates one if directory doesn't contain one" << std::endl;
        return 0;
    }
    
    for(size_t i = 1; i < carg; i+=2){
        if(strcmp(args[i], "-o") == 0 || strcmp(args[i], "--open") == 0){
            
            char* path = args[i+1];
            char newPath[strlen(path)];

            if(strncmp(path, "~", 1) == 0){
                char* home = getenv("HOME");
                strcpy(newPath, home);
                strcat(newPath, &path[1]);
                
                path = newPath;
                
            }
            
            std::cout << path << std::endl;
            
            if(makePath(path)){
                
                if(addSubDirectory(path, "scripts") && addSubDirectory(path, "resources")){
                    Lutherie lutherie = Lutherie(path);
                }
            }
        
            return 0;
        }
    }
    
//    World& world = World::createWorld<MySystem>();
//    Entity entity = world.createEntity();
//    world.setComponent(entity, new MyComponent());
    
    
    return 0;
}
