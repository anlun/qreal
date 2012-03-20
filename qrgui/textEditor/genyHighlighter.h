#pragma once

#include <QSyntaxHighlighter>

namespace qReal {
namespace gui {

class GenyHighlighter : QSyntaxHighlighter {
public:
	GenyHighlighter(QTextDocument *document);
	void highlightBlock(const QString& text);

private:
	struct HighlightingRule
	{
		QRegExp pattern;
		QTextCharFormat format;
	};
	QList<HighlightingRule> mHighlightingRules;

	QTextCharFormat mSingleLineCommentFormat;
	QTextCharFormat mKeywordFormat;
	QTextCharFormat mOpenBraceLineFormat;
	QTextCharFormat mCloseBraceLineFormat;

	QTextCharFormat mInsertionFormat; //@@ @@
	QTextCharFormat mTaskInsertionFormat;//@@! @@

	/*
	QRegExp mCommentStartExpression;
	QRegExp mCommentEndExpression;

	QTextCharFormat mKeywordFormat;
	QTextCharFormat mClassFormat;
	QTextCharFormat mSingleLineCommentFormat;
	QTextCharFormat mMultiLineCommentFormat;
	QTextCharFormat mQuotationFormat;
	QTextCharFormat mFunctionFormat;
	QTextCharFormat mFigitsFormat;
	*/
};

}
}
