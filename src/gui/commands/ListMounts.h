#ifndef OPENBOOK_FS_GUI_LISTMOUNTS_H_
#define OPENBOOK_FS_GUI_LISTMOUNTS_H_

#include "Options.h"
#include <QStringList>

namespace   openbook {
namespace filesystem {
namespace       gui {

class ListMounts:
	public Options
{
	public:
        static const std::string COMMAND;
        static const std::string DESCRIPTION;
        ListMounts(QString port = "3030");
        QStringList go();
};


}
}
}

#endif
