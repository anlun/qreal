#pragma once

#include <QMainWindow>
#include <QCompleter>
#include <QAbstractItemModel>
#include <QStringListModel>
#include <QCloseEvent>
#include <QAction>
#include "codeArea.h"

namespace qReal {
namespace gui {

class CodeEditor : public QMainWindow {
	Q_OBJECT

	public:
		CodeEditor(QWidget *parent = 0);
		CodeEditor(const QString& filename, QWidget *parent = 0);
		~CodeEditor();

		void setHighlightedLineNumbers(const QList<int>& lineNumbers);

	//protected:
	//	void closeEvent(QCloseEvent* event);

	private slots:
		void open();
		bool save();

	private:
		void initCompleter();
		void createActions();

		bool maybeSave();
		
		//void loadFile(QString const &fileName);
		//void saveFile(QString const &fileName);

		//QAbstractItemModel* modelFromFile(QString const &fileName);
		QStringListModel* modelFromFile(QString const &fileName);

		QAction* mOpenAct;
		QAction* mSaveAct;

		CodeArea mCodeArea;
		QCompleter* mCompleter;
};

}
}
