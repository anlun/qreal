#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStringListModel>
#include <QCursor>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>

#include "codeEditor.h"

using namespace qReal;
using namespace genyCodeEditor;

CodeEditor::CodeEditor(QWidget *parent)
	: QMainWindow(parent)
	, mNewAct(this)
	, mOpenAct(this)
	, mOpenProjectAct(this)
	, mSaveAct(this)
	, mSaveAsAct(this)
	, mSaveToModelAct(this)
	, mToggleHighlightedLineTypeAct(this)
	, mToggleControlLineVisibleAct(this)
	, mFileMenu(0)
	, mViewMenu(0)
	, mCompleter(0)
{
	mCodeAreaTab.addTab(new CodeArea, tr("unknown"));
	mCodeAreaTab.setTabsClosable(true);

	setCentralWidget(&mCodeAreaTab);
	
	initCompleter();
	initActions();
	createMenus();

	connect(&mCodeAreaTab, SIGNAL( currentChanged(int) )
			, this, SLOT( currentTabChanged(int) ));
	connect(&mCodeAreaTab, SIGNAL( tabCloseRequested(int) )
			, this, SLOT( tabCloseRequested(int) ));
}

CodeEditor::CodeEditor(QString const &gemakeFileName, QWidget *parent)
	: QMainWindow(parent)
	, mNewAct(this)
	, mOpenAct(this)
	, mOpenProjectAct(this)
	, mSaveAct(this)
	, mSaveAsAct(this)
	, mSaveToModelAct(this)
	, mToggleHighlightedLineTypeAct(this)
	, mToggleControlLineVisibleAct(this)
	, mFileMenu(0)
	, mViewMenu(0)
	, mCompleter(0)
{
	//mCodeAreaTab.addTab(new CodeArea, QFileInfo(gemakeFileName).fileName());
	mCodeAreaTab.setTabsClosable(true);

	if (mProject.init(gemakeFileName)) {
		foreach (QString const &filePath, mProject.includedFileNames()) {
			int currentTab = 
				mCodeAreaTab.addTab(new CodeArea, QFileInfo(filePath).fileName());
			mCodeAreaTab.setCurrentIndex(currentTab);
			loadFile(filePath);
		}
	} else {
		//TODO
	}

	setCentralWidget(&mCodeAreaTab);
	
	initCompleter();
	initActions();
	createMenus();

	connect(&mCodeAreaTab, SIGNAL( currentChanged(int) )
			, this, SLOT( currentTabChanged(int) ));
	connect(&mCodeAreaTab, SIGNAL( tabCloseRequested(int) )
			, this, SLOT( tabCloseRequested(int) ));
}

CodeEditor::~CodeEditor()
{
	if (mCompleter) {
		delete mCompleter;
	}
	mCompleter = 0;

	if (mFileMenu) {
		delete mFileMenu;
	}
	mFileMenu = 0;

	if (mViewMenu) {
		delete mViewMenu;
	}
	mViewMenu = 0;

}

CodeArea* CodeEditor::currentCodeArea()
{
	return dynamic_cast<CodeArea *>(mCodeAreaTab.currentWidget());
}

void CodeEditor::initCompleter()
{
	mCompleter = new QCompleter(this);

	mCompleter->setModel(modelFromFile("wordlist.txt")); // TODO
	mCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	
	mCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	//mCompleter->setCaseSensitivity(Qt::CaseSensitive);
	mCompleter->setWrapAround(false);

	currentCodeArea()->setCompleter(mCompleter);
}

