#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStringListModel>
#include <QCursor>
#include <QMessageBox>
#include "codeEditor.h"

using namespace qReal;
using namespace gui;

CodeEditor::CodeEditor(QWidget *parent)
	: QMainWindow(parent)
	, mOpenAct(0)
	, mSaveAct(0)
	, mCodeArea(this)
	, mCompleter(0)
{
	setCentralWidget(&mCodeArea);
	
	initCompleter();
	createActions();
}

CodeEditor::CodeEditor(const QString& filename, QWidget *parent)
	: QMainWindow(parent)
	, mOpenAct(0)
	, mSaveAct(0)
	, mCodeArea(this)
	, mCompleter(0)
{
	setCentralWidget(&mCodeArea);
	QFile file(filename);
	QTextStream *inStream = 0;
	if (!file.isOpen() && file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		inStream = new QTextStream(&file);
	}

	if (inStream) {
		mCodeArea.document()->setPlainText(inStream->readAll());
		delete inStream;
	}
	
	initCompleter();
	createActions();
}

CodeEditor::~CodeEditor()
{
	if (mCompleter) {
		mCodeArea.setCompleter(0);
		delete mCompleter;
	}
	mCompleter = 0;

	if (mOpenAct) {
		delete mOpenAct;
	}
	mOpenAct = 0;

	if (mSaveAct) {
		delete mSaveAct;
	}
	mSaveAct = 0;
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
	mOpenAct = new QAction(tr("Open"), this);
	mOpenAct->setStatusTip(tr("Open an existing file"));
	connect(mOpenAct, SIGNAL(triggered()), this, SLOT(open()));

	mSaveAct = new QAction(tr("Save"), this);
	mSaveAct->setStatusTip(tr("Save the document to disk"));
	connect(mSaveAct, SIGNAL(triggered()), this, SLOT(save()));
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
	//TODO
}

bool CodeEditor::save()
{
	//TODO
	return true;
}

bool CodeEditor::maybeSave()
{
	if (mCodeArea.document()->isModified()) {
		QMessageBox::StandardButton ret;
		ret = QMessageBox::warning(this, tr("Code editor"),
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
