#include <QDebug>
#include <QStringList>
#include "interpreter.h"
#include "gemake.h"

using namespace qReal;
using namespace genyInterpreter;

Interpreter::Interpreter(
		QString const &repoPath
		, QString const &taskFilename
		, Id curObjectId
		, Gemake* geMaker
		)
	: mTaskFile(taskFilename)
	, mInStream(0)
	, mRepoApi(new qrRepo::RepoApi(repoPath))
	, mNeedToDeleteRepoApi(true)
	, mGeMaker(geMaker)
	, mCurrentObjectId(curObjectId)
{
}

Interpreter::Interpreter(
		qrRepo::RepoApi const *repoApi
		, QString const &taskFilename
		, Id curObjectId
		, Gemake* geMaker
		)
	: mTaskFile(taskFilename)
	, mInStream(0)
	, mRepoApi(repoApi)
	, mNeedToDeleteRepoApi(false)
	, mGeMaker(geMaker)
	, mCurrentObjectId(curObjectId)
{
}

Interpreter::~Interpreter()
{
	if (mInStream)
		delete mInStream;
	mTaskFile.close();

	if (mNeedToDeleteRepoApi && mRepoApi) {
		delete mRepoApi;
	}
}

Id Interpreter::getCurObjId() const
{
	return mCurrentObjectId;
}

QString Interpreter::controlExpressionParse(QString const &expression)
{
	if (expression.at(0) != '!') {
		if (!expression.contains('@'))
			return getCurObjProperty(expression);
		else {
			QStringList listOfSplitting = expression.split("@");
			if (listOfSplitting.size() == 2) {
				return getObjProperty(mObjectsByLabels[listOfSplitting[0].trimmed()], listOfSplitting[1].trimmed());
			}
			else
				qDebug() << "Fail in \'@@ @ @@\' expression";
		}
	}
	else if (expression.startsWith("!task ")){
		QString subTaskName = expression.mid(6).trimmed();

		Interpreter ipreter(mRepoApi, mGeMaker->getTaskFilename(subTaskName), getCurObjId(), mGeMaker);
		return ipreter.interpret();
	} else
		qDebug() << "Fail in @@! expression";
	
	return "";
}

QString Interpreter::getObjProperty(Id const &objectId, QString const &propertyName) const
{
	Id const logicalId = mRepoApi->logicalId(objectId);
	Id const usedId = logicalId != Id() ? logicalId : objectId;

	if (propertyName == "uniqueName") {
		return mGeMaker->getUniqueName(usedId);
	}

	if (!mRepoApi->exist(usedId)) {
		qDebug() << "Error! Trying to work with not existed element Id in current repository!";
		return ""; //TODO: возможно лучше бросать исключение!
	}

	if (!mRepoApi->hasProperty(usedId, propertyName)) {
		qDebug() << "Error! Trying to get not existed property of current element!";
		qDebug() << propertyName << usedId;
		return ""; //TODO: возможно лучше бросать исключение!
	}

	return mRepoApi->property(usedId, propertyName).toString();
}

QString Interpreter::getCurObjProperty(QString const &propertyName) const
{
	return getObjProperty(getCurObjId(), propertyName);
}

Interpreter::ControlStringType Interpreter::controlStringType(QString const &str) const
{
	QString workStr = str.trimmed();
	if (!workStr.startsWith("#!"))
		return notControlType;

	workStr = workStr.right(workStr.length() - 2).trimmed();//убираем #!

	if (workStr.startsWith("/"))
		return commentType;
	if (workStr.startsWith("foreach "))
		return foreachType;	
	if (workStr.startsWith("for "))
		return forType;
	if (workStr.startsWith("{"))
		return leftBraceType;
	if (workStr.startsWith("}"))
		return rightBraceType;
	if (workStr.startsWith("toFile"))
		return toFileType;
	if (workStr.startsWith("saveObj"))
		return saveObjType;
	if (workStr.startsWith("switch"))
		return switchType;
	if (workStr.startsWith("case") || workStr.startsWith("default"))
		return caseType;
	if (workStr.startsWith("if"))
		return ifType;

	return notControlType;
}

