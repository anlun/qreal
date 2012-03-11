#include <QtGui>

#include "codeEditor.h"

using namespace qReal;
using namespace gui;

int main(int argv, char **args) {
    QApplication app(argv, args);

    CodeEditor editor("1.cpp");
    editor.setWindowTitle(QObject::tr("Code Editor Example"));
    editor.show();

    return app.exec();
}

