#include "fileListItem.h"

using namespace qReal;
using namespace genyCodeEditor;

FileListItem::FileListItem(
		QString const &text
		, QString const &pathToFile
		, QWidget* parent
		) : QLabel(text, parent), mPathToFile(pathToFile)
		//) : QLineEdit(text, parent), mPathToFile(pathToFile)
{
}
