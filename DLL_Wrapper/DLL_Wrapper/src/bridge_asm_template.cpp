#include "stdafx.h"

/*
	fprintf...
	%s	-	function name
	%u	-	index of function to call
	%s	-	function name
	%s	-	function name
	%s	-	function name
	%s	-	function name
	%s	-	function name
	%s	-	function name
*/
const char	*asm_bridge = R"ASMBRIDGE(
;; %s
function_index equ	 %u			 ;; index of function to call

IFDEF RAX
;; 64 bit assembly

;; EXTERNs here
;; EXTERN MessageBoxA: PROC
EXTERN c_ext_intercepts: QWORD	 ;; array of function pointers.

.DATA
.CODE

PUBLIC %s
%s PROC

	;; jmp to value at index value of r10 * sizeof(qword)
	LEA	RAX, c_ext_intercepts
	MOV RAX, [RAX + function_index * 8]
	jmp	QWORD PTR RAX

%s ENDP

ELSE
;; 32 bit assembly

.586
.MODEL FLAT, C
.STACK
.DATA
.CODE

;; EXTERNs here
EXTERN c_ext_intercepts: DWORD	;; array of function pointers

PUBLIC %s
%s PROC

	LEA		EAX, c_ext_intercepts
	MOV		EAX, [EAX + function_index * 4]
	jmp		DWORD PTR EAX

%s ENDP

ENDIF
END

)ASMBRIDGE";