void CodeEditor::initActions()
{	
	mNewAct.setText(tr("New"));
	mNewAct.setStatusTip(tr("Create a new file"));
	connect(&mNewAct, SIGNAL(triggered()), this, SLOT(newFile()));

	mOpenAct.setText(tr("Open"));
	mOpenAct.setStatusTip(tr("Open an existing file"));
	connect(&mOpenAct, SIGNAL(triggered()), this, SLOT(open()));

	mOpenProjectAct.setText(tr("Open project"));
	mOpenProjectAct.setStatusTip(tr("Open an existing Geny project"));
	connect(&mOpenProjectAct, SIGNAL(triggered()), this, SLOT(openProject()));

	mSaveAct.setText(tr("Save"));
	mSaveAct.setStatusTip(tr("Save the document to disk"));
	connect(&mSaveAct, SIGNAL(triggered()), this, SLOT(save()));

	mSaveAsAct.setText(tr("Save As"));
	mSaveAsAct.setStatusTip(tr("Save the document under a new name"));
	connect(&mSaveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

	mSaveToModelAct.setText(tr("Save to model"));
	mSaveToModelAct.setStatusTip(tr("Save Geny project to model"));
	connect(&mSaveToModelAct, SIGNAL(triggered()), this, SLOT(saveToModel()));

	mToggleHighlightedLineTypeAct.setText(tr("Switch line highlighting"));
	mToggleHighlightedLineTypeAct.setStatusTip(tr("Toggle highlighting beetwen control and not lines"));
	connect(&mToggleHighlightedLineTypeAct, SIGNAL(triggered()), this, SLOT(toggleHighlightedLineType()));

	mToggleControlLineVisibleAct.setText(tr("Switch control line visibility"));
	mToggleControlLineVisibleAct.setStatusTip(tr("Show/unshow control lines"));
	connect(&mToggleControlLineVisibleAct, SIGNAL(triggered()), this, SLOT(toggleControlLineVisible()));
}

void CodeEditor::createMenus()
{
	mFileMenu = menuBar()->addMenu(tr("File"));
	mFileMenu->addAction(&mNewAct);
	mFileMenu->addAction(&mOpenAct);
	mFileMenu->addAction(&mOpenProjectAct);
	mFileMenu->addAction(&mSaveAct);
	mFileMenu->addAction(&mSaveAsAct);
	mFileMenu->addAction(&mSaveToModelAct);
	
	mViewMenu = menuBar()->addMenu(tr("View"));
	mViewMenu->addAction(&mToggleHighlightedLineTypeAct);
	mViewMenu->addAction(&mToggleControlLineVisibleAct);
}

void CodeEditor::setHighlightedLineNumbers(QList<int> const &lineNumbers)
{
	currentCodeArea()->setHighlightedLineNumbers(lineNumbers);
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

void CodeEditor::newFile()
{
	int newTabIndex = mCodeAreaTab.addTab(new CodeArea, tr("unknown"));
	mCodeAreaTab.setCurrentIndex(newTabIndex);
}

void CodeEditor::open()
{
	QString const fileName = QFileDialog::getOpenFileName(this);
	int newTabIndex = mCodeAreaTab.addTab(new CodeArea()
			, QFileInfo(fileName).fileName());
	mCodeAreaTab.setCurrentIndex(newTabIndex);
	
	loadFile(fileName);
}

void CodeEditor::openProject()
{
	//TODO
}

bool CodeEditor::save()
{
	if (currentCodeArea()->connectedFileName().isEmpty()) {
		return saveAs();
	} else {
		return saveFile(currentCodeArea()->connectedFileName());
	}

	return true;
}

bool CodeEditor::saveAs() {
	QString const fileName = QFileDialog::getSaveFileName(this);
	if (fileName.isEmpty())
		return false;

	bool result = saveFile(fileName);

	if (result) {
		mCodeAreaTab.setTabText(mCodeAreaTab.currentIndex()
				, QFileInfo(fileName).fileName());
	}

	return result;
}

bool CodeEditor::maybeSave()
{
	if (currentCodeArea()->document()->isModified()) {
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
	//setWindowModified(mCodeArea.document()->isModified());
	setWindowModified(currentCodeArea()->document()->isModified());
}

bool CodeEditor::saveToModel()
{
	QString projectFolder = "Geny_Project_Model";
	if (!QDir(projectFolder).exists()) {
		QDir().mkdir(projectFolder);
	}

	QFile gemakeFile(projectFolder + "/" + "gemakefile");
	if (!gemakeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "Can't create gemakefile!";
		return false;
	}

	QTextStream gemakeFileStream(&gemakeFile);
	gemakeFileStream << ".\n\n";
	foreach (QString const &fileName, mProject.includedFileNames()) {
		gemakeFileStream << fileName + "\n";
	}

	//TODO smth about special cases :-)
	foreach (QString const &includedFileName, mProject.includedFileNames()) {
		QFile(includedFileName).copy(projectFolder + "/"
				+ QFileInfo(includedFileName).fileName());
	}

	return true;
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

	//mCodeArea.clearHighlightedBlocksList();
	currentCodeArea()->clearHighlightedBlocksList();

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

		//mCodeArea.appendHtml(curLine);
		currentCodeArea()->appendHtml(curLine);
		
		curLineNumber++;
	}
	currentCodeArea()->addBlockToHighlightNumbers(blockToHighlightNumbers);
	currentCodeArea()->alignControlLines();

	setCurrentFile(fileName);
}

void CodeEditor::setCurrentFile(QString const &fileName)
{
	currentCodeArea()->setConnectedFileName(fileName);
	currentCodeArea()->document()->setModified(false);
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
	out << currentCodeArea()->toFileSaveString();
	//QApplication::restoreOverrideCursor();
	
	setCurrentFile(fileName);
	return true;
}

void CodeEditor::toggleHighlightedLineType()
{
	currentCodeArea()->toggleHighlightedLineType();
}

void CodeEditor::toggleControlLineVisible()
{
	currentCodeArea()->toggleControlLineVisible();
}

void CodeEditor::currentTabChanged(int)
{
	currentCodeArea()->setCompleter(mCompleter);
}

void CodeEditor::tabCloseRequested(int index)
{
	int const currentTabIndex = mCodeAreaTab.currentIndex();
	mCodeAreaTab.setCurrentIndex(index);
	maybeSave();

	if (currentTabIndex != index) {
		mCodeAreaTab.setCurrentIndex(currentTabIndex);
		mCodeAreaTab.removeTab(index);
		return;
	}

	if (mCodeAreaTab.count() == 1) {
		mCodeAreaTab.addTab(new CodeArea, tr("unknown"));
	}
	mCodeAreaTab.removeTab(index);
}
