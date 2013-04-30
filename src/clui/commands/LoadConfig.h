#ifndef OPENBOOK_FS_CLUI_LOADCONFIG_H_
#define OPENBOOK_FS_CLUI_LOADCONFIG_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       clui {

class LoadConfig:
	public Options
{
	TCLAP::UnlabeledValueArg<std::string> fileName;
	
	public:

	static const std::string COMMAND;
        static const std::string DESCRIPTION;

	LoadConfig(TCLAP::CmdLine& cmd);
	void go();
};

}
}
}

#endif
