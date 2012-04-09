#pragma once

#include <QList>
#include <QString>

namespace qReal {
namespace genyCodeEditor {

class ProjectInfo {
public:
	ProjectInfo();
	ProjectInfo(QString const &gemakeFileName);

	QString gemakeFileName() const;
	bool setGemakeFileName(QString const &newGemakeFileName);

	QString repoPath() const;
	QList<QString> mIncludedFileNames() const;

private:
	QList<QString> mIncludedFileNames;
	QString mRepoPath;
	QString mGemakeFileName;
};

}	
}
