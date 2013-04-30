#ifndef OPENBOOK_FS_CLUI_LISTKNOWNPEERS_H_
#define OPENBOOK_FS_CLUI_LISTKNOWNPEERS_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       clui {

class ListKnownPeers:
	public Options
{

	public:
        static const std::string COMMAND;
        static const std::string DESCRIPTION;
		ListKnownPeers(TCLAP::CmdLine& cmd);
		void go();

};


}
}
}

#endif
