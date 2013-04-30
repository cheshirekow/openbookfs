#ifndef OPENBOOK_FS_CLUI_LISTMOUNTS_H_
#define OPENBOOK_FS_CLUI_LISTMOUNTS_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       clui {

class ListMounts:
	public Options
{
	public:
        static const std::string COMMAND;
		ListMounts(TCLAP::CmdLine& cmd);
		void go();
};


}
}
}

#endif
