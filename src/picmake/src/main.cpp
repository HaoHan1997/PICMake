#include "Svar.h"
#include "filesystem.hpp"
#include "VecParament.h"

using namespace GSLAM;

std::string getName(){
    return GSLAM::filesystem::current_path().stem();
}

int main(int argc,char** argv)
{
    svar.parseMain(argc,argv);

    std::string mode=svar.arg<std::string>("m","bin","The target compile mode: bin,shared,static");
    std::string name=svar.arg<std::string>("name",getName(),"The target name, default is the parent folder name");
    std::string requires=svar.arg<std::string>("require","","The required dependencies");
    std::string srcs=svar.arg<std::string>("src","src","The source cpp folders for files, eg: main.cpp,src");
    std::string standard=svar.arg<std::string>("standard","11","The c++ standard wanna use");
    std::string build=svar.arg<std::string>("build","","The folder to hold the build targets");
    std::string build_type=svar.arg<std::string>("build_type","Release","CMAKE_BUILD_TYPE: Default, Release, Debug");
    bool        install=svar.arg("install",true,"Auto generate install& uninstall");

    if(svar.get("help",false)) return svar.help();

    if(filesystem::exists("CMakeLists.txt")) {
        std::cout<<"CMakeLists.txt already exists, abort generating."<<std::endl;
        return 0;
    }

    std::stringstream sst;
    sst<<"cmake_minimum_required(VERSION 2.8)"<<std::endl;
    sst<<"project("<<name<<")"<<std::endl;
    sst<<"\ninclude(PICMake)"<<std::endl;

    sst<<"set(CMAKE_CXX_STANDARD "<<standard<<")"<<std::endl;

    sst<<"if(NOT CMAKE_BUILD_TYPE)"<<std::endl;
    sst<<"  set(CMAKE_BUILD_TYPE "<<build_type<<")"<<std::endl;
    sst<<"endif()\n"<<std::endl;

    sv::Svar requires_vec=VecParament<std::string>(requires).data;
    sv::Svar srcs_vec= VecParament<std::string>(srcs).data;

    for(auto& c:mode) c=toupper(c);

    sst<<"\npi_add_target("<<name<<" "<<mode;

    bool abortbuild=false;
    for(sv::Svar s:srcs_vec){
        if(!GSLAM::filesystem::exists(s.as<std::string>()))
        {
            std::cout<<"Can't find file or directory "<<s.as<std::string>()<<std::endl;
            abortbuild=true;
        }
        sst<<" "<<s.as<std::string>();
    }

    sst<<" REQUIRED System";
    for(sv::Svar s:requires_vec)
        sst<<" "<<s.as<std::string>();

    sst<<")\n";

    sst<<"\npi_report_target()";

    if(install){

        sst<<"\n\n# Now do make install";

        sst<<"\nget_property(TARGETS2COMPILE GLOBAL PROPERTY TARGETS2COMPILE)";
        sst<<"\npi_install(TARGETS ${TARGETS2COMPILE})";
    }


    std::cout<<sst.str()<<std::endl;
    std::ofstream ofs("CMakeLists.txt");
    ofs<<sst.str();

    if(build.size()&&!abortbuild){
       GSLAM::filesystem::create_directories(build);
       std::system(("cd "+build+" ; cmake .. ; make").c_str());
    }

    return 0;
}
