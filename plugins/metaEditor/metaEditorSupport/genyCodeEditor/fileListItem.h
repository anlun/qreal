#pragma once

#include <QLabel>
#include <QLineEdit>

namespace qReal {
namespace genyCodeEditor {

class FileListItem : public QLabel
//class FileListItem : public QLineEdit
{
	Q_OBJECT
public:
		FileListItem(
				QString const &text
				, QString const &pathToFile
				, QWidget* parent = 0
			);

		QString pathToFile() const;
		void setPathToFile(QString const &pathToFile);

private:
		QString mPathToFile;

};

}
}
