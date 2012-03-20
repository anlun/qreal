#include <QDebug>
#include <QtAlgorithms>
#include <QTextCursor>
#include <QAbstractItemView>
#include <QScrollBar>
#include "codeArea.h"

using namespace qReal;
using namespace gui;

CodeArea::CodeArea(QWidget *parent)
	: QTextEdit(parent)
	, mHighlighter(0)
	, mCompleter(0)
{
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

	//mHighlighter = new TextHighlighter(document());
	mHighlighter = new GenyHighlighter(document());

	highlightCurrentLine();
	//setReadOnly(true);
}

CodeArea::~CodeArea()
{
	if (mHighlighter)
		delete mHighlighter;
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
