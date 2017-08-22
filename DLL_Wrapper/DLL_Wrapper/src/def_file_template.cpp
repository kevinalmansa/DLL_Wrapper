#include "stdafx.h"

/*
fprintf...
%s	-	target dll (ex: user32)
*/
const char	*def_header = R"DEFFILE(; DEF File
LIBRARY		%s
EXPORTS
)DEFFILE";

/* Note: Forward Exports & Normal Exports can be specified in here */