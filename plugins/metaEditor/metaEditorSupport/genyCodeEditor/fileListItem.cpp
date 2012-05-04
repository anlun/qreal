#include <QInputDialog>

#include "fileListItem.h"

using namespace qReal;
using namespace genyCodeEditor;

FileListItem::FileListItem(
			QString const &text
			, QString const &pathToFile
			, QWidget* parent
		) : QLabel(text, parent)
		, mPathToFile(pathToFile)
		, mRenameAct(this)
		, mDeleteAct(this)
{	
	mRenameAct.setText(tr("Rename a file"));
	mRenameAct.setStatusTip(tr("Rename a file in project"));
	connect(&mRenameAct, SIGNAL(triggered()), this, SLOT(renameFile()));
	mPopupMenu.addAction(&mRenameAct);

	mDeleteAct.setText(tr("Delete a file"));
	mDeleteAct.setStatusTip(tr("Delete a file from project"));
	//connect(&mRenameAct, SIGNAL(triggered()), this, SLOT(newFile()));
	mPopupMenu.addAction(&mDeleteAct);

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(
		this, SIGNAL(customContextMenuRequested(QPoint const &)),
		this, SLOT(showContextMenu(QPoint const &))
		);
}

FileListItem::~FileListItem()
{
}

void FileListItem::showContextMenu(QPoint const &pos)
{
	mPopupMenu.exec(mapToGlobal(pos));
}

void FileListItem::renameFile()
{
	bool ok;
	QString const newFileName = QInputDialog::getText(
			this, tr("Enter new file name")
			, tr("File name:")
			, QLineEdit::Normal, text(), &ok
			);
	if (!ok || newFileName.isEmpty())
		return;
	//TODO: Implement case of repeated file name
	
	setText(newFileName);
	emit renamed(mPathToFile, newFileName);

	mPathToFile = newFileName; //TODO
}
