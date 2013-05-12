#ifndef OPENBOOK_FS_GUI_LISTMOUNTS_H_
#define OPENBOOK_FS_GUI_LISTMOUNTS_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       gui {

class ListMounts:
	public Options
{
	public:
        static const std::string COMMAND;
        static const std::string DESCRIPTION;
        ListMounts();
		void go();
};


}
}
}

#endif
