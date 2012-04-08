#pragma once

#include <QMainWindow>
#include <QCompleter>
#include <QAbstractItemModel>
#include <QStringListModel>
#include <QCloseEvent>
#include <QAction>
#include <QMenu>
#include <QMenuBar>

#include "codeArea.h"

namespace qReal {
namespace genyCodeEditor {

class CodeEditor : public QMainWindow {
	Q_OBJECT

	public:
		CodeEditor(QWidget *parent = 0);
		CodeEditor(QString const &fileName, QWidget *parent = 0);
		~CodeEditor();

		void setHighlightedLineNumbers(const QList<int>& lineNumbers);

	//protected:
	//	void closeEvent(QCloseEvent* event);

	private slots:
		void newFile();
		void open();
		bool save();
		bool saveAs();
		void documentWasModified();

		void toggleHighlightedLineType();
		void toggleControlLineVisible();

	private:
		void initCompleter();
		void createActions();
		void createMenus();

		bool maybeSave();
		
		void loadFile(QString const &fileName);
		bool saveFile(QString const &fileName);

		void setCurrentFile(QString const &fileName);

		//QAbstractItemModel* modelFromFile(QString const &fileName);
		QStringListModel* modelFromFile(QString const &fileName);

		QAction* mNewAct;
		QAction* mOpenAct;
		QAction* mSaveAct;
		QAction* mSaveAsAct;

		QAction* mToggleHighlightedLineTypeAct;
		QAction* mToggleControlLineVisibleAct;

		QMenu* mFileMenu;
		QMenu* mViewMenu;

		CodeArea mCodeArea;
		QCompleter* mCompleter;

		QString mCurFileName;
};

}
}
