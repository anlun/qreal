#pragma once

#include <QPair>
#include <QString>
#include <QFile>
#include <QTextStream>
//#include "../../../qrrepo/repoApi.h" // когда в транке лежит
#include "../../../../unreal/trunk/qrrepo/repoApi.h" // когда лежит в tools

namespace qReal {
namespace genyInterpreter {

class Gemake;

class Interpreter {
public:
	Interpreter(QString const &repoPath, QString const &taskFilename,
			Id curObject, Gemake* gemaker);
	~Interpreter();

	QString interpret();

private:
	//Can move cursor position in stream!
	QString interpret(QTextStream& stream);

	//Control string starts with #!
	bool isControlString(QString const &) const;

	enum ControlStringType {
		commentType, foreachType,
		forType,
		leftBraceType, rightBraceType,
		toFileType, saveObjType,
		switchType, caseType,
		notControlType
	};
	ControlStringType controlStringType(QString const &) const;

	//Can move cursor position in stream!
	QString notControlStringParse(QString const &);
	//Can move cursor position in stream!
	QString controlStringParse(QString const &, QTextStream& stream);

	//Returns a pair of the type name of searching objects
	//and in that list they are searched
	QPair<QString, QString> foreachStringParse(QString const &);

	//Returns a method name in 'for' string
	QString forStringParse(QString const &);

	//Returns filename to write 
	QString toFileStringFilename(QString const &);

	//Returns a property name for the switch
	QString switchStringParse(QString const &);

	//Returns a case value
	QString caseStringParse(QString const &);

	QString saveObjLabel(QString const &);
	void addLabel(QString const &);

	//Control expression between @@ @@
	QString controlExpressionParse(QString const &expression);
	QString getBraceBlock(QTextStream&);

	QPair<QString, QString> getNextCaseBlock(QTextStream&);

	QString getObjProperty(Id const &objectId,
			QString const &propertyName) const;
	QString getCurObjProperty(QString const &propertyName) const;

	Id getCurObjectMethodResult(QString const &methodName);
	IdList getCurObjectMethodResultList(QString const &);

	//нужно, так как возможно использование списка Id вместо одного
	Id getCurObjId() const;

	QFile mTaskFile;
	QTextStream* mInStream;

	qrRepo::RepoApi rApi;
	QString mRepoPath;

	Gemake* mGeMaker;

	Id mCurrentObjectId;
	QMap<QString, Id> mObjectsByLabels;
};

}
}
