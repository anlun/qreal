#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "projectInfo.h"

using namespace qReal;
using namespace genyCodeEditor;

ProjectInfo::ProjectInfo()
{
}

ProjectInfo::ProjectInfo(QString const &gemakeFile)
{
	init(gemakeFile);
}

void ProjectInfo::fileRenamed(QString const &oldName, QString const &newName)
{
	mIncludedFileNames.remove(oldName);
	mIncludedFileNames.insert(newName);
}

void ProjectInfo::addFileName(QString const &newFileName)
{
	mIncludedFileNames.insert(newFileName);
	emit fileAdded(newFileName);
}

QString ProjectInfo::gemakeFileName() const
{
	return mGemakeFileName;
}

QString ProjectInfo::repoPath() const
{
	return mRepoPath;
}

QSet<QString> ProjectInfo::includedFileNames() const
{
	return mIncludedFileNames;
}

bool ProjectInfo::init(QString const &newGemakeFileName)
{
	mGemakeFileName = newGemakeFileName;
	mIncludedFileNames.clear();
	mRepoPath = "";

	QFile gemakeFile(mGemakeFileName);
	QTextStream* inStream;

	if (!gemakeFile.isOpen()
		&& gemakeFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		inStream = new QTextStream(&gemakeFile);
	}
	
	if (!gemakeFile.isOpen() || inStream == 0) {
		qDebug() << "cannot load make file \"" 
			<< gemakeFile.fileName() << "\"";
		return false;
	}

	mRepoPath = inStream->readLine();

	while (!inStream->atEnd()) {
		QString const curFileName = inStream->readLine();
		if (curFileName.trimmed().isEmpty())
			continue;

		mIncludedFileNames.insert(curFileName.trimmed());
	}

	delete inStream;
	
	return true;

}