bool Interpreter::isControlString(QString const &str) const
{
	return controlStringType(str) != notControlType ? true : false;
}

QPair<QString, QString> Interpreter::foreachStringParse(QString const &str)
{
	QStringList strElements = str.split(' ');
	strElements.removeAll("");

	if ( (strElements.length() != 4) || 
			(strElements[0] != "#!foreach") || (strElements[2] != "in") ) {
		qDebug()  << "Error! Bad \'foreach\' structure!";
		return QPair<QString, QString>("", ""); //TODO: возможно лучше бросать исключение!
	}

	/*
	strElements.at(1) : elementsType;
	strElements.at(3) : elementsListName;
	*/

	return QPair<QString, QString>(strElements.at(1), strElements.at(3));
}

QString Interpreter::forStringParse(QString const &str)
{
	QStringList strElements = str.split(' ');
	strElements.removeAll("");

	if ( (strElements.length() != 2) || 
			(strElements[0] != "#!for")) {
		qDebug()  << "Error! Bad \'for\' structure!";
		return QString(); //TODO: возможно лучше бросать исключение!
	}

	/*
	strElements.at(1) : that method to call;
	ex : #!for to - strElements.at(1) = "to"
	ex : #!for parent - strElements.at(1) = "parent"
	*/

	return strElements.at(1);
}

QString Interpreter::toFileStringFilename(QString const &str)
{
	QStringList strElements = str.split(' ');
	strElements.removeAll("");

	if ( (strElements.length() != 2) || 
			(strElements[0] != "#!toFile")) {
		qDebug()  << "Error! Bad \'toFile\' structure!";
		return ""; //TODO: возможно лучше бросать исключение!
	}

	return notControlStringParse(strElements[1]);
}

QString Interpreter::switchStringParse(QString const &str)
{
	QStringList strElements = str.split(' ');
	
	if ( (strElements.length() != 2) || 
			(strElements[0] != "#!switch")) {
		qDebug()  << "Error! Bad \'switch\' structure!";
		return ""; //TODO: возможно лучше бросать исключение!
	}

	return strElements[1].trimmed();
}

QString Interpreter::caseStringParse(QString const &str)
{
	QStringList const strElements = str.split(' ');

	if (controlStringType(strElements[0]) != caseType) {
		qDebug()  << "Error! Bad \'case\' structure!";
		return ""; //TODO: возможно лучше бросать исключение!
	}

	if (strElements[0].startsWith("#!default"))
		return "";

	QString caseValue = str.mid(7).trimmed();//отрезаем "#!case "
	int const firstApostoIndex = caseValue.indexOf("\'");
	int const lastApostoIndex = caseValue.lastIndexOf("\'");

	if (firstApostoIndex == -1) {
		qDebug() << "Error! Bad \'case\' structure!";
		return "";
	}

	return caseValue.mid(firstApostoIndex + 1, lastApostoIndex - firstApostoIndex - 1);
}

QString Interpreter::IfStringParseHelper::alphabet()
{	
	return "абвгдеёжзиклмнопрстуфхцчшщъыьэюя"\
		"АБВГДЕЁЖЗИКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
}

QString Interpreter::IfStringParseHelper::expressionTemplate()
{	
	return "[" + IfStringParseHelper::alphabet() + "a-zA-Z0-9 ]*"; //TODO: problem of empty property name may occur
}
	
Interpreter::IfStringParseHelper::IfControlStringType
	Interpreter::IfStringParseHelper::stringType(QString const &str)
{
	QRegExp const positiveStatement("#!if %" + IfStringParseHelper::expressionTemplate() + "% == %" + IfStringParseHelper::expressionTemplate() + "%");
	if (positiveStatement.exactMatch(str)) {
		return EqualStatement;
	}

	QRegExp const negativeStatement("#!if %" + IfStringParseHelper::expressionTemplate() + "% != %" + IfStringParseHelper::expressionTemplate() + "%");
	if (negativeStatement.exactMatch(str)) {
		return InequalStatement;
	}

	QRegExp const containsStatement("#!if %" + IfStringParseHelper::expressionTemplate() + "% contains %" + IfStringParseHelper::expressionTemplate() + "%");
	if (containsStatement.exactMatch(str)) {
		return ContainsStatement;
	}

	return NoIfStatement;
}

