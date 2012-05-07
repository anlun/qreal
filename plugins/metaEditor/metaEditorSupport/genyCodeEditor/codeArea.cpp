#include <QDebug>
#include <QtAlgorithms>
#include <QTextCursor>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QPainter>
#include "codeArea.h"

using namespace qReal;
using namespace genyCodeEditor;

CodeArea::CodeArea(QWidget *parent)
	: QPlainTextEdit(parent)
	, mHighlighter(0)
	, mCompleter(0)
	, mLineNumberArea(0)
	, mAreControlLinesNeededToBeHighlighted(false)
	, mAreControlLinesNeededToBeShowed(true)
{
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightLines()));

	mHighlighter = new SyntaxHighlighter(document());

	highlightLines();
	setLineWrapMode(QPlainTextEdit::NoWrap);
	
	mLineNumberArea = new LineNumberArea(this);
	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));

	alignControlLines();

	// Need to resolve bug of invisible first character in line
	// if CodeArea was empty before typing
	appendPlainText("\n");
	document()->clear();
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

	//'9' - the widest symbol
	//3 - just for pretty look 
	return 3 + fontMetrics().width(QLatin1Char('9')) * digits;
}

void CodeArea::updateLineNumberAreaWidth(int /* newBlockCount */)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeArea::updateLineNumberArea(QRect const &rect, int dy) {
	if (dy) {
		mLineNumberArea->scroll(0, dy);
	} else {
		mLineNumberArea->update(0, rect.y(), mLineNumberArea->width(), rect.height());
	}

	if (rect.contains(viewport()->rect())) {
		updateLineNumberAreaWidth(0);
	}
}

void CodeArea::resizeEvent(QResizeEvent* e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	mLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeArea::highlightLines()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	extraSelections.append(highlightCurrentLine());
	extraSelections.append(highlightedLinesSelectionList());
	extraSelections.append(highlightedBlocksSelectionList());

	setExtraSelections(extraSelections);
}

QTextEdit::ExtraSelection CodeArea::highlightCurrentLine()
{
	QTextEdit::ExtraSelection selection;

	if (!isReadOnly()) {

		QColor lineColor = QColor(Qt::blue).lighter(190);

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
	}

	return selection;
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

QList<QTextEdit::ExtraSelection> CodeArea::highlightedBlocksSelectionList()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;

		QColor lineColor = QColor(Qt::gray).lighter(140);

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);

		//QTextBlock block = firstVisibleBlock();
		QTextBlock block = document()->firstBlock();
		while (block.isValid()) {
			//if (!mAreControlLinesNeededToBeShowed && 
			//		mControlBlocks.contains(block))
			//	block.setVisible(false);

			if (block.isVisible()) {
				bool contains = mControlBlocks.contains(block);
				if ( !(contains ^ mAreControlLinesNeededToBeHighlighted) ) {
					selection.cursor = QTextCursor(block);
					selection.cursor.clearSelection();
					//selection.cursor.select(QTextCursor::LineUnderCursor);
					extraSelections.append(selection);
				}
			}

			block = block.next();
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
		if (block.isVisible() && bottom >= e->rect().top()) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(Qt::black);
			if (mControlBlocks.contains(block)) {
				painter.setPen(Qt::red);
			}

			painter.drawText(0, top, mLineNumberArea->width(), fontMetrics().height(),
					Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		blockNumber++;
	}
}

void CodeArea::lineNumberAreaMousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton) {
		QTextCursor const cursorForMousePoint = cursorForPosition(e->pos());
		QTextBlock const curBlock = cursorForMousePoint.block();

		if (mControlBlocks.contains(curBlock)) {
			mControlBlocks.removeAll(curBlock);
		} else {
			mControlBlocks.append(curBlock);
		}
	}

	QPlainTextEdit::mousePressEvent(e);

	alignControlLines();
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

	tc.insertHtml("<b>" + completion + "</b>");
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
	QPlainTextEdit::focusInEvent(e);
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
		QPlainTextEdit::keyPressEvent(e);

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

void CodeArea::clearHighlightedBlocksList()
{
	mControlBlocks.clear();
	highlightLines(); //to update block changing
}

void CodeArea::addBlockToHighlightNumbers(QList<int> const &blockNumbers)
{
	foreach (int number, blockNumbers) {
		mControlBlocks.append(document()->findBlockByNumber(number));
	}
	highlightLines(); //to update block changing
}

void CodeArea::alignControlLines()
{
	int curTabNumber = 0;
	//foreach (QTextBlock const &block, mControlBlocks) { // blocks must be in logical order!!!

	QTextBlock block = document()->firstBlock();
	while (block.isValid()) {
		if (!mControlBlocks.contains(block)) {
			block = block.next();
			continue;
		}

		QString text = block.text().trimmed();

		curTabNumber = std::max(0
			, curTabNumber + tabulationChangeOnLine(text));
		/*
		if (text.startsWith("}") && (curTabNumber > 0)) {
			curTabNumber--;
		}
		*/

		QTextCursor cursor(block);
		cursor.insertText(QString(curTabNumber, '\t'));
		cursor.insertHtml(text);
		cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
		cursor.removeSelectedText();

		curTabNumber += tabulationChangeAfterLine(text);
		/*
		if (text.startsWith("{")) {
			curTabNumber++;
		}
		*/

		block = block.next();
	}
}

void CodeArea::toggleHighlightedLineType()
{
	mAreControlLinesNeededToBeHighlighted = !mAreControlLinesNeededToBeHighlighted;
	highlightLines();
}

void CodeArea::toggleControlLineVisible()
{
	mAreControlLinesNeededToBeShowed = !mAreControlLinesNeededToBeShowed;
	foreach(QTextBlock block, mControlBlocks) {
		block.setVisible(mAreControlLinesNeededToBeShowed);
	}
	highlightLines();

	//resizeEvent(new QResizeEvent(size(), size()));
}

QString CodeArea::connectedFileName()
{
	return mFileName;
}

void CodeArea::setConnectedFileName(QString const &newFileName)
{
	mFileName = newFileName;
}

QString CodeArea::toFileSaveString()
{
	QString result;

	QTextBlock block = document()->firstBlock();
	while (block.isValid()) {
		QString text = block.text();

		if (mControlBlocks.contains(block)) {
			text = text.trimmed();

			// it means control string starts block
			if (tabulationChangeAfterLine(text) > 0) {
				text += "\n#!{";
			}

			text = "#!" + text;
		}
		result += text + "\n";

		block = block.next();
	}
	
	return result;
}

int CodeArea::tabulationChangeOnLine(QString const &str)
{
	if (str.startsWith("}")) {
		return -1;
	}
	return 0;
}

int CodeArea::tabulationChangeAfterLine(QString const &str)
{
	if (
		str.startsWith("foreach") ||
		str.startsWith("toFile") ||
		str.startsWith("switch") ||
		str.startsWith("case")
	)
	{
		return 1;
	}
	return 0;
}
