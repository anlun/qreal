Task JavaClass
#!/ <b>Produce Java class from repo</b>

#!foreach Class in elementsByType(Class)
#!{
#!toFile @@name@@.h
#!{
class @@name@@ {
#!foreach MethodsContainer in children
#!{
#!foreach Method in children
#!{
	@@methodVisibility@@ @@methodReturnType@@ @@methodName@@( @@!task MethodParameters@@ ) {
	}
#!}
#!}

#!foreach FieldsContainer in children
#!{
#!foreach Field in children
#!{
	@@fieldVisibility@@ @@fieldType@@ @@fieldName@@;
#!}
#!}
}
#!}
#!}
