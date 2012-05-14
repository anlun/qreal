#pragma once

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMap>

#include "../../qrrepo/repoApi.h"

namespace qReal {
namespace genyInterpreter {

class Gemake {
public:
	//TODO: make normal RepoApi initialization by first line of gemake file
	//Gemake(QString const &gemakeFilename);
	Gemake(QString const &gemakeFilename, qrRepo::RepoApi const * repoApi);
	~Gemake();

	bool init();
	void make();
	QString getTaskFilename(QString const &taskName) const;

private:
	QFile mMakeFile;
	QMap<QString, QString> *mFilesByTasks;
	QTextStream *mInStream;
	QString mRepoPath;

	QString mPathToGemakeFile;

	qrRepo::RepoApi const *mApi;
};

}
}
