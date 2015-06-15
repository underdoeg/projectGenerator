
#ifndef VSWINPROJECT_H_
#define VSWINPROJECT_H_

#include "baseProject.h"
#include "ofConstants.h"
#include "ofAddon.h"

class visualStudioProject : public baseProject {

public:

    void setup(string ofRoot= "../../../");

    void setup();

    bool createProjectFile();
    bool loadProjectFile();
    bool saveProjectFile();

    void addSrc(string srcFile, string folder, SrcType type=DEFAULT);
    void addInclude(string includeName);
    void addLibrary(string libraryName, LibType libType);
    void addCFLAG(string cflag, LibType libType = RELEASE_LIB); // C
    void addCPPFLAG(string cppflag, LibType libType = RELEASE_LIB); // C++

    void addAddon(ofAddon & addon);

	static string LOG_NAME;

	pugi::xml_document filterXmlDoc;


	void appendFilter(string folderName);

private:

};

#endif
