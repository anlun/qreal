#include <QTextStream>
#include <cmath>
#include <QtCore/QObject>
#include <QDir>

#include "../../../../qrkernel/exception/exception.h"
#include "../sequentialGenerator.h"

#include <QDebug>

using namespace qReal;
using namespace robots::generator;

SequentialGenerator::SequentialGenerator(qrRepo::RepoControlInterface &api,
					 qReal::ErrorReporterInterface &errorReporter,
					 QString const &destinationPath)
	: NxtOSEKgenerator(api, errorReporter, destinationPath)
{
}

SequentialGenerator::SequentialGenerator(QString const &pathToRepo,
					 qReal::ErrorReporterInterface &errorReporter,
					 QString const &destinationPath)
	: NxtOSEKgenerator(pathToRepo, errorReporter, destinationPath)
{
}

void SequentialGenerator::addToGeneratedStringSetVariableInit() {
	foreach (SmartLine const &curVariable, mVariables) {
		mGeneratedStringSet[mVariablePlaceInGenStrSet].append(
				SmartLine("int " + curVariable.text() + ";", curVariable.elementId()));
	}
}

qReal::ErrorReporterInterface &SequentialGenerator::generate()
{
	IdList const initialNodes = mApi->elementsByType("InitialNode");

	int curInitialNodeNumber = 0;
	foreach (Id const &curInitialNode, initialNodes) {
		if (!mApi->isGraphicalElement(curInitialNode)) {
			continue;
		}

		QString resultCode;
		mGeneratedStringSet.clear();
		mGeneratedStringSet.append(QList<SmartLine>()); //first list for variable initialization
		mVariablePlaceInGenStrSet = 0;

		mElementToStringListNumbers.clear();
		mVariables.clear();

		AbstractElementGenerator* gen = ElementGeneratorFactory::generator(this, curInitialNode);
		mPreviousElement = curInitialNode;

		gen->generate(); //may throws a exception

		addToGeneratedStringSetVariableInit();

		int curTabNumber = 1;
		foreach (QList<SmartLine> const &lineList, mGeneratedStringSet) {
			foreach (SmartLine const &curLine, lineList) {
				if ( (curLine.indentLevelChange() == SmartLine::decrease)
						|| (curLine.indentLevelChange() == SmartLine::increaseDecrease) ) {
					curTabNumber--;
				}

				resultCode += QString(curTabNumber, '\t') + curLine.text() + "\n";

				if ( (curLine.indentLevelChange() == SmartLine::increase)
						|| (curLine.indentLevelChange() == SmartLine::increaseDecrease) ) {
					curTabNumber++;
				}
			}
		}
		delete gen;

		//QDir projectsDir; //TODO: use user path to projects

		QString projectName = "example" + QString::number(curInitialNodeNumber);
		QString projectDir = "nxt-tools/" + projectName;

		//Create project directory
		if (!QDir(projectDir).exists()) {
			if (!QDir("nxt-tools/").exists()) {
				QDir().mkdir("nxt-tools/");
			}
			QDir().mkdir(projectDir);
		}

		// Generate C file (start)
		QFile templateCFile(":/nxtOSEK/sequentialGenerator/templates/template.c");
		if (!templateCFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QString errorMessage(QObject::tr("cannot open \"%1\""));
			errorMessage.replace("%1", templateCFile.fileName());
			mErrorReporter.addError(errorMessage);
			return mErrorReporter;
		}

		QTextStream templateCStream(&templateCFile);
		QString resultString = templateCStream.readAll();
		templateCFile.close();

		resultString.replace("@@PROJECT_NAME@@", projectName);
		resultString.replace("@@CODE@@", resultCode);

		QFile resultCFile(projectDir + "/" + projectName + ".c");
		if (!resultCFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QString errorMessage(QObject::tr("cannot open \"%1\""));
			errorMessage.replace("%1", resultCFile.fileName());
			mErrorReporter.addError(errorMessage);

			return mErrorReporter;
		}

		QTextStream outC(&resultCFile);
		outC << resultString;
		outC.flush();
		resultCFile.close();
		// Generate C file (end)

		// Generate OIL file (start)
		QFile templateOILFile(":/nxtOSEK/sequentialGenerator/templates/template.oil");
		if (!templateOILFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QString errorMessage(QObject::tr("cannot open \"%1\""));
			errorMessage.replace("%1", templateOILFile.fileName());
			mErrorReporter.addError(errorMessage);
			return mErrorReporter;
		}

		QFile resultOILFile(projectDir + "/" + projectName + ".oil");
		if (!resultOILFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QString errorMessage(QObject::tr("cannot open \"%1\""));
			errorMessage.replace("%1", resultOILFile.fileName());
			mErrorReporter.addError(errorMessage);
			return mErrorReporter;
		}

		QTextStream outOIL(&resultOILFile);
		outOIL << templateOILFile.readAll();
		templateOILFile.close();

		outOIL.flush();
		resultOILFile.close();
		// Generate OIL file (end)

		// Generate makefile (start)
		QFile templateMakeFile(":/nxtOSEK/sequentialGenerator/templates/template.makefile");
		if (!templateMakeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QString errorMessage(QObject::tr("cannot open \"%1\""));
			errorMessage.replace("%1", templateMakeFile.fileName());
			mErrorReporter.addError(errorMessage);
			return mErrorReporter;
		}

		QFile resultMakeFile(projectDir + "/makefile");
		if (!resultMakeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QString errorMessage(QObject::tr("cannot open \"%1\""));
			errorMessage.replace("%1", resultMakeFile.fileName());
			mErrorReporter.addError(errorMessage);

			return mErrorReporter;
		}

		QTextStream outMake(&resultMakeFile);
		outMake << templateMakeFile.readAll().replace("@@PROJECT_NAME@@", projectName.toUtf8());
		templateMakeFile.close();

		outMake.flush();
		resultMakeFile.close();
		// Generate makefile (end)

		curInitialNodeNumber++;
	}

	if (initialNodes.isEmpty()) {
		mErrorReporter.addError(QObject::tr("There is nothing to generate, diagram doesn't have Initial Node"));
	}

	return mErrorReporter;
}

