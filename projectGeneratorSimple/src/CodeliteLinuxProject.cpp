/*
 * CodeliteLinuxProject.cpp
 *
 *  Created on: 28/12/2011
 *      Author: arturo
 */

#include "CodeliteLinuxProject.h"
#include "ofFileUtils.h"
#include "ofLog.h"
#include "Utils.h"

string CodeliteLinuxProject::LOG_NAME = "CodeliteLinuxProject";

void CodeliteLinuxProject::setup() {
    ;
}

bool CodeliteLinuxProject::createProjectFile(){

    ofDirectory dir(projectDir);
	if(!dir.exists()) dir.create(true);

    ofFile project(ofFilePath::join(ofFilePath::join(projectDir, "codelite"), projectName + ".cbp"));
    string src =  ofFilePath::join(ofFilePath::join(templatePath, "codelite"), "emptyExample.project");
    string dst = ofFilePath::join(project.path(), "codelite");
    bool ret;

    if(!project.exists()){
		ret = ofFile::copyFromTo(src,dst);
		if(!ret){
			ofLogError(LOG_NAME) << "error copying project template from " << src << " to " << dst;
			return false;
		}else{
			findandreplaceInTexfile(dst, "emptyExample", projectName);
		}
    }

    ofFile workspace(ofFilePath::join(projectDir, projectName + ".workspace"));
    if(!workspace.exists()){
		src = ofFilePath::join(templatePath,"emptyExample_" + target + ".workspace");
		dst = workspace.path();
		ret = ofFile::copyFromTo(src,dst);
		if(!ret){
			ofLogError(LOG_NAME) << "error copying workspace template from "<< src << " to " << dst;
			return false;
		}else{
			findandreplaceInTexfile(dst, "emptyExample", projectName);
		}
    }

    ofFile makefile(ofFilePath::join(projectDir,"Makefile"));
    if(!makefile.exists()){
		src = ofFilePath::join(templatePath,"Makefile");
		dst = makefile.path();
		ret = ofFile::copyFromTo(src,dst);
		if(!ret){
			ofLogError(LOG_NAME) << "error copying Makefile template from " << src << " to " << dst;
			return false;
		}
    }

    ofFile config(ofFilePath::join(projectDir,"config.make"));
    if(!config.exists()){
    	src = ofFilePath::join(templatePath,"config.make");
    	dst = config.path();
    	ret = ofFile::copyFromTo(src,dst);
    	if(!ret){
    		ofLogError(LOG_NAME) << "error copying config.make template from " << src << " to " << dst;
    		return false;
    	}
    }


    // handle the relative roots.
    string relRoot = getOFRelPath(ofFilePath::removeTrailingSlash(projectDir));
    if (relRoot != "../../../"){
        string relPath2 = relRoot;
        relPath2.erase(relPath2.end()-1);
        findandreplaceInTexfile(projectDir + "config.make", "../../..", relPath2);
        findandreplaceInTexfile(ofFilePath::join(projectDir , projectName + ".workspace"), "../../../", relRoot);
        findandreplaceInTexfile(ofFilePath::join(projectDir , projectName + ".cbp"), "../../../", relRoot);
    }

    return true;
}

bool CodeliteLinuxProject::loadProjectFile(){

    //project.open(ofFilePath::join(projectDir , projectName + ".cbp"));

    ofFile project(projectDir + projectName + ".cbp");
	if(!project.exists()){
		ofLogError(LOG_NAME) << "error loading" << project.path() << "doesn't exist";
		return false;
	}
	pugi::xml_parse_result result = doc.load(project);
	bLoaded =result.status==pugi::status_ok;
	return bLoaded;
}

bool CodeliteLinuxProject::saveProjectFile(){

    findandreplaceInTexfile(ofFilePath::join(projectDir , projectName + ".workspace"),"emptyExample",projectName);
    pugi::xpath_node_set title = doc.select_nodes("//Option[@title]");
    if(!title.empty()){
        if(!title[0].node().attribute("title").set_value(projectName.c_str())){
            ofLogError(LOG_NAME) << "can't set title";
        }
    }
    return doc.save_file((projectDir + projectName + ".cbp").c_str());
}

void CodeliteLinuxProject::addSrc(string srcName, string folder, SrcType type){
	pugi::xml_node node = appendValue(doc, "Unit", "filename", srcName);
	if(!node.empty()){
		node.child("Option").attribute("virtualFolder").set_value(folder.c_str());
	}
}

void CodeliteLinuxProject::addInclude(string includeName){
    ofLogNotice() << "adding include " << includeName;
    appendValue(doc, "Add", "directory", includeName);
}

void CodeliteLinuxProject::addLibrary(string libraryName, LibType libType){
    ofLogNotice() << "adding library " << libraryName;
    appendValue(doc, "Add", "library", libraryName, true);
    // overwriteMultiple for a lib if it's there (so libsorder.make will work)
    // this is because we might need to say libosc, then ws2_32
}

string CodeliteLinuxProject::getName(){
	return projectName;
}

string CodeliteLinuxProject::getPath(){
	return projectDir;
}
