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
	mFileLabelListWidget->setLayout(layout);
	
	foreach (QString const &fileName, mProject->includedFileNames()) {
		layout->addWidget(
				new FileListItem(fileName, fileName, mFileLabelListWidget)
				);
	}
	//layout->addStretch();

	setWidget(mFileLabelListWidget);

	connect(
		project, SIGNAL(fileAdded(QString const &)),
		this, SLOT(fileAdded(QString const &))
		);
}

FileListDock::~FileListDock()
{
	if (mFileLabelListWidget) {
		delete mFileLabelListWidget;
	}
	mFileLabelListWidget = 0;
}

void FileListDock::fileAdded(QString const &newFileName)
{
	mFileLabelListWidget->layout()->addWidget(
			new FileListItem(newFileName, newFileName, mFileLabelListWidget)
			);
}

void FileListDock::addFile(QString const &fileName)
{
	FileListItem* newListItem =
		new FileListItem(fileName, fileName, mFileLabelListWidget);
	mFileLabelListWidget->layout()->addWidget(newListItem);
	connect(
		newListItem, SIGNAL(renamed(QString const &, QString const &)),
		mProject, SLOT(fileRenamed(QString const &, QString const &))
		);
}
