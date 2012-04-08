#include <QtAlgorithms>
#include "codeViewer.h"

using namespace qReal::gui;

CodeViewer::CodeViewer(QWidget *parent): QPlainTextEdit(parent)
{
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

	mHighlighter = new TextHighlighter(document());

	highlightCurrentLine();
	setReadOnly(true);
}

CodeViewer::~CodeViewer()
{
	if (mHighlighter)
		delete mHighlighter;
}

void CodeViewer::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;

		QColor lineColor = QColor(Qt::blue).lighter(160);

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	extraSelections.append(highlightedLinesSelectionList());
	setExtraSelections(extraSelections);
}

void CodeViewer::setHighlightedLineNumbers(const QList<int>& list)
{
	mHighlightedLineNumbers = list;
}

QList<QTextEdit::ExtraSelection> CodeViewer::highlightedLinesSelectionList()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;

		QColor lineColor = QColor(Qt::red).lighter(160);

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);

		foreach (int lineNum, mHighlightedLineNumbers) {
			selection.cursor = QTextCursor(document()->findBlockByNumber(lineNum));
			selection.cursor.clearSelection();
			//selection.cursor.select(QTextCursor::LineUnderCursor);
			selection.cursor.select(QTextCursor::BlockUnderCursor);
			extraSelections.append(selection);
		}
	}

	return extraSelections;
}