SequentialGenerator::AbstractElementGenerator::AbstractElementGenerator(SequentialGenerator *emboxGen,
		qReal::Id const &elementId): mNxtGen(emboxGen), mElementId(elementId)
{
}

void SequentialGenerator::AbstractElementGenerator::createListsForIncomingConnections()
{
	//connects string lists in mGeneratedStringSet with mElementId in mElementToStringListNumbers
	//starts at 1 because must not be created list for current first appearance
	for (int i = 1; i < mNxtGen->mApi->incomingConnectedElements(mElementId).size(); i++) {
		mNxtGen->mGeneratedStringSet << QList<SmartLine>();
		mNxtGen->mElementToStringListNumbers[mElementId.toString()] << mNxtGen->mGeneratedStringSet.size() - 1;
	}
}

SequentialGenerator::SimpleElementGenerator::SimpleElementGenerator(SequentialGenerator *emboxGen,
		qReal::Id elementId): AbstractElementGenerator(emboxGen, elementId)
{
}

SequentialGenerator::FunctionElementGenerator::FunctionElementGenerator(SequentialGenerator *emboxGen,
		qReal::Id elementId): SimpleElementGenerator(emboxGen, elementId)
{
}

SequentialGenerator::LoopElementGenerator::LoopElementGenerator(SequentialGenerator *emboxGen,
		qReal::Id elementId): AbstractElementGenerator(emboxGen, elementId)
{
}

SequentialGenerator::IfElementGenerator::IfElementGenerator(SequentialGenerator *emboxGen,
		qReal::Id elementId): AbstractElementGenerator(emboxGen, elementId)
{
}

QList<QString> SequentialGenerator::SimpleElementGenerator::portsToEngineNames(QString const &portsProperty)
{
	QList<QString> result;

	//port {A, B, C} -> NXT_PORT_{A, B, C}
	if (portsProperty.contains("A")) {
		result.append("NXT_PORT_A");
	} else if (portsProperty.contains("B")) {
		result.append("NXT_PORT_B");
	} else if (portsProperty.contains("C")) {
		result.append("NXT_PORT_C");
	}

	return result;
}

