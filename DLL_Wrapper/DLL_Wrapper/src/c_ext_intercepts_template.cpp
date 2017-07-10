#include "stdafx.h"

/*
	fprintf...
	%u	-	call_table_size
*/
const char		*c_ext_intercepts_global = R"INTERCEPTSGLOBAL(
/*
** Global array of function pointers used by bridge assembly function
** to JMP into target DLL and execute function normally after intercepting.
*/
#include "stdafx.h"
#include "intercepts.h"

void		*(c_ext_intercepts[%u]) = {0};

)INTERCEPTSGLOBAL";
