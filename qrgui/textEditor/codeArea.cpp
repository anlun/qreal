#include <QDebug>
#include <QtAlgorithms>
#include <QTextCursor>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QPainter>
#include "codeArea.h"

using namespace qReal;
using namespace gui;

CodeArea::CodeArea(QWidget *parent)
	: QTextEdit(parent)
	, mHighlighter(0)
	, mCompleter(0)
	, mLineNumberArea(0)
{
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

	//mHighlighter = new TextHighlighter(document());
	mHighlighter = new GenyHighlighter(document());

	highlightCurrentLine();
	//setReadOnly(true);
	
	mLineNumberArea = new LineNumberArea(this);
	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));
}

CodeArea::~CodeArea()
{
	if (mHighlighter)
		delete mHighlighter;
	if (mLineNumberArea)
		delete mLineNumberArea;
}

int CodeArea::lineNumberAreaWidth() const
{
	int digits = 1;
	int max = qMax(1, blockCount());
	while (max >= 10) {
		max /= 10;
		digits++;
	}

	//9 - the widest symbol
	//3 - just for pretty look 
	return 3 + fontMetrics().width(QLatin1Char('9')) * digits;
}

void CodeArea::updateLineNumberAreaWidth(int /* newBlockCount */)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeArea::updateLineNumberArea(QRect const &rect, int dy) {
	if (dy) {
		lineNumberArea->scroll(0, dy);
	} else {
		lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
	}

	if (rect.contains(viewport()->rect())) {
		updateLineNumberAreaWidth(0);
	}
}

void CodeArea::resizeEvent(QResizeEvent* e)
{
	QTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	mLineNumberArea->setGeometry(QRect(cr.left()), cr.top(), lineNumberAreaWidth(), cr.height());
}

void CodeArea::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;

		QColor lineColor = QColor(Qt::blue).lighter(190);

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	extraSelections.append(highlightedLinesSelectionList());
	setExtraSelections(extraSelections);
}

void CodeArea::setHighlightedLineNumbers(QList<int> const &list)
{
	mHighlightedLineNumbers = list;
}

QList<QTextEdit::ExtraSelection> CodeArea::highlightedLinesSelectionList()
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

void CodeArea::lineNumberAreaPaintEvent(QPaintEvent* e)
{
	QPainter painter(mLineNumberArea);
	painter.fillRect(e->rect(), Qt::lightGray);

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int) blockBoundingRect(block).height();

	while (block.isValid() && top <= e->rect().bottom()) {
		if (block.isVisible() && bottom >= e->rect(.top())) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(Qt::black);
			painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
					Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		blockNumber++;
	}
}

QCompleter* CodeArea::completer() const
{
	return mCompleter;
}

void CodeArea::setCompleter(QCompleter* completer)
{
	if (mCompleter)
		QObject::disconnect(mCompleter, 0, this, 0);

	mCompleter = completer;

	if (!mCompleter)
		return;

	mCompleter->setWidget(this);
	mCompleter->setCompletionMode(QCompleter::PopupCompletion);
	mCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	//mCompleter->setCaseSensitivity(Qt::CaseSensitive);
	QObject::connect(mCompleter, SIGNAL(activated(QString const &)),
			this, SLOT(insertCompletion(QString const &)));
}

void CodeArea::insertCompletion(QString const &completion)
{
	if (mCompleter->widget() != this)
		return;

	QTextCursor tc = textCursor();
	int extraSymbols = completion.length() - mCompleter->completionPrefix().length();
	tc.movePosition(QTextCursor::Left);

	// deleting existing word to replace it
	tc.select(QTextCursor::WordUnderCursor);
	tc.removeSelectedText();

	tc.insertHtml("<b><asdf>" + completion + "</asdf></b>");
	setTextCursor(tc);
}

QString CodeArea::textUnderCursor() const
{
	QTextCursor tc = textCursor();
	tc.select(QTextCursor::WordUnderCursor);
	return tc.selectedText();
}

void CodeArea::focusInEvent(QFocusEvent* e)
{
	if (mCompleter)
		mCompleter->setWidget(this);
	QTextEdit::focusInEvent(e);
}

void CodeArea::keyPressEvent(QKeyEvent* e)
{
	if (mCompleter && mCompleter->popup()->isVisible()) {
		switch (e->key()) {
			case Qt::Key_Enter:
			case Qt::Key_Return:
			case Qt::Key_Escape:
			case Qt::Key_Tab:
			case Qt::Key_Backtab:
				e->ignore();
				return;
			default:
				break;
		}
	}

	bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && (e->key() == Qt::Key_E)); // CTRL+E
	if (!mCompleter || !isShortcut)
		QTextEdit::keyPressEvent(e);

	const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
	if (!mCompleter || (ctrlOrShift && e->text().isEmpty()))
		return;

	static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
	bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
	QString completionPrefix = textUnderCursor();

	if (!isShortcut && (hasModifier || e->text().isEmpty() || completionPrefix.length() < mCompletionStartPrefixSize
				|| eow.contains(e->text().right(1))) ) // MAGIC CONSTANTS!!!
	{
		mCompleter->popup()->hide();
		return;
	}

	if (completionPrefix != mCompleter->completionPrefix()) {
		mCompleter->setCompletionPrefix(completionPrefix);
		
		mCompleter->popup()->setCurrentIndex(
				mCompleter->completionModel()->index(0, 0)
				);
	}

	QRect cr = cursorRect();
	cr.setWidth(mCompleter->popup()->sizeHintForColumn(0)
			+ mCompleter->popup()->verticalScrollBar()->sizeHint().width());
	mCompleter->complete(cr);
}
