#include "lutherie.hpp"
#include <chrono>
//using namespace ECS;

Lutherie::Lutherie(const char* dir, const char* sDir, const char* rDir, const char* lDir) : projectDir(dir), scriptsDir(sDir), resourcesDir(rDir), libDir(lDir), ecs(new ECSLua(luaL_newstate(), dir)) {

		//setenv("LD_LIBRARY_PATH", std::string(std::string(dir) + std::string("lib")).c_str(), 0);
	#if defined (__linux__)

		std::string layerPath = std::string(std::string(dir)+std::string("etc/explicit_layer.d"));
    #elif defined(LUTHERIE_MAC)

		std::string layerPath = std::string(std::string(dir)+std::string("etc/explicit_layer.d"));
		std::string icdPath =  std::string(std::string(dir)+std::string("etc/icd.d/MoltenVK_icd.json"));
		setenv("VK_ICD_FILENAMES", icdPath.c_str(), 1);

	#endif
		setenv("VK_LAYER_PATH", layerPath.c_str(), 1);

	char * libScripts = new char[strlen(libDir) + 12];
	strcpy(libScripts, libDir);
	strcat(libScripts, "lua/scripts/");

#if defined(LUTHERIE_MAC) || defined(__unix__)
    
    void (*f)(const char*) = &ECSLua::executeLua;
    fs::doOnFilesInDir(libScripts, f);    
    fs::doOnFilesInDir(scriptsDir, f);
    
#else
	if (std::filesystem::exists(libScripts)) {
		for (std::filesystem::directory_entry it : std::filesystem::recursive_directory_iterator(libScripts)) {
			std::cout << it.path() << std::endl;
			ecs->executeLua(it.path().string().c_str());
}
	}
	if (std::filesystem::exists(sDir)) {
		for (std::filesystem::directory_entry it : std::filesystem::recursive_directory_iterator(sDir)) {
			std::cout << it.path() << std::endl;
			ecs->executeLua(it.path().string().c_str());
		}
	}
#endif

	delete[] libScripts;

    gfx = new VulkGfx();
    mainLoop();
}

Lutherie::Lutherie() {}

Lutherie::~Lutherie(){

	delete ecs;
    
    delete gfx;
    
	
}

void Lutherie::mainLoop(){
    using namespace ECS;

    while(!gfx->windowShouldClose()){
        
        auto start = std::chrono::high_resolution_clock::now();
        World::updateActive(World::allWorlds);
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration_cast<std::chrono::duration<double>>(end-start).count();

    }
}


