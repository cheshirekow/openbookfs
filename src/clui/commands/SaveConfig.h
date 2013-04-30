#ifndef OPENBOOK_FS_CLUI_SAVECONFIG_H_
#define OPENBOOK_FS_CLUI_SAVECONFIG_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       clui {

class SaveConfig:
public Options{

	TCLAP::UnlabeledValueArg<std::string> fileName;
	
	public:
	    static const std::string COMMAND;
	    static const std::string DESCRIPTION;

        SaveConfig(TCLAP::CmdLine& cmd);
        void go();

};

}
}
}

#endif