void SequentialGenerator::FunctionElementGenerator::variableAnalysis(QByteArray const &code)
{
	QList<QByteArray> const funcBlocks = code.split(';');

	foreach (QByteArray const &block, funcBlocks) {
			//Only one possible place for first variable appear
		int const firstEqualSignPos = block.indexOf('=');
		if (firstEqualSignPos == -1) {
			continue;
		}

		//must be a normal variable name
		QByteArray leftPart = block.left(firstEqualSignPos);

		leftPart = leftPart.trimmed();
		QString forbiddenLastSimbols = "+-=*/><";
		if (forbiddenLastSimbols.contains((leftPart.at(leftPart.length() - 1)))) {
			continue;
		}
		bool isVariableExisted = false;
		foreach (SmartLine const &curVariable, mNxtGen->mVariables) {
			if (curVariable.text() == QString::fromUtf8(leftPart)) {
				isVariableExisted = true;
				break;
			}
		}
		if (!isVariableExisted) {
			mNxtGen->mVariables.append(SmartLine(QString::fromUtf8(leftPart), mElementId));
		}
	}
}

QList<SmartLine> SequentialGenerator::FunctionElementGenerator::simpleCode()
{
	QList<SmartLine> result;

	qReal::Id const logicElementId = mNxtGen->mApi->logicalId(mElementId); //TODO

	QByteArray byteFuncCode = mNxtGen->mApi->stringProperty(logicElementId, "Body").toUtf8();

	byteFuncCode.replace("Сенсор1", "ecrobot_get_sonar_sensor(NXT_PORT_S1)");
	byteFuncCode.replace("Сенсор2", "ecrobot_get_sonar_sensor(NXT_PORT_S2)");
	byteFuncCode.replace("Сенсор3", "ecrobot_get_sonar_sensor(NXT_PORT_S3)");
	byteFuncCode.replace("Сенсор4", "ecrobot_get_sonar_sensor(NXT_PORT_S4)");

	variableAnalysis(byteFuncCode);

	QString funcCode = QString::fromUtf8(byteFuncCode);
	foreach (QString const &str, funcCode.split(';')) {
		result.append(SmartLine(str.trimmed() + ";", mElementId));
	}

	return result;
}

