#pragma once

#include <QStringList>

namespace qReal {
namespace genyCodeEditor {

class ProjectInfo {
public:
	ProjectInfo();
	ProjectInfo(QString const &gemakeFileName);
	
	bool init(QString const &newGemakeFileName);

	QString gemakeFileName() const;
	QString repoPath() const;
	QStringList includedFileNames() const;

private:
	QStringList mIncludedFileNames;
	QString mRepoPath;
	QString mGemakeFileName;
};

}	
}
