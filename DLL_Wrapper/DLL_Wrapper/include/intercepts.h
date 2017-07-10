#pragma once
#include "tinyxml2.h"
#include <string>
#include <vector>

namespace Configuration
{

	struct InterceptFunction
	{
		struct Param
		{
			std::string	name;
			std::string	type;
		};

		std::string			_name;
		std::string			_return_type;
		std::vector<Param>	_paramaters;
	};

	class Intercepts
	{
		//configuration->intercepts
		const tinyxml2::XMLNode			*_root;
		std::vector<InterceptFunction>	_functions;

		bool							_verifyElement(const tinyxml2::XMLElement *element, std::string &value,
			const char *tag);
	
	public:
		Intercepts();
		~Intercepts();

 		bool									load(const tinyxml2::XMLNode *intercepts_root);
		const std::vector<InterceptFunction>	&getFunctions() const;
		bool									contain(const std::string &value) const;
	};
}
 