QList<SmartLine> SequentialGenerator::SimpleElementGenerator::simpleCode()
{
	QList<SmartLine> result;

	qReal::Id const logicElementId = mNxtGen->mApi->logicalId(mElementId); //TODO

	//TODO: to make "break mode" to do smth
	if (mElementId.element() == "EnginesForward") {
		QStringList const cmds = mNxtGen->mApi->stringProperty(logicElementId, "Power").split(";", QString::SkipEmptyParts);
		for (int i = 0; i < cmds.size() - 1; ++i)
			result.append(SmartLine(cmds.at(i) + ";", mElementId));
		foreach (QString const &enginePort, portsToEngineNames(mNxtGen->mApi->stringProperty(logicElementId, "Ports"))) {
			result.append(SmartLine(
						"nxt_motor_set_speed(" + enginePort + ", " + cmds.last() + ", 0);",
						mElementId));
		}

	} else if (mElementId.element() == "EnginesBackward") {
		foreach (QString const &enginePort, portsToEngineNames(mNxtGen->mApi->stringProperty(logicElementId, "Ports"))) {
			result.append(SmartLine(
						"nxt_motor_set_speed(" + enginePort + ", "
							+ "-" + mNxtGen->mApi->stringProperty(logicElementId, "Power") + ", 0);",
						mElementId));
		}

	} else if (mElementId.element() == "EnginesStop") {
		foreach (QString const &enginePort, portsToEngineNames(mNxtGen->mApi->stringProperty(logicElementId, "Ports"))) {
			result.append(SmartLine(
						"nxt_motor_set_speed(" + enginePort + ", 0, 0);",
						mElementId));
		}

	} else if (mElementId.element() == "Timer") {
		result.append(SmartLine(
					"systick_wait_ms(" + mNxtGen->mApi->stringProperty(logicElementId, "Delay") + ");",
					mElementId));

	} else if (mElementId.element() == "Beep") {
		result.append(SmartLine(
					"ecrobot_sound_tone(1000, 100, 50);", //TODO: change sound to smth
					mElementId));

	} else if (mElementId.element() == "PlayTone") {
		result.append(SmartLine(
					"ecrobot_sound_tone(" + mNxtGen->mApi->stringProperty(logicElementId, "Frequency") + ", "
						+ mNxtGen->mApi->stringProperty(logicElementId, "Duration") + ", 50);", //50 - volume of a sound
					mElementId));

	} else if (mElementId.element() == "FinalNode") {
		result.append(SmartLine(
					"return;",
					mElementId));

	} else if (mElementId.element() == "NullificationEncoder") {
		QString port = mNxtGen->mApi->stringProperty(logicElementId, "Port");
		result.append(SmartLine(
					"nxt_motor_set_count(NXT_PORT_" + port + ", 0);",
					mElementId));

	} else if (mElementId.element() == "InitialBlock") {
		for (int i = 1; i <= 4; i++) {
			//4 - number of ports on nxt robot
			QString const curPort = "port_" + QString::number(i);
			QByteArray const portValue = mNxtGen->mApi->stringProperty(logicElementId, curPort).toUtf8();

			if (portValue == "Ультразвуковой сенсор") {
				result.append(SmartLine(
						"ecrobot_init_sonar_sensor(NXT_PORT_S" + QString::number(i) + ")",
						mElementId));

			//in nxtOSEK there are no instructions to initiate touch sensors
			//} else if (portValue == "Сенсор нажатия (булево значение)") {
			//} else if (portValue == "Сенсор нажатия (сырое значение)") {

			} else if (portValue == "Сенсор цвета (полные цвета)") {
				result.append(SmartLine(
						"ecrobot_init_nxtcolorsensor(NXT_PORT_S" + QString::number(i) + ", NXT_LIGHTSENSOR_WHITE)",
						mElementId));

			} else if (portValue == "Сенсор цвета (красный)") {
				result.append(SmartLine(
						"ecrobot_init_nxtcolorsensor(NXT_PORT_S" + QString::number(i) + ", NXT_LIGHTSENSOR_RED)",
						mElementId));

			} else if (portValue == "Сенсор цвета (зеленый)") {
				result.append(SmartLine(
						"ecrobot_init_nxtcolorsensor(NXT_PORT_S" + QString::number(i) + ", NXT_LIGHTSENSOR_GREEN)",
						mElementId));

			} else if (portValue == "Сенсор цвета (синий)") {
				result.append(SmartLine(
						"ecrobot_init_nxtcolorsensor(NXT_PORT_S" + QString::number(i) + ", NXT_LIGHTSENSOR_BLUE)",
						mElementId));

			} else if (portValue == "Сенсор цвета (пассивный)") {
				result.append(SmartLine(
						"ecrobot_init_nxtcolorsensor(NXT_PORT_S" + QString::number(i) + ", NXT_COLORSENSOR)",
						mElementId));

			}
		}

	} else if (mElementId.element() == "WaitForColor") {
		int const port = mNxtGen->mApi->stringProperty(logicElementId, "Port").toInt();
		QByteArray const colorStr = mNxtGen->mApi->stringProperty(logicElementId, "Color").toUtf8();

		QString colorNxtType;

		if (colorStr == "Красный") {
			colorNxtType = "NXT_COLOR_RED";

		} else if (colorStr == "Зелёный") {
			colorNxtType = "NXT_COLOR_GREEN";

		} else if (colorStr == "Синий") {
			colorNxtType = "NXT_COLOR_BLUE";

		} else if (colorStr == "Чёрный") {
			colorNxtType = "NXT_COLOR_BLACK";

		} else if (colorStr == "Жёлтый") {
			colorNxtType = "NXT_COLOR_YELLOW";

		} else if (colorStr == "Белый") {
			colorNxtType = "NXT_COLOR_WHITE";
		}

		if (!colorNxtType.isEmpty()) {
			result.append(SmartLine(
					"while (ecrobot_get_nxtcolorsensor_id(NXT_PORT_S" + QString::number(port)
						+ ") != " + colorNxtType + ")",
					mElementId));
			result.append(SmartLine("{", mElementId));
			result.append(SmartLine("}", mElementId));
		}

	} else if (mElementId.element() == "WaitForColorIntensity") {
		int const port = mNxtGen->mApi->stringProperty(logicElementId, "Port").toInt();
		QString const intensity = mNxtGen->mApi->stringProperty(logicElementId,  "Intensity");
		QString const inequalitySign = mNxtGen->mApi->stringProperty(logicElementId, "Sign");

		QString const condition = inequalitySign + " " + intensity;

		result.append(SmartLine(
				"while (!(ecrobot_get_nxtcolorsensor_light(NXT_PORT_S" + QString::number(port)
					+ ") " + condition + "))",
				mElementId));
		result.append(SmartLine("{", mElementId));
		result.append(SmartLine("}", mElementId));

	} else if (mElementId.element() == "WaitForTouchSensor") {
		int const port = mNxtGen->mApi->stringProperty(logicElementId, "Port").toInt();

		result.append(SmartLine(
				"while (!ecrobot_get_touch_sensor(NXT_PORT_S" + QString::number(port) + "))",
				mElementId));
		result.append(SmartLine("{", mElementId));
		result.append(SmartLine("}", mElementId));

	} else if (mElementId.element() == "WaitForSonarDistance") {
		int const port = mNxtGen->mApi->stringProperty(logicElementId, "Port").toInt();
		QString const distance = mNxtGen->mApi->stringProperty(logicElementId, "Distance");
		QString const inequalitySign = transformSign(QString(mNxtGen->mApi->stringProperty(logicElementId, "Sign").toUtf8()));
		QString const condition = inequalitySign + " " + distance;

		result.append(SmartLine(
				"while (!(ecrobot_get_sonar_sensor(NXT_PORT_S" + QString::number(port) + ") " + condition + "))",
				mElementId));
		result.append(SmartLine("{", mElementId));
		result.append(SmartLine("}", mElementId));

	} else if (mElementId.element() == "WaitForEncoder") {
		QString const port = mNxtGen->mApi->stringProperty(logicElementId, "Port");
		QString const tachoLimit = mNxtGen->mApi->stringProperty(logicElementId, "TachoLimit");
		result.append(SmartLine(
				"while (nxt_motor_get_count(NXT_PORT_" + port + ") < " + tachoLimit + ")",
				mElementId));
		result.append(SmartLine("{", mElementId));
		result.append(SmartLine("}", mElementId));

	}

	//for InitialNode returns empty list

	return result;
}

