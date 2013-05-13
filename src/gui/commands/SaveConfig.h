#ifndef OPENBOOK_FS_GUI_SAVECONFIG_H_
#define OPENBOOK_FS_GUI_SAVECONFIG_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       gui {

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
