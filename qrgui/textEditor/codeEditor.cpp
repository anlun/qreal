#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStringListModel>
#include <QCursor>
#include <QMessageBox>
#include <QFileDialog>
#include "codeEditor.h"

using namespace qReal;
using namespace gui;

CodeEditor::CodeEditor(QWidget *parent)
	: QMainWindow(parent)
	, mNewAct(0)
	, mOpenAct(0)
	, mSaveAct(0)
	, mSaveAsAct(0)
	, mFileMenu(0)
	, mCodeArea(this)
	, mCompleter(0)
{
	setCentralWidget(&mCodeArea);
	
	initCompleter();
	createActions();
	createMenus();
}

CodeEditor::CodeEditor(QString const &fileName, QWidget *parent)
	: QMainWindow(parent)
	, mNewAct(0)
	, mOpenAct(0)
	, mSaveAct(0)
	, mSaveAsAct(0)
	, mFileMenu(0)
	, mCodeArea(this)
	, mCompleter(0)
{
	setCentralWidget(&mCodeArea);
	loadFile(fileName);
	
	initCompleter();
	createActions();
	createMenus();
}

CodeEditor::~CodeEditor()
{
	if (mCompleter) {
		mCodeArea.setCompleter(0);
		delete mCompleter;
	}
	mCompleter = 0;

	if (mNewAct) {
		delete mNewAct;
	}
	mNewAct = 0;

	if (mOpenAct) {
		delete mOpenAct;
	}
	mOpenAct = 0;

	if (mSaveAct) {
		delete mSaveAct;
	}
	mSaveAct = 0;

	if (mFileMenu) {
		delete mFileMenu;
	}
	mFileMenu = 0;
}

void CodeEditor::initCompleter()
{
	mCompleter = new QCompleter(this);

	mCompleter->setModel(modelFromFile("wordlist.txt")); // TODO
	mCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	
	mCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	//mCompleter->setCaseSensitivity(Qt::CaseSensitive);
	mCompleter->setWrapAround(false);
	mCodeArea.setCompleter(mCompleter);
}

void CodeEditor::createActions()
{	
	mNewAct = new QAction(tr("New"), this);
	mNewAct->setStatusTip(tr("Create a new file"));
	connect(mNewAct, SIGNAL(triggered()), this, SLOT(newFile()));

	mOpenAct = new QAction(tr("Open"), this);
	mOpenAct->setStatusTip(tr("Open an existing file"));
	connect(mOpenAct, SIGNAL(triggered()), this, SLOT(open()));

	mSaveAct = new QAction(tr("Save"), this);
	mSaveAct->setStatusTip(tr("Save the document to disk"));
	connect(mSaveAct, SIGNAL(triggered()), this, SLOT(save()));

	mSaveAsAct = new QAction(tr("Save As"), this);
	mSaveAsAct->setStatusTip(tr("Save the document under a new name"));
	connect(mSaveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

}

void CodeEditor::createMenus()
{
	mFileMenu = menuBar()->addMenu(tr("File"));
	mFileMenu->addAction(mNewAct);
	mFileMenu->addAction(mOpenAct);
	mFileMenu->addAction(mSaveAct);
	mFileMenu->addAction(mSaveAsAct);
}

void CodeEditor::setHighlightedLineNumbers(const QList<int>& lineNumbers)
{
	mCodeArea.setHighlightedLineNumbers(lineNumbers);
}

//QAbstractItemModel* CodeEditor::modelFromFile(QString const &fileName)
QStringListModel* CodeEditor::modelFromFile(QString const &fileName)
{
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly))
		return new QStringListModel(mCompleter);

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QStringList words;

	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		if (!line.isEmpty())
			words << line.trimmed();
	}

	QApplication::restoreOverrideCursor();
	return new QStringListModel(words, mCompleter);
}

