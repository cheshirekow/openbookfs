#ifndef OPENBOOK_FS_GUI_SETDATADIR_H_
#define OPENBOOK_FS_GUI_SETDATADIR_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       gui {

class SetDataDir:
	public Options
{

	
	public:
	static const std::string COMMAND;
	static const std::string DESCRIPTION;

    SetDataDir(QString port);
    void go(QString data_dir);
};


}
}
}

#endif
