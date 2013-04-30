#ifndef OPENBOOK_FS_CLUI_SETLOCALSOCKET_H_
#define OPENBOOK_FS_CLUI_SETLOCALSOCKET_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       clui {

class SetLocalSocket:
	public Options
{
	TCLAP::UnlabeledValueArg<int> port;
	
	public:
	    static const std::string COMMAND;
	    static const std::string DESCRIPTION;
	
        SetLocalSocket(TCLAP::CmdLine& cmd);
        void go();
};

}
}
}

#endif
