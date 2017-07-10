#include "stdafx.h"

/*
	fprintf...
	%s	-	return type
	%s	-	function name
	%s	-	paramaters type + name
	%s	-	target dll -> user33.dll
	%s	-	function name
	%s	-	function name
	%s	-	function name
	%u	-	function index
	%s	-	bridge_name
	%s	-	paramaters names
*/

const char	*intercepted_function = R"CPPINTERCEPTEDF(
#include "stdafx.h"
#include "intercepts.h"

extern "C" %s	%s(%s)
{
	HINSTANCE			handle;
	FARPROC				intercepted_function;

	handle = LoadLibraryA("%s");
	if (handle == NULL)
	{
		printf("Error: Couldn't load Target DLL\n");
		return -1;
	}

	intercepted_function = GetProcAddress(handle, "%s");
	if (intercepted_function == NULL)
	{
		printf("Error: Couldn't load function Target %s\n");
		return -1;
	}

	// ARBITRARY CODE GOES HERE
	// CAN LOG PARAMATERS, EXECUTE ARBITRARY CODE, ETC
	printf("%s Called. The function has been intercepted\n");

	// CALL INTERCEPTED FUNCTION
	// Note: This could also be done by casting intercepted_function to
	// the appropriate function pointer. Linking to an assembly routine
	// on the other hand gives a lower level control.
	c_ext_intercepts[%u] = intercepted_function;
	%s(%s);

	return 0;
}

)CPPINTERCEPTEDF";