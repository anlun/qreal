#pragma once

#include <QTextEdit>
#include <QPlainTextEdit>
#include <QObject>
#include <QCompleter>
#include <QFocusEvent>
#include <QKeyEvent>
#include "textHighlighter.h"
#include "genyHighlighter.h"
#include "lineNumberArea.h"

namespace qReal {
namespace gui {

class CodeArea : public QPlainTextEdit {
	Q_OBJECT

public:
	CodeArea(QWidget *parent = 0);
	~CodeArea();

	void setHighlightedLineNumbers(QList<int> const &);
		
	void setCompleter(QCompleter* completer);
	QCompleter* completer() const;

	void lineNumberAreaPaintEvent(QPaintEvent* e);
	void lineNumberAreaMousePressEvent(QMouseEvent* e);
	int lineNumberAreaWidth() const;

	void clearHighlightedBlocksList();
	void addBlockToHighlightNumbers(QList<int> const &blockNumbers);

protected:
	void keyPressEvent(QKeyEvent* e);
	void focusInEvent(QFocusEvent* e);
	void resizeEvent(QResizeEvent* e);

private slots:
	/// Highlights current line, control blocks, lines from mHighlightedLineNumbers
	void highlightLines();

	void insertCompletion(QString const &completion);
	void updateLineNumberAreaWidth(int newBlockCount);
	void updateLineNumberArea(
			QRect const &rect
			, int dy             ///< change in y
			);

private:
	static const int mCompletionStartPrefixSize = 3;

	QTextEdit::ExtraSelection highlightCurrentLine();
	QList<QTextEdit::ExtraSelection> highlightedLinesSelectionList();
	QList<QTextEdit::ExtraSelection> highlightedBlocksSelectionList();
	QString textUnderCursor() const;

	//TextHighlighter *mHighlighter;
	GenyHighlighter* mHighlighter;
	QList<int> mHighlightedLineNumbers;
		
	QCompleter* mCompleter;

	LineNumberArea* mLineNumberArea;

	QList<QTextBlock> mHighlightedBlocks;
};

}
}