void CodeEditor::open()
{
	if (maybeSave()) {
		QString const fileName = QFileDialog::getOpenFileName(this);
		if (!fileName.isEmpty())
			loadFile(fileName);
	}
}

bool CodeEditor::save()
{
	if (mCurFileName.isEmpty()) {
		return saveAs();
	} else {
		return saveFile(mCurFileName);
	}

	return true;
}

bool CodeEditor::saveAs() {
	QString const fileName = QFileDialog::getSaveFileName(this);
	if (fileName.isEmpty())
		return false;

	return saveFile(fileName);
}

bool CodeEditor::maybeSave()
{
	if (mCodeArea.document()->isModified()) {
		QMessageBox::StandardButton const ret =
			QMessageBox::warning(this, tr("Code editor"),
				tr("The document has been modified.\n"\
				"Do you want to save your changes?"),
				QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		if (ret == QMessageBox::Save) {
			return save();
		} else if (ret == QMessageBox::Cancel) {
			return false;
		}
	}
	return true;
}

void CodeEditor::documentWasModified()
{
	setWindowModified(mCodeArea.document()->isModified());
}

void CodeEditor::loadFile(QString const &fileName)
{
	QFile file(fileName);
	if (file.isOpen() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::warning(this, tr("Code editor"),
				tr("Cannot read file %1:\n%2")
				.arg(fileName)
				.arg(file.errorString()));
		return;
	}

	mCodeArea.clearHighlightedBlocksList();

	QTextStream in(&file);
	int curLineNumber = 0;
	QList<int> blockToHighlightNumbers;
	while (!in.atEnd()) {
		QString curLine = in.readLine();
		if (curLine.startsWith("#!")) {
			blockToHighlightNumbers.append(curLineNumber);
			curLine = curLine.right(curLine.size() - 2); //chop first 2 symbols "#!"
		}
		
		QRegExp controlInsertionRegExp("\@\@\\w+\@\@");
		int index = controlInsertionRegExp.indexIn(curLine);
		while (index >= 0) {
			int length = controlInsertionRegExp.matchedLength();
			QString toReplaceStr = controlInsertionRegExp.cap();
			toReplaceStr = toReplaceStr.left(toReplaceStr.length() - 2) + "</b>"; // changing last @@ to </b>
			toReplaceStr = "<b>" + toReplaceStr.right(toReplaceStr.length() -2); // changing first @@ to <b>

			curLine.replace(index, length, toReplaceStr);

			index = controlInsertionRegExp.indexIn(curLine, index + length);
		}

		//mCodeArea.appendPlainText(curLine);
		mCodeArea.appendHtml(curLine);
		
		curLineNumber++;
	}
	mCodeArea.addBlockToHighlightNumbers(blockToHighlightNumbers);

	mCodeArea.alignControlLines();

	/*
	//QApplication::setOverrideCursor(Qt::WaitCursor);
	mCodeArea.setPlainText(in.readAll());
	//mCodeArea.setHtml(in.readAll());
	//QApplication::restoreOverrideCursor();
	*/
	
	setCurrentFile(fileName);
}

void CodeEditor::setCurrentFile(QString const &fileName)
{
	mCurFileName = fileName;
	mCodeArea.document()->setModified(false);
	setWindowModified(false);
}

bool CodeEditor::saveFile(QString const &fileName)
{
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this, tr("Code Editor"),
				tr("Cannot write file %1: \n%2.")
				.arg(fileName)
				.arg(file.errorString()));
		return false;
	}

	QTextStream out(&file);
	//QApplication::setOverrideCursor(Qt::WaitCursor);
	out << mCodeArea.toPlainText();
	//QApplication::restoreOverrideCursor();
	
	setCurrentFile(fileName);
	return true;
}

void CodeEditor::newFile()
{
	if (maybeSave()) {
		QString fileName = QFileDialog::getOpenFileName(this);
		if (!fileName.isEmpty())
			loadFile(fileName);
	}
}
