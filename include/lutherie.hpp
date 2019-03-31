#ifndef lutherie_hpp
#define lutherie_hpp

#include <luajit-2.0/lua.hpp>

//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>

//#include <glm/vec3.hpp>
//#include <glm/vec4.hpp>
//#include <glm/mat4x4.hpp>

#include <ECSlua.hpp>
#include "fs.h"
#include <stdlib.h>
#include <gfx.hpp>

#if defined(_WIN32) || defined(_WIN64)
    #include <filesystem>
#endif

#include<string>

class Lutherie {
public:
    
    static bool importAll3D(){
        return false;
    }
    
    static bool import3D(const std::string& pFile){
//        Assimp::Importer importer;
//
//        const aiScene scene = importer.ReadFile(pFile);
//
//        if(!scene){
//            throw std::runtime_error(importer.GetErrorString());
//            return false;
//        }

        return true;
    }
        
    
    Lutherie(const char* dir, const char* sDir, const char* rDir, const char* lDir);
    ~Lutherie();
    
private:
    
    const char* projectDir;
    const char* scriptsDir;
    const char* resourcesDir;
    const char* libDir;
    
    Gfx* gfx;
    
    ECSLua* ecs;
    
    bool frameBufferResized = false;
    
    void initWindow();

    void mainLoop();
    
    Lutherie();
};

#endif /* lutherie_hpp */
