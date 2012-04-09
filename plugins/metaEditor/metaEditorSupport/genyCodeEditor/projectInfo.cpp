#include <QFile>
#include <QTextStream>
#include "projectInfo.h"

ProjectInfo::ProjectInfo()
{
}

ProjectInfo::ProjectInfo(QString const &gemakeFile)
{
	setGemakeFileName(gemakeFile);
}

QString ProjectInfo::gemakeFileName() const
{
	return mGemakeFileName;
}

QString ProjectInfo::repoPath() const
{
	return mRepoPath;
}

QList<QString> ProjectInfo::mIncludedFileNames() const
{
	return mIncludedFileNames;
}

bool ProjectInfo::setGemakeFileName(QString const &newGemakeFileName)
{
	mGemakeFileName = arg;
	mIncludedFileNames.clear();
	mRepoPath = "";

	QFile gemakeFile;
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

		mIncludedFileNames.append(curFileName.trimmed());
	}

	delete inStream;
	
	return true;

}
