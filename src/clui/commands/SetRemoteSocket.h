#ifndef OPENBOOK_FS_CLUI_REMOTESOCKETOPTIONS_H_
#define OPENBOOK_FS_CLUI_REMOTESOCKETOPTIONS_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       clui {

class SetRemoteSocket:
	public Options
{
	TCLAP::UnlabeledValueArg<int> addressFamily;
	TCLAP::UnlabeledValueArg<std::string> node;
	TCLAP::UnlabeledValueArg<std::string> remoteService;

	public:
    static const std::string COMMAND;
    static const std::string DESCRIPTION;
	
    SetRemoteSocket(TCLAP::CmdLine& cmd);
	void go();
};

}
}
}

#endif
