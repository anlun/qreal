#pragma once

#include "../abstractElementGenerator.h"

namespace robots {
namespace generator {

//class for simple elements such as Beep, WaitForColor etc
class SimpleElementGenerator: public AbstractElementGenerator {
public:
	SimpleElementGenerator(FuncOrientedGenerator* gen, qReal::Id elementId): AbstractElementGenerator(gen, elementId) {
	}
	
protected:
	virtual void generateMethodBody();
	virtual void generateBodyWithoutNextElementCall() = 0;
};

}
}