bool Interpreter::ifStringParse(QString const &str)
{
	QString const workStr = str.toUtf8();

	IfStringParseHelper::IfControlStringType currentType = IfStringParseHelper::stringType(workStr);
	if (currentType == IfStringParseHelper::NoIfStatement) {
		qDebug() << "Error! Bad \'if\' structure!";
		return false;
	}

	QString const ruleParamStatement = "%" + IfStringParseHelper::expressionTemplate() + "%";
	QRegExp const paramStatement(ruleParamStatement);
	int pos = paramStatement.indexIn(workStr);
	QString propertyName = paramStatement.capturedTexts().at(0);
	paramStatement.indexIn(workStr, pos + 1);
	QString value = paramStatement.capturedTexts().at(0);
	
	// Deleting '%', '%'
	propertyName = propertyName.mid(1, propertyName.size() - 2);
	value = value.mid(1, value.size() - 2);
	QString const property = getCurObjProperty(propertyName).toUtf8();

	switch (currentType) {
		case IfStringParseHelper::EqualStatement:
			{
				return property == value;
			}
		case IfStringParseHelper::InequalStatement:
			{
				return property != value;
			}
		case IfStringParseHelper::ContainsStatement:
			{
				return property.contains(value);
			}
		case IfStringParseHelper::NoIfStatement:
			{
				return false;
			}
	}

	return false;
}

QString Interpreter::saveObjLabel(QString const &str)
{
	return str.mid(9).trimmed();//object label
}

void Interpreter::addLabel(QString const& str) {
	mObjectsByLabels.insert(str, getCurObjId());
}

//Для обращения к методу elementsByType передается "elementsByType(__type_name__)"
IdList Interpreter::getCurObjectMethodResultList(QString const &methodName)
{
	if (methodName == "children")
		return mRepoApi->children(getCurObjId());

	if (methodName == "outgoingLinks")
		return mRepoApi->outgoingLinks(getCurObjId());

	if (methodName == "incomingLinks")
		return mRepoApi->incomingLinks(getCurObjId());

	if (methodName == "links")
		return mRepoApi->links(getCurObjId());

	if (methodName == "outgoingConnections")
		return mRepoApi->outgoingConnections(getCurObjId());

	if (methodName == "incomingConnections")
		return mRepoApi->incomingConnections(getCurObjId());

	if (methodName == "outgoingUsages")
		return mRepoApi->outgoingUsages(getCurObjId());

	if (methodName == "incomingUsages")
		return mRepoApi->incomingUsages(getCurObjId());

	if (methodName == "connectedElements")
		return mRepoApi->connectedElements(getCurObjId());

	if (methodName == "outgoingConnectedElements") {
		return mRepoApi->outgoingConnectedElements(getCurObjId());
	}

	if (methodName == "incomingConnectedElements")
		return mRepoApi->incomingConnectedElements(getCurObjId());

	//Для обращения к методу elementsByType передается "elementsByType(__type_name__)"
	if (methodName.startsWith("elementsByType")) {
		QString const elementsType = getArgument(methodName);

		if (elementsType.isEmpty())
			return IdList();

		//return mRepoApi->elementsByType(elementsType);
		return mRepoApi->logicalElements(Id("", "", elementsType, ""));
	}

	// These methods don't have absolute equal in repo
	if (methodName == "elements")
		//return mRepoApi->logicalElements();
		return mRepoApi->graphicalElements();

	qDebug() << "Error! Uses unknown RepoApi list method!";

	return IdList();
}

