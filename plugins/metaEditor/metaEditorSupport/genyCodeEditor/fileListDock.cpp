#include <QVBoxLayout>
#include "fileListDock.h"
#include "fileListItem.h"

using namespace qReal;
using namespace genyCodeEditor;

FileListDock::FileListDock(ProjectInfo* project, QWidget* parent)
	: QDockWidget(tr("Project files"), parent)
	, mProject(project)
{	
	setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	mFileLabelListWidget = new QScrollArea(this);
	
	// set white background
	QPalette pal(mFileLabelListWidget->palette());
	pal.setColor(QPalette::Background, Qt::white);
	mFileLabelListWidget->setAutoFillBackground(true);
	mFileLabelListWidget->setPalette(pal);

	QVBoxLayout* layout = new QVBoxLayout(mFileLabelListWidget);
	foreach (QString const &fileName, mProject->includedFileNames()) {
		layout->addWidget(
				new FileListItem(fileName, fileName, mFileLabelListWidget)
				);
	}
	layout->addStretch();
	mFileLabelListWidget->setLayout(layout);

	setWidget(mFileLabelListWidget);
}

FileListDock::~FileListDock() {
	if (mFileLabelListWidget) {
		delete mFileLabelListWidget;
	}
	mFileLabelListWidget = 0;
}
