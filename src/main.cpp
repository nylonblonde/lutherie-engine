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
        std::cout << group.size() << std::endl;
    }
    
};

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
            
            if(fs::makePath(path)){
                
                char scriptsDir[] = "scripts/";
                char resDir[] = "resources/";
                char libDir[] = "lib/";
                
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
                    
                    //TODO: copy default libraries to project's library directory. Change addSubdirectory to return int so we can detect if directory was created or existed
                    
                    Lutherie lutherie = Lutherie(path, scriptsPath, resPath, libPath);
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
