#ifndef OPENBOOK_FS_GUI_LISTKNOWNPEERS_H_
#define OPENBOOK_FS_GUI_LISTKNOWNPEERS_H_

#include "Options.h"
#include <QStringList>
namespace   openbook {
namespace filesystem {
namespace       gui {

class ListKnownPeers:
	public Options
{

	public:
        static const std::string COMMAND;
        static const std::string DESCRIPTION;
        ListKnownPeers();
        QStringList go();

};


}
}
}

#endif
