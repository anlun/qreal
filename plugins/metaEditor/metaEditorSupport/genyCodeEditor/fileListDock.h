#pragma once

#include <QScrollArea>
#include <QDockWidget>
#include "projectInfo.h"

namespace qReal {
namespace genyCodeEditor {

class FileListDock : public QDockWidget
{
	Q_OBJECT

public:
	FileListDock(ProjectInfo* project, QWidget* parent = 0);
	~FileListDock();
private:
	QScrollArea* mFileLabelListWidget;
	ProjectInfo* mProject;
};

}
}
