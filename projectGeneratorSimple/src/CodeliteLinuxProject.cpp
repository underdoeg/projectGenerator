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
    templatePath = ofToDataPath("template-codelite-linux");
}

bool CodeliteLinuxProject::createProjectFile(){

    ofDirectory dir(projectDir);
	if(!dir.exists()) dir.create(true);
    
    ofDirectory codeliteDir(ofFilePath::join(projectDir, "codelite"));
	if(!codeliteDir.exists()) codeliteDir.create(true);
    
    string templatePathCodelite = ofFilePath::join(templatePath, "codelite");
    
    ofFile project(ofFilePath::join(codeliteDir.path(), projectName + ".project"));
    string src =  ofFilePath::join(templatePathCodelite, "testApp.project");
    string dst = project.path();
    bool ret;

    if(!project.exists()){
		ret = ofFile::copyFromTo(src,dst);
		if(!ret){
			ofLogError(LOG_NAME) << "error copying project template from " << src << " to " << dst;
			return false;
		}else{
			findandreplaceInTexfile(dst, "testApp", projectName);
		}
    }
    
    ofFile ofLibProject(ofFilePath::join(codeliteDir.path(), "libOpenFrameworks.project"));
    if(!ofLibProject.exists()){
		src = ofFilePath::join(templatePathCodelite,"libOpenFrameworks.project");
		dst = ofLibProject.path();
		ret = ofFile::copyFromTo(src,dst);
		if(!ret){
			ofLogError(LOG_NAME) << "error copying of project template from "<< src << " to " << dst;
			return false;
		}else{
			findandreplaceInTexfile(dst, "testApp", projectName);
		}
    }
    
    
    ofFile workspace(ofFilePath::join(codeliteDir.path(), projectName + ".workspace"));
    if(!workspace.exists()){
		src = ofFilePath::join(templatePathCodelite,"testApp.workspace");
		dst = workspace.path();
		ret = ofFile::copyFromTo(src,dst);
		if(!ret){
			ofLogError(LOG_NAME) << "error copying workspace template from "<< src << " to " << dst;
			return false;
		}else{
			findandreplaceInTexfile(dst, "testApp", projectName);
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
    if (relRoot != ".../../../../../../"){
        string relPath2 = relRoot;
        relPath2.erase(relPath2.end()-1);
        findandreplaceInTexfile(projectDir + "config.make", "../../../../../..", relPath2);
        findandreplaceInTexfile(ofFilePath::join(projectDir , "codelite/"+projectName + ".workspace"), "../../../../../../..", relPath2+"/..");
        findandreplaceInTexfile(ofFilePath::join(projectDir ,"codelite/"+ projectName + ".project"), "../../../../../../..", relPath2+"/..");
        findandreplaceInTexfile(ofFilePath::join(projectDir , "codelite/libOpenFrameworks.project"), "../../../../../../..", relPath2+"/..");
    }

    return true;
}

bool CodeliteLinuxProject::loadProjectFile(){

    //project.open(ofFilePath::join(projectDir , projectName + ".cbp"));

    ofFile project(ofFilePath::join(ofFilePath::join(projectDir, "codelite") , projectName + ".project"));
	if(!project.exists()){
		ofLogError(LOG_NAME) << "error loading" << project.path() << "doesn't exist";
		return false;
	}
	pugi::xml_parse_result result = doc.load(project);
	bLoaded =result.status==pugi::status_ok;
	return bLoaded;
}

bool CodeliteLinuxProject::saveProjectFile(){
    
    findandreplaceInTexfile(ofFilePath::join(ofFilePath::join(projectDir, "codelite") , projectName + ".workspace"), "testApp", projectName);
    /*
    findandreplaceInTexfile(ofFilePath::join(ofFilePath::join(projectDir, "codelite"), projectName + ".project"), "testApp", projectName);
    pugi::xpath_node_set title = doc.select_nodes("//CodeLite_Workspace[@Name]");
    if(!title.empty()){
        if(!title[0].node().attribute("title").set_value(projectName.c_str())){
            ofLogError(LOG_NAME) << "can't set title";
        }
    }
    */
    
    string projectPath = ofFilePath::join(ofFilePath::join(projectDir, "codelite") , projectName + ".project");
    return doc.save_file(projectPath.c_str());
}

void CodeliteLinuxProject::addSrc(string srcName, string folder, SrcType type){
    //std::string path = "//CodeLite_Project/VirtualDirectory[@Name='"+folder+"']";
    
    vector<string> paths = ofSplitString(folder, "/", true, true);
    
    pugi::xml_node pugiFolder = doc.child("CodeLite_Project");
    
    for(auto& path: paths){
        pugi::xpath_node_set node_set = pugiFolder.select_nodes(("VirtualDirectory[@Name='"+path+"']").c_str());
        if(!node_set.empty()){
            pugiFolder = node_set[0].node();
        }else{
            pugiFolder = pugiFolder.append_child("VirtualDirectory");
            pugiFolder.append_attribute("Name").set_value(path.c_str());
        }
    }
    
    
    
    string sourceRel = "../"+srcName;
    
    if(!pugiFolder.find_child_by_attribute("Name", sourceRel.c_str()))
        pugiFolder.append_child("File").append_attribute("Name").set_value((sourceRel).c_str());
    //fileNode.attribute("Name").set_value(srcName.c_str());
    
    /*
    std::cout << "Document:\n";
    doc.save(std::cout);
    cout << endl;
	/*
	pugi::xml_node node = appendValue(doc, "VirtualDirectory", "Name", srcName);
	if(!node.empty()){
		node.child("Option").attribute("virtualFolder").set_value(folder.c_str());
	}
	*/
}

void CodeliteLinuxProject::addInclude(string includeName){
    /*
    ofLogNotice() << "adding include " << includeName;
    appendValue(doc, "Add", "directory", includeName);
    */
}

void CodeliteLinuxProject::addLibrary(string libraryName, LibType libType){
    /*
    ofLogNotice() << "adding library " << libraryName;
    appendValue(doc, "Add", "library", libraryName, true);
    */
    // overwriteMultiple for a lib if it's there (so libsorder.make will work)
    // this is because we might need to say libosc, then ws2_32
}

string CodeliteLinuxProject::getName(){
	return projectName;
}

string CodeliteLinuxProject::getPath(){
	return projectDir;
}
