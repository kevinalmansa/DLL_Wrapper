#pragma once
//I would like to use the standard C fopen
#pragma warning(disable:4996) 

#include <string>
#include "configuration.h"
#include "peheader.h"

#define FORWARDS_FNAME "forwards.h"
#define INTERCEPTS_FNAME "intercepts.h"
#define CALL_TABLE_FNAME "c_ext_intercepts.cpp"

/*
** Global Variables Used By This Class
**
** These variables act as templates for the code generation
** to be used with one of the printf variants - here fprintf.
**
** This is not the safest nor cleanest method to do this, but
** due to the overal lack of template engines in C++, and my
** desire to keep this project lightweight, this was the best
** solution I could find.
**
** This solution is lightweight, maintines readability, and
** simple to modify.
*/
//intercepts_h_template.cpp
extern const char	*intercepts_h_header;
extern const char	*intercepts_h_function;
//c_ext_intercepts_template.cpp
extern const char	*c_ext_intercepts_global;
//intercepted_function_template.cpp
extern const char	*intercepted_function;
//bridge_asm_template.cpp
extern const char	*asm_bridge;

class CodeGenerator
{
	const Configuration::Configuration	&_config;
	std::string							_forwards_fname;
	std::string							_intercepts_fname;
	std::string							_call_table_fname;

	std::string							_generateParamatersDef(const std::vector<Configuration::InterceptFunction::Param> &params);
	std::string							_generateParamatersCall(const std::vector<Configuration::InterceptFunction::Param> &params);

public:
	CodeGenerator(const Configuration::Configuration &config);
	~CodeGenerator();

	bool			generate(const PEHeader::Exports &exports);

	bool			generateForwardsHeader(const PEHeader::Exports &exports);
	bool			generateInterceptsHeader(unsigned int call_table_size);
	bool			generateCExtCallTable(unsigned int call_table_size);
	bool			generateInterceptedFunction(const Configuration::InterceptFunction &function, unsigned int function_index);
	bool			generateBridge(const Configuration::InterceptFunction &function, unsigned int function_index);
};