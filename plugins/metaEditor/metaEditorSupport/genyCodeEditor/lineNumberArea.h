#pragma once

#include <QWidget>
#include <QPaintEvent>
#include <QSize>

namespace qReal {
namespace genyCodeEditor {

class CodeArea;

class LineNumberArea : public QWidget {
public:
	LineNumberArea(CodeArea* codeArea);
	QSize sizeHint() const;

protected:
	void paintEvent(QPaintEvent* e);
	void mousePressEvent(QMouseEvent* e);

private:
	CodeArea* mCodeArea;
};

}
}