QString SequentialGenerator::SimpleElementGenerator::transformSign(QString const &sign)
{
	if (sign == "меньше") {
		return "<";
	} else if (sign == "больше") {
		return ">";
	} else if (sign == "не меньше") {
		return ">=";
	} else if (sign == "не больше") {
		return "<=";
	} else if (sign == "равно") {
		return "==";
	}
	return "";
}

QList<SmartLine> SequentialGenerator::SimpleElementGenerator::loopPrefixCode() const
{
	QList<SmartLine> result;
	result << SmartLine("while (true) {", mElementId, SmartLine::increase);
	return result;
}

QList<SmartLine> SequentialGenerator::SimpleElementGenerator::loopPostfixCode() const
{
	QList<SmartLine> result;
	result << SmartLine("}", mElementId, SmartLine::decrease);
	return result;
}

QList<SmartLine> SequentialGenerator::LoopElementGenerator::loopPrefixCode() const
{
	QList<SmartLine> result;

	qReal::Id const logicElementId = mNxtGen->mApi->logicalId(mElementId); //TODO
	result << SmartLine("for (int __iter__ = ; __iter__ < " +
			mNxtGen->mApi->property(logicElementId, "Iterations").toString()
				+ "; __iter__++) {", mElementId, SmartLine::increase); //TODO
	return result;
}

QList<SmartLine> SequentialGenerator::LoopElementGenerator::loopPostfixCode() const
{
	QList<SmartLine> result;
	result << SmartLine("}", mElementId, SmartLine::decrease);
	return result;
}

QList<SmartLine> SequentialGenerator::IfElementGenerator::loopPrefixCode() const
{
	QList<SmartLine> result;

	qReal::Id const logicElementId = mNxtGen->mApi->logicalId(mElementId); //TODO
	result << SmartLine("while (" +
			mNxtGen->mApi->property(logicElementId, "Condition").toString()
				+ ") {", mElementId, SmartLine::increase); //TODO
	return result;
}

QList<SmartLine> SequentialGenerator::IfElementGenerator::loopPostfixCode() const
{
	QList<SmartLine> result;
	result << SmartLine("}", mElementId, SmartLine::decrease);
	return result;
}

bool SequentialGenerator::SimpleElementGenerator::preGenerationCheck()
{
	IdList const outgoingConnectedElements = mNxtGen->mApi->outgoingConnectedElements(mElementId);
	if (outgoingConnectedElements.size() > 1) {
		//case of error in diagram
		mNxtGen->mErrorReporter.addError(
				QObject::tr("Error! There are more than 1 outgoing connected elements with simple robot element!")
				);
		return false;
	}

	return true;
}

