#pragma once

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMap>

namespace genyInterpreter {

class Gemake {
public:
	Gemake(QString const &gemakeFilename);
	~Gemake();

	bool init();
	void make();
	QString getTaskFilename(QString const &taskName) const;

private:
	QFile mMakeFile;
	QMap<QString, QString> *mFilesByTasks;
	QTextStream* mInStream;
	QString mRepoPath;
};

}
