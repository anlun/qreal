#pragma once

#include <QSet>

namespace qReal {
namespace genyCodeEditor {

class ProjectInfo: public QObject {
	Q_OBJECT

public:
	ProjectInfo();
	ProjectInfo(QString const &gemakeFileName);
	
	bool init(QString const &newGemakeFileName);

	void addFileName(QString const &newFileName);

	QString gemakeFileName() const;
	QString repoPath() const;
	QSet<QString> includedFileNames() const;

public slots:
	void fileRenamed(QString const &oldName, QString const &newName);

signals:
	void fileAdded(QString const &newFileName);

private:
	QSet<QString> mIncludedFileNames;
	QString mRepoPath;
	QString mGemakeFileName;
};

}	
}