bool SequentialGenerator::LoopElementGenerator::preGenerationCheck()
{
	IdList const outgoingLinks = mNxtGen->mApi->outgoingLinks(mElementId);

	if ((outgoingLinks.size() != 2) ||
		( (mNxtGen->mApi->property(mNxtGen->mApi->logicalId(outgoingLinks.at(0)), "Guard").toString() == "Итерация")
		  &&
		  (mNxtGen->mApi->property(mNxtGen->mApi->logicalId(outgoingLinks.at(1)), "Guard").toString() == "Итерация") )
	)
		return false;

	return true;
}

bool SequentialGenerator::IfElementGenerator::preGenerationCheck()
{
	IdList const outgoingLinks = mNxtGen->mApi->outgoingLinks(mElementId);

	//TODO: append checking arrows
	return (outgoingLinks.size() == 2);
}

bool SequentialGenerator::SimpleElementGenerator::nextElementsGeneration()
{
	IdList const outgoingConnectedElements = mNxtGen->mApi->outgoingConnectedElements(mElementId);
	mNxtGen->mGeneratedStringSet << simpleCode();

	if (outgoingConnectedElements.size() == 1) {
		if (outgoingConnectedElements.at(0) == Id::rootId()) {
			QString errorMessage = 	QObject::tr("Element %1 has no"\
						" correct next element because its link has no end object."\
						" May be you need to connect it to diagram object.");
			errorMessage.replace("%1", mElementId.toString());

			mNxtGen->mErrorReporter.addError(
					errorMessage
					, mElementId);
			return false;
		}

		AbstractElementGenerator* gen = ElementGeneratorFactory::generator(mNxtGen, outgoingConnectedElements.at(0));
		mNxtGen->mPreviousElement = mElementId;
		gen->generate();
		delete gen;
		return true;
	} else if ((mElementId.element() == "FinalNode") && (outgoingConnectedElements.size() == 0)) {
		return true;
	} else {
		//case of error end of diagram
		mNxtGen->mErrorReporter.addError(
				QObject::tr("Error! There is no outgoing connected elements with no final node!")
				);
		return false;
	}

	return true;
}

bool SequentialGenerator::LoopElementGenerator::nextElementsGeneration()
{
	IdList const outgoingLinks = mNxtGen->mApi->outgoingLinks(mElementId);
	Q_ASSERT(outgoingLinks.size() == 2);

	int elementConnectedByIterationEdgeNumber = -1;
	int afterLoopElementNumber = -1;

	if (mNxtGen->mApi->stringProperty(mNxtGen->mApi->logicalId(outgoingLinks.at(0)), "Guard").toUtf8() == "итерация") {
		elementConnectedByIterationEdgeNumber = 0;
		afterLoopElementNumber = 1;
	} else {
		elementConnectedByIterationEdgeNumber = 1;
		afterLoopElementNumber = 0;
	}

	//generate loop
	Id const loopNextElement = mNxtGen->mApi->to(outgoingLinks.at(elementConnectedByIterationEdgeNumber));
	if (loopNextElement == Id::rootId()) {
		QString errorMessage = QObject::tr("Loop block %1 has no correct loop branch!"\
					" May be you need to connect it to some diagram element.");
		errorMessage.replace("%1", mElementId.toString());
		mNxtGen->mErrorReporter.addError(
				errorMessage
				, mElementId);
		return false;
	}

	AbstractElementGenerator* loopGen = ElementGeneratorFactory::generator(mNxtGen,
			loopNextElement);

	mNxtGen->mPreviousElement = mElementId;
	mNxtGen->mPreviousLoopElements.push(mElementId);
	if (!loopGen->generate()) {
		return false;
	}
	delete loopGen;

	//generate next blocks
	Id const nextBlockElement = mNxtGen->mApi->to(outgoingLinks.at(afterLoopElementNumber));
	if (nextBlockElement == Id::rootId()) {
		QString errorMessage = QObject::tr("Loop block %1 has no correct next block branch!"\
					" May be you need to connect it to some diagram element.");
		errorMessage.replace("%1", mElementId.toString());
		mNxtGen->mErrorReporter.addError(
				errorMessage
				, mElementId);
		return false;
	}

	AbstractElementGenerator* nextBlocksGen = ElementGeneratorFactory::generator(mNxtGen,
			nextBlockElement);

	mNxtGen->mPreviousElement = mElementId;
	mNxtGen->mPreviousLoopElements.push(mElementId);
	if (!nextBlocksGen->generate()) {
		return false;
	}
	delete nextBlocksGen;

	return true;
}

