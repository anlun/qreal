#pragma once

#include <QMainWindow>
#include <QCompleter>
#include <QAbstractItemModel>
#include <QStringListModel>
#include <QCloseEvent>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QTabWidget>
#include <QListWidget>
#include <QScrollArea>

#include "codeArea.h"
#include "projectInfo.h"
#include "fileListDock.h"

namespace qReal {
namespace genyCodeEditor {

class CodeEditor : public QMainWindow {
	Q_OBJECT

public:
	CodeEditor(QWidget *parent = 0);

	CodeEditor(
			QString const &gemakeFileName //< gemakeFileName - path to Geny make file
			, QWidget *parent = 0
		);
	~CodeEditor();

	void setHighlightedLineNumbers(QList<int> const &lineNumbers);

//protected:
//	void closeEvent(QCloseEvent* event);

private slots:
	void newFile();
	void open();
	void openProject();
	bool save();
	bool saveAs();
	bool saveToModel();
	void documentWasModified();

	void toggleHighlightedLineType();
	void toggleControlLineVisible();

	void currentTabChanged(int index);
	void tabCloseRequested(int index);

private:
	void initCompleter();
	void initActions();
	void createMenus();
	void createProjectFileDock();

	bool maybeSave();
	
	void loadFile(QString const &fileName);
	bool saveFile(QString const &fileName);

	void setCurrentFile(QString const &fileName);

	/// Returns current active in tabs CodeArea
	CodeArea* currentCodeArea();

	//QAbstractItemModel* modelFromFile(QString const &fileName);
	QStringListModel* modelFromFile(QString const &fileName);

	QAction mNewAct;
	QAction mOpenAct;
	QAction mOpenProjectAct;
	QAction mSaveAct;
	QAction mSaveAsAct;
	QAction mSaveToModelAct;

	QAction mToggleHighlightedLineTypeAct;
	QAction mToggleControlLineVisibleAct;

	QMenu* mFileMenu;
	QMenu* mViewMenu;

	QTabWidget mCodeAreaTab;
	//QScrollArea* mProjectFileListWidget;
	FileListDock* mFileListDock;

	QCompleter* mCompleter;

	ProjectInfo mProject;
};

}
}
