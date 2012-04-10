#include <QtGui>

#include "codeEditor.h"

using namespace qReal;
using namespace genyCodeEditor;

int main(int argv, char **args) {
    QApplication app(argv, args);

    CodeEditor editor("gemake.ex");
    //editor.setWindowTitle(QObject::tr("Code Editor Example"));
    editor.show();

    return app.exec();
}

