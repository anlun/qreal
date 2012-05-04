#pragma once

#include <QLabel>
#include <QMenu>
#include <QAction>

namespace qReal {
namespace genyCodeEditor {

class FileListItem : public QLabel
{
	Q_OBJECT
public:
	FileListItem(
			QString const &text
			, QString const &pathToFile
			, QWidget* parent = 0
		);
	~FileListItem();

	QString pathToFile() const;
	void setPathToFile(QString const &pathToFile);

signals:
	void renamed(QString const &oldName, QString const &newName);

private slots:
	void showContextMenu(QPoint const &pos);
	void renameFile();

private:
	QString mPathToFile;

	QMenu mPopupMenu;
	QAction mRenameAct;
	QAction mDeleteAct; 

};

}
}