bool SequentialGenerator::IfElementGenerator::generateBranch(int branchNumber)
{
	IdList const outgoingLinks = mNxtGen->mApi->outgoingLinks(mElementId);

	Id const branchElement = mNxtGen->mApi->to(outgoingLinks.at(branchNumber));
	if (branchElement == Id::rootId()) {
		QString errorMessage = QObject::tr("If block %1 has no 2 correct branches!"\
					" May be you need to connect one of them to some diagram element.");
		mNxtGen->mErrorReporter.addError(
				errorMessage
				, mElementId);
		return false;
	}

	AbstractElementGenerator* nextBlocksGen = ElementGeneratorFactory::generator(mNxtGen,
			branchElement);

	mNxtGen->mPreviousElement = mElementId;

	if (!nextBlocksGen->generate()) {
		return false;
	}
	delete nextBlocksGen;

	return true;
}

QPair<bool, qReal::Id> SequentialGenerator::IfElementGenerator::checkBranchForBackArrows(qReal::Id const &curElementId) {
	//initial step of checking
	IdList emptyList;
	return checkBranchForBackArrows(curElementId, &emptyList);
}

QPair<bool, qReal::Id> SequentialGenerator::IfElementGenerator::checkBranchForBackArrows(qReal::Id const &curElementId,
		qReal::IdList* checkedElements) {
	qReal::Id logicElementId = curElementId;
	if (!mNxtGen->mApi->isLogicalElement(curElementId)) {
		logicElementId = mNxtGen->mApi->logicalId(curElementId);
	}

	if (checkedElements->contains(logicElementId)) {
		//if we have already observed this element by checkBranchForBackArrows function
		return QPair<bool, qReal::Id>(false, qReal::Id());
	}

	//if we have observed this element and generated code of this element
	foreach (QString const &observedElementString, mNxtGen->mElementToStringListNumbers.keys()) {
		qReal::Id const observedElementId = qReal::Id::loadFromString(observedElementString);
		qReal::Id const observedElementLogicId = mNxtGen->mApi->logicalId(observedElementId);

		if ((logicElementId == observedElementId)
				|| (logicElementId == observedElementLogicId)) {
			return QPair<bool, qReal::Id>(true, logicElementId);
		}
	}

	//add element to list
	checkedElements->append(logicElementId);

	foreach (qReal::Id const &childId, mNxtGen->mApi->outgoingConnectedElements(logicElementId)) {
		if (childId == Id::rootId()) {
			QString errorMessage = QObject::tr("Link from %1"\
						" has no object on its end."\
						" May be you need to connect it to diagram object.");
			errorMessage.replace("%1", logicElementId.toString());
			mNxtGen->mErrorReporter.addError(
					errorMessage
					, mElementId);
			return QPair<bool, qReal::Id>(false, qReal::Id());
		}

		QPair<bool, qReal::Id> childResult = checkBranchForBackArrows(childId, checkedElements);
		if (childResult.first) {
			return childResult;
		}
	}

	//release element to list
	checkedElements->removeAll(logicElementId);

	return QPair<bool, qReal::Id>(false, qReal::Id());
}

