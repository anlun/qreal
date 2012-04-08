#include "codeArea.h"
#include "lineNumberArea.h"

using namespace qReal;
using namespace genyCodeEditor;

LineNumberArea::LineNumberArea(CodeArea* codeArea)
	: QWidget(codeArea)
	, mCodeArea(codeArea)
{
}

QSize LineNumberArea::sizeHint() const
{
	return QSize(mCodeArea->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent* e)
{
	mCodeArea->lineNumberAreaPaintEvent(e);
}

void LineNumberArea::mousePressEvent(QMouseEvent* e)
{
	mCodeArea->lineNumberAreaMousePressEvent(e);
}
