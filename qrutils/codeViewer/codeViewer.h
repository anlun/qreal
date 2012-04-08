#pragma once

#include <QPlainTextEdit>
#include <QObject>
#include "textHighlighter.h"

namespace qReal {
namespace gui {

class CodeViewer : public QPlainTextEdit {
	Q_OBJECT

	public:
		CodeViewer(QWidget *parent = 0);
		~CodeViewer();

		void setHighlightedLineNumbers(const QList<int>&);

	private slots:
		void highlightCurrentLine();

	private:
		QList<QTextEdit::ExtraSelection> highlightedLinesSelectionList();

		TextHighlighter *mHighlighter;
		QList<int> mHighlightedLineNumbers;
};

}
}