Id Interpreter::getCurObjectMethodResult(QString const &methodName)
{	
	if (methodName == "parent")
		return mRepoApi->parent(getCurObjId());

	if (methodName == "to")
		return mRepoApi->to(getCurObjId());

	if (methodName == "from")
		return mRepoApi->from(getCurObjId());

	return Id();
}

QString Interpreter::notControlStringParse(QString const &parsingStr)
{
	//Обработка @@_smth_@@
	QStringList listOfSplitting = parsingStr.split("@@");
	if (listOfSplitting.length() % 2 == 0) {
		qDebug() << "problem with number of @@ in task" << mTaskFile.fileName();
		return "";
	}

	//теперь каждый нечетный элемент listOfSplitting - что-то между @@ @@
	int iterationNumber = 0;
	foreach (QString curElem, listOfSplitting) {
		if (iterationNumber % 2 == 1)
			listOfSplitting.replace(iterationNumber, controlExpressionParse(curElem));
		iterationNumber++;
	}

	QString resultStr;
	foreach (QString curElem, listOfSplitting) {
		resultStr += curElem;
	}

	return resultStr;
}

QString Interpreter::getBraceBlock(QTextStream& stream)
{
	QString resultStr;

	QString curLine = stream.readLine();
	if (controlStringType(curLine) != leftBraceType) {
		qDebug() << "Error! After block operator not #!{ but \'" << curLine << "\' found!";
		return "";
	}

	int braceBalance = 1;
	curLine = "";

	while ( (braceBalance != 0) && !stream.atEnd() ) {
		resultStr += curLine + "\n";

		curLine  = stream.readLine();
		if (controlStringType(curLine) == leftBraceType)
			braceBalance++;
		if (controlStringType(curLine) == rightBraceType)
			braceBalance--;
	}

	if ( (braceBalance != 0) && (stream.atEnd()) ) {
		qDebug() << "Error! There is no brace balance!";
		return "";
	}

	return resultStr;
}

QPair<QString, QString> Interpreter::getNextCaseBlock(QTextStream& stream)
{
	if (stream.atEnd())
		return QPair<QString, QString>();

	QString caseStr = stream.readLine();

	if (controlStringType(caseStr) != caseType) {
		qDebug() << "Error! There is must be case string but \'" << caseStr << "\' found!";
		return QPair<QString, QString>();
	}

	QString braceBlock = getBraceBlock(stream);
	
	QTextStream caseBlockStream(&braceBlock);

	return QPair<QString, QString>(caseStringParse(caseStr), interpret( caseBlockStream ));
}

