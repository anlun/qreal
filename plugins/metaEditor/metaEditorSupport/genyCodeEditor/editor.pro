######################################################################
# Automatically generated by qmake (2.01a) Fri Dec 16 20:51:26 2011
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += \
	codeArea.h \
	codeEditor.h \
	syntaxHighlighter.h \
	lineNumberArea.h \
	projectInfo.h \
	fileListItem.h \
	fileListDock.h

SOURCES += \
	codeArea.cpp \
	codeEditor.cpp \
	syntaxHighlighter.cpp \
	main.cpp \
	lineNumberArea.cpp \
	projectInfo.cpp \
	fileListItem.cpp \
	fileListDock.cpp

LIBS += -L../../../../bin -lqrkernel -lqrutils -lqrmc -lqrrepo
QMAKE_LFLAGS="-Wl,-O1,-rpath,$(PWD)/../../../../bin"
