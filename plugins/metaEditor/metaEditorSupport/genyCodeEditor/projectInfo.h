#pragma once

#include <QList>
#include <QString>

namespace qReal {
namespace genyCodeEditor {

class ProjectInfo {
public:
	ProjectInfo();
	ProjectInfo(QString const &gemakeFileName);
	
	bool init(QString const &newGemakeFileName);

	QString gemakeFileName() const;
	QString repoPath() const;
	QList<QString> includedFileNames() const;

private:
	QList<QString> mIncludedFileNames;
	QString mRepoPath;
	QString mGemakeFileName;
};

}	
}