QString Interpreter::controlStringParse(QString const& parsingStr, QTextStream& stream)
{
	switch (controlStringType(parsingStr)) {
		case commentType:
			return "";
		case foreachType:
			{
				QString braceBlock = getBraceBlock(stream);
				QTextStream foreachBlockStream(&braceBlock);

				QString resultStr;

				QPair<QString, QString> elemAndListNames = foreachStringParse(parsingStr);
				Id objectId = getCurObjId();//TODO: change this method

				//qDebug() << getCurObjectMethodResultList(elemAndListNames.second);
				//qDebug() << elemAndListNames;

				// Здесь развертка foreach
				foreach (Id element, getCurObjectMethodResultList(elemAndListNames.second)) {
					if (elemAndListNames.first == "." || element.element() == elemAndListNames.first) {
						//обновление mCurrentObjectId
						mCurrentObjectId = element;
						
						//qDebug() << mCurrentObjectId;
						//qDebug() << mCurrentObjectId.element();
						//qDebug() << mGeMaker->getUniqueName(mCurrentObjectId);

						resultStr += interpret(foreachBlockStream);
						
						foreachBlockStream.seek(0);
					}
				}
				
				mCurrentObjectId = objectId;//TODO: change this method

				return resultStr;
			}
		case forType:
			{
				QString braceBlock = getBraceBlock(stream);
				QTextStream forBlockStream(&braceBlock);

				QString resultStr;

				QString methodName = forStringParse(parsingStr);
				Id objectId = getCurObjId();//TODO: change this method

				mCurrentObjectId = getCurObjectMethodResult(methodName);
				resultStr += interpret(forBlockStream);
				
				mCurrentObjectId = objectId;//TODO: change this method

				return resultStr;
			}

		case leftBraceType:			
			{
				qDebug() << "Error! In" << mTaskFile.fileName() << ". #!{ but not control expression (e.g. #!foreach) found!";
				return "";
			}

		case rightBraceType:
			{
				qDebug() << "Error! In" << mTaskFile.fileName() << ". #!} but not control expression (e.g. #!foreach) found!";
				return "";
			}
		case toFileType:
			{
				//TODO
				QFile file(toFileStringFilename(parsingStr));
				if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
					qDebug() << "cannot open \"" << file.fileName() << "\"";
					return "";
				}

				QString braceBlock = getBraceBlock(stream);
				QTextStream toFileBlockStream(&braceBlock);

				QTextStream out(&file);
				out << interpret(toFileBlockStream);
				file.close();

				return "";
			}
		case saveObjType:
			{
				addLabel(saveObjLabel(parsingStr));
				return "";
			}
		case switchType:
			{
				QString resultStr;
				QString switchPropertyName = switchStringParse(parsingStr);
				
				QString switchProperty;
				if (switchPropertyName == "ELEMENT_TYPE")
					switchProperty = getCurObjId().element();
				else
					switchProperty = getCurObjProperty(switchPropertyName);

				QString braceBlock = getBraceBlock(stream);

				QTextStream switchBlockStream(&braceBlock);
				switchBlockStream.readLine(); //TODO FAIL

				for (QPair<QString, QString> nextCaseBlock = getNextCaseBlock(switchBlockStream);
				    !nextCaseBlock.second.isEmpty(); 
				    nextCaseBlock = getNextCaseBlock(switchBlockStream)) {
					if (nextCaseBlock.first == switchProperty || nextCaseBlock.first == "") { 
					//nextCaseBlock.first == "" - default case
						QTextStream caseBlockStream(&(nextCaseBlock.second));
						resultStr += interpret(caseBlockStream);
						break;
					}
				}

				return resultStr;
			}
		case ifType:
			{
				QString braceBlock = getBraceBlock(stream);
				QTextStream blockStream(&braceBlock);

				if (!ifStringParse(parsingStr)) {
					return "";
				}

				return interpret(blockStream);
			}
		case notControlType:
			{
				return "";
			}
	}

	return "";
}

QString Interpreter::interpret(QTextStream& stream)
{
	QString resultStr;

	while (!stream.atEnd()) {
		QString curStr = stream.readLine();

		if (!isControlString(curStr)) {
			//resultStr += notControlStringParse(curStr) + "\n";
			QString str = notControlStringParse(curStr);
			resultStr += str.isEmpty() ? str : str + "\n";
		}
		else {
			resultStr += controlStringParse(curStr, stream);//может сдвигать stream! Это нужно для for'а
		}
	}

	return resultStr;
}

QString Interpreter::interpret()
{
	if (!mTaskFile.isOpen() && mTaskFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		mInStream = new QTextStream(&mTaskFile);
	}
	
	if (!mTaskFile.isOpen() || mInStream == 0){
		qDebug() << "cannot load file \"" << mTaskFile.fileName() << "\"";
		return "";
	}
	mInStream->seek(0); //for second execute of interpret

	QString const curStr = mInStream->readLine();
	if (!curStr.startsWith("Task ")) {
		qDebug() << "Task file" << mTaskFile.fileName() << "doesn't start with \"Task __name__\"";
		return "";
	}

	QString const taskName = curStr.right(curStr.length() - 5); //5 - "Task " length;

	return interpret(*mInStream);
}

QString Interpreter::getArgument(QString const &str)
{
	int const leftParenthesisPos = str.indexOf('(');
	int const rightParenthesisPos = str.indexOf(')');

	if ( (leftParenthesisPos > -1) && (rightParenthesisPos > leftParenthesisPos) )
		return str.mid(leftParenthesisPos + 1,
				rightParenthesisPos - leftParenthesisPos - 1);

	return QString();
}
