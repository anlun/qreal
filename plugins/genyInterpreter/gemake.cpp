#include <QStringList>
#include <QDebug>
#include <QFileInfo>

#include "interpreter.h"
#include "gemake.h"

using namespace qReal;
using namespace genyInterpreter;

//Gemake::Gemake(QString const &gemakeFilename)
Gemake::Gemake(
		QString const &gemakeFilename
		, qrRepo::RepoApi const * repoApi
	)
	: mMakeFile(gemakeFilename)
	, mFilesByTasks(0)
	, mInStream(0)
	, mRepoPath("")
	, mPathToGemakeFile(QFileInfo(gemakeFilename).path())
	, mApi(repoApi)
{
	init();
}

Gemake::~Gemake()
{
	if (!mInStream)
		delete mInStream;
	if (!mFilesByTasks)
		delete mFilesByTasks;
	mMakeFile.close();
}

QString Gemake::getTaskFilename(QString const &taskName) const
{
	return mFilesByTasks->value(taskName);
}

bool Gemake::init()
{
	if (!mMakeFile.isOpen()
		&& mMakeFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		mInStream = new QTextStream(&mMakeFile);
	}
	
	if (!mMakeFile.isOpen() || mInStream == 0) {
		qDebug() << "cannot load make file \"" 
			<< mMakeFile.fileName() << "\"";
	}

	mRepoPath = mInStream->readLine();

	QStringList taskFilenames;
	while (!mInStream->atEnd()) {
		QString const curFilename = mInStream->readLine();
		if (curFilename.trimmed().isEmpty())
			continue;

		taskFilenames.append(mPathToGemakeFile + "/" + curFilename.trimmed());
	}
	
	if (!mFilesByTasks)
		delete mFilesByTasks;
	mFilesByTasks = new QMap<QString, QString>();

	foreach (QString const &curFilename, taskFilenames) {
		QFile curFile(curFilename);
		QTextStream* curStream = 0;
		if (curFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			curStream = new QTextStream(&curFile);
			QString const startLine = curStream->readLine();
			if (startLine.startsWith("Task ")) {
				mFilesByTasks->insert(startLine.mid(5).trimmed(), curFilename);
			}
			else
				qDebug() << "File" 
					<< curFilename
					<< "doesn't start with task name";
		}
		else
			qDebug() << "File" << curFilename << "wasn't found!";
	}

	return true;
}

void Gemake::make()
{
	Interpreter ipreter(
			//mRepoPath
			mApi
			, mFilesByTasks->value("Main")
			, qReal::Id()
			, this
			);
	qDebug() << ipreter.interpret();
}
