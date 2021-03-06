#include "../funcOrientedGenerator.h"
#include "elementGeneratorFactory.h"

#include <QTextStream>
#include <QDir>
#include <QQueue>
#include <QDebug>

using namespace qReal;
using namespace robots::generator;

FuncOrientedGenerator::FuncOrientedGenerator(qrRepo::RepoControlInterface &api,
					     qReal::ErrorReporterInterface &errorReporter,
					     QString const &destinationPath)
	: NxtOSEKgenerator(api, errorReporter, destinationPath) {
}

FuncOrientedGenerator::FuncOrientedGenerator(QString const &pathToRepo,
					     qReal::ErrorReporterInterface &errorReporter,
					     QString const &destinationPath)
	: NxtOSEKgenerator(pathToRepo, errorReporter, destinationPath) {
}

void FuncOrientedGenerator::writeGeneratedCodeToFile(QString const &resultCode, QString const &initNodeProcedureName,
		int initialNodeNumber) {
	QString const projectName = "example" + QString::number(initialNodeNumber);
	QString const projectDir = "nxt-tools/" + projectName;

	//Create project directory
	if (!QDir(projectDir).exists()) {
		if (!QDir("nxt-tools/").exists()) {
			QDir().mkdir("nxt-tools/");
		}
		QDir().mkdir(projectDir);
	}

	// Generate C file (start)
	QFile templateCFile(":/nxtOSEK/funcOrientedGenerator/templates/template.c");
	if (!templateCFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		mErrorReporter.addError("cannot open \"" + templateCFile.fileName() + "\"");
		return;
	}

	QTextStream templateCStream(&templateCFile);
	QString resultString = templateCStream.readAll();
	templateCFile.close();

	resultString.replace("@@PROJECT_NAME@@", projectName);
	resultString.replace("@@PROCEDURE_CODE@@", resultCode);
	resultString.replace("@@INIT_NODE_PROCEDURE_NAME@@", initNodeProcedureName);

	QFile resultCFile(projectDir + "/" + projectName + ".c");
	if (!resultCFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QString errorMessage = QObject::tr("cannot open \"%1\"");
		errorMessage.replace("%1", resultCFile.fileName());
		mErrorReporter.addError(errorMessage);
		return;
	}

	QTextStream outC(&resultCFile);
	outC << resultString;
	outC.flush();
	resultCFile.close();
	// Generate C file (end)

	// Generate OIL file (start)
	QFile templateOILFile(":/nxtOSEK/funcOrientedGenerator/templates/template.oil");
	if (!templateOILFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QString errorMessage = QObject::tr("cannot open \"%1\"");
		errorMessage.replace("%1", templateOILFile.fileName());
		mErrorReporter.addError(errorMessage);
		return;
	}

	QFile resultOILFile(projectDir + "/" + projectName + ".oil");
	if (!resultOILFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QString errorMessage = QObject::tr("cannot open \"%1\"");
		errorMessage.replace("%1", resultOILFile.fileName());
		mErrorReporter.addError(errorMessage);
		return;
	}

	QTextStream outOIL(&resultOILFile);
	outOIL << templateOILFile.readAll();
	templateOILFile.close();

	outOIL.flush();
	resultOILFile.close();
	// Generate OIL file (end)

	// Generate makefile (start)
	QFile templateMakeFile(":/nxtOSEK/funcOrientedGenerator/templates/template.makefile");
	if (!templateMakeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QString errorMessage = QObject::tr("cannot open \"%1\"");
		errorMessage.replace("%1", templateMakeFile.fileName());
		mErrorReporter.addError(errorMessage);
		return;
	}

	QFile resultMakeFile(projectDir + "/makefile");
	if (!resultMakeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QString errorMessage = QObject::tr("cannot open \"%1\"");
		errorMessage.replace("%1", resultMakeFile.fileName());
		mErrorReporter.addError(errorMessage);
		return;
	}

	QTextStream outMake(&resultMakeFile);
	outMake << templateMakeFile.readAll().replace("@@PROJECT_NAME@@", projectName.toUtf8());
	templateMakeFile.close();

	outMake.flush();
	resultMakeFile.close();
	// Generate makefile (end)
}

qReal::ErrorReporterInterface &FuncOrientedGenerator::generate()
{
	prepareIdToMethodNameMap();
	IdList const initialNodes = mApi->elementsByType("InitialNode");

	int curInitialNodeNumber = 0;
	foreach (Id const &curInitialNode, initialNodes) {
		if (!mApi->isLogicalElement(curInitialNode)) {//? may be necessary to use graphical  elements
			continue;
		}

		mVariables.clear();
		mGeneratedStrings.clear();
		mAlreadyGeneratedElements.clear();

		QQueue<Id> elementsToGenerate;
		elementsToGenerate.enqueue(curInitialNode);

		while (!elementsToGenerate.empty()) {
			Id const curElement = elementsToGenerate.dequeue();
			AbstractElementGenerator* gen = ElementGeneratorFactory::generator(this, curElement);
			if (gen) {
				//changes mVariables, mGeneratedStrings, mAlreadyGeneratedElements
				gen->generate(); 
				delete gen;
			}

			foreach (Id const &nextElement, mApi->outgoingConnectedElements(curElement)) {
				if (!mAlreadyGeneratedElements.contains(nextElement) &&
						!elementsToGenerate.contains(nextElement) && 
						(nextElement.element() != "ROOT_ID")) {
					elementsToGenerate.enqueue(nextElement);
				}
			}

			mAlreadyGeneratedElements.insert(curElement);
		}

		QList<SmartLine> variableGeneratedStrings;
		foreach (SmartLine const &line, mVariables) {
			variableGeneratedStrings.append(SmartLine("int " + line.text() + ";", line.elementId()));
		}

		QString const resultCode = smartLineListToString(variableGeneratedStrings, 0) +
			smartLineListToString(mGeneratedStrings, 0);

		writeGeneratedCodeToFile(resultCode, mIdToMethodNameMap[curInitialNode.toString()],
				curInitialNodeNumber);

		curInitialNodeNumber++;
	}

	return mErrorReporter; 
}

void FuncOrientedGenerator::prepareIdToMethodNameMap()
{
	mIdToMethodNameMap.clear();

	IdList const allNodes = mApi->logicalElements();
	foreach (Id const &curNode, allNodes) {
		AbstractElementGenerator* gen = ElementGeneratorFactory::generator(this, curNode);
		if (gen == 0) {
			continue;
		}

		mIdToMethodNameMap.insert(curNode.toString(), gen->nextMethodName());
		delete gen;
	}
}

QString FuncOrientedGenerator::smartLineListToString(QList<SmartLine> const &list, int startIndentSize) {
	QString result;
	int curIndentSize = startIndentSize;
	foreach (SmartLine const &curLine, list) {
		if ( (curLine.indentLevelChange() == SmartLine::decrease)
				|| (curLine.indentLevelChange() == SmartLine::increaseDecrease) ) {
			curIndentSize--;
		}

		result += QString(curIndentSize, '\t') + curLine.text() + "\n";

		if ( (curLine.indentLevelChange() == SmartLine::increase)
				|| (curLine.indentLevelChange() == SmartLine::increaseDecrease) ) {
			curIndentSize++;
		}
	}

	return result;
}