bool SequentialGenerator::IfElementGenerator::nextElementsGeneration()
{
	IdList const outgoingLinks = mNxtGen->mApi->outgoingLinks(mElementId);
	Q_ASSERT(outgoingLinks.size() == 2);

	//we search for arrow with condition
	int const conditionArrowNum =
			mNxtGen->mApi->property(mNxtGen->mApi->logicalId(outgoingLinks.at(0)), "Guard").toString().isEmpty()
			? 1 : 0;

	qReal::Id const logicElementId = mNxtGen->mApi->logicalId(mElementId); //TODO

	//TODO: save number of new created list
	QString condition = "(" + mNxtGen->mApi->property(logicElementId, "Condition").toString() + ")";

	QByteArray const conditionOnArrow =
		mNxtGen->mApi->stringProperty(mNxtGen->mApi->logicalId(outgoingLinks.at(conditionArrowNum)), "Guard").toUtf8();
	if (conditionOnArrow == "меньше 0") {
		condition += " < 0";
	} else if (conditionOnArrow == "больше 0") {
		condition += " > 0";
	} else {
		condition += " == 0";
	}

	//check for back arrows
	Id const positiveBranchElement = mNxtGen->mApi->to(mNxtGen->mApi->logicalId(outgoingLinks.at(conditionArrowNum)));
	if (positiveBranchElement == Id::rootId()) {
		QString errorMessage = QObject::tr("If block %1 has no 2 correct branches!"\
					" May be you need to connect one of them to some diagram element.");
		errorMessage.replace("%1", mElementId.toString());
		mNxtGen->mErrorReporter.addError(
				errorMessage
				, mElementId);
		return false;
	}

	QPair<bool, qReal::Id> const positiveBranchCheck = checkBranchForBackArrows(positiveBranchElement);
	bool const isPositiveBranchReturnsToBackElems = positiveBranchCheck.first;

	Id const negativeBranchElement = mNxtGen->mApi->to(outgoingLinks.at(1 - conditionArrowNum));
	if (negativeBranchElement == Id::rootId()) {
		QString errorMessage = QObject::tr("If block %1 has no 2 correct branches!"\
					" May be you need to connect one of them to some diagram element.");
		errorMessage.replace("%1", mElementId.toString());

		mNxtGen->mErrorReporter.addError(
				errorMessage
				, mElementId);
		return false;
	}

	QPair<bool, qReal::Id> const negativeBranchCheck = checkBranchForBackArrows(negativeBranchElement);

	bool const isNegativeBranchReturnsToBackElems = negativeBranchCheck.first;

	if (isPositiveBranchReturnsToBackElems && isNegativeBranchReturnsToBackElems) {
		if (positiveBranchCheck.second != negativeBranchCheck.second) {
			mNxtGen->mErrorReporter.addError(
					QObject::tr("This diagram isn't structed diagram,"\
						" because there are IF block with 2 back arrows!")
					, mElementId);
			return false;
		}

		//TODO: repair for case with merged branches
		return false;
	}

	if (isPositiveBranchReturnsToBackElems != isNegativeBranchReturnsToBackElems) {
		int const cycleBlock = isPositiveBranchReturnsToBackElems ? conditionArrowNum : 1 - conditionArrowNum;
		if (conditionArrowNum == cycleBlock) {
			condition = "!" + condition;
		}

		QList<SmartLine> ifBlock;
		ifBlock << SmartLine("if (" + condition + ") {", mElementId, SmartLine::increase);
		ifBlock << SmartLine("break;", mElementId, SmartLine::withoutChange);
		ifBlock << SmartLine("}", mElementId, SmartLine::decrease);
		mNxtGen->mGeneratedStringSet << ifBlock;
		generateBranch(cycleBlock);

		generateBranch(1 - cycleBlock);
		return true;
	}

	if (!isPositiveBranchReturnsToBackElems && !isNegativeBranchReturnsToBackElems) {
		QList<SmartLine> ifBlockPrefix;
		ifBlockPrefix << SmartLine("if (" + condition + ") {", mElementId, SmartLine::increase);
		mNxtGen->mGeneratedStringSet << ifBlockPrefix;

		//generate true/false blocks
		generateBranch(conditionArrowNum);
		QList<SmartLine> elseBlock;
		elseBlock << SmartLine("} else {", mElementId, SmartLine::increaseDecrease);
		mNxtGen->mGeneratedStringSet << elseBlock;
		generateBranch(1 - conditionArrowNum);

		QList<SmartLine> ifBlockPostfix;
		ifBlockPostfix << SmartLine("}", mElementId, SmartLine::decrease);
		mNxtGen->mGeneratedStringSet << ifBlockPostfix;

		return true;
	}

	return true;
}

bool SequentialGenerator::AbstractElementGenerator::generate()
{
	if (!preGenerationCheck()) {
		return false;
	}

	if (mNxtGen->mElementToStringListNumbers.contains(mElementId.toString())) {
		//if we have already observed this element with more than 1 incoming connection

		qReal::Id loopElement = mElementId;
		if (!mNxtGen->mPreviousLoopElements.empty()) {
			loopElement = mNxtGen->mPreviousLoopElements.pop();
		}

		//loopElement must create loop code
		AbstractElementGenerator *loopElementGen = ElementGeneratorFactory::generator(mNxtGen, loopElement);

		mNxtGen->mGeneratedStringSet[mNxtGen->mElementToStringListNumbers[loopElement.toString()].pop()]
			+= loopElementGen->loopPrefixCode();

		mNxtGen->mGeneratedStringSet << loopElementGen->loopPostfixCode();

		return true;
	}

	//in case element has more than 1 incoming connection
	//means that element has incoming connections from another elements, we haven`t already observed
	createListsForIncomingConnections();

	return nextElementsGeneration();
}
