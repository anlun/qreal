#pragma once

#include <QTextEdit>
#include <QObject>
#include <QCompleter>
#include <QFocusEvent>
#include <QKeyEvent>
#include "textHighlighter.h"
#include "genyHighlighter.h"

class CodeArea : public QTextEdit {
	Q_OBJECT

	public:
		CodeArea(QWidget *parent = 0);
		~CodeArea();

		void setHighlightedLineNumbers(QList<int> const &);
		
		void setCompleter(QCompleter* completer);
		QCompleter* completer() const;

	protected:
		void keyPressEvent(QKeyEvent* e);
		void focusInEvent(QFocusEvent* e);

	private slots:
		void highlightCurrentLine();
		void insertCompletion(QString const &completion);

	private:
		static const int mCompletionStartPrefixSize = 3;

		QList<QTextEdit::ExtraSelection> highlightedLinesSelectionList();
		QString textUnderCursor() const;

		//TextHighlighter *mHighlighter;
		GenyHighlighter* mHighlighter;
		QList<int> mHighlightedLineNumbers;
		
		QCompleter* mCompleter;
};
