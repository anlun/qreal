#pragma once

#include <QTextEdit>
#include <QPlainTextEdit>
#include <QObject>
#include <QCompleter>
#include <QFocusEvent>
#include <QKeyEvent>
#include "syntaxHighlighter.h"
#include "lineNumberArea.h"

namespace qReal {
namespace genyCodeEditor {

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

	void alignControlLines();

	/// Change that lines to highlight - control or not
	void toggleHighlightedLineType();

	/// Change visibility of control lines
	void toggleControlLineVisible();

	void setConnectedFileName(QString const &newFileName);
	QString connectedFileName();

	QString toFileSaveString();

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
			, int dy             ///< change in y coord
			);

private:
	static int const mCompletionStartPrefixSize = 3;

	QTextEdit::ExtraSelection highlightCurrentLine();
	QList<QTextEdit::ExtraSelection> highlightedLinesSelectionList();
	QList<QTextEdit::ExtraSelection> highlightedBlocksSelectionList();
	QString textUnderCursor() const;

	SyntaxHighlighter* mHighlighter;
	QList<int> mHighlightedLineNumbers;
		
	QCompleter* mCompleter;
	LineNumberArea* mLineNumberArea;
	QList<QTextBlock> mControlBlocks;

	bool mAreControlLinesNeededToBeHighlighted;
	bool mAreControlLinesNeededToBeShowed;

	// name of connected file, to that will be
	// saved content in case of save event (not save as)
	QString mFileName;
};

}
}
