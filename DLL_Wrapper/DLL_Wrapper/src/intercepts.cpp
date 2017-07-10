#include "stdafx.h"
#include "intercepts.h"
#include <iostream>

bool Configuration::Intercepts::_verifyElement(const tinyxml2::XMLElement * element, std::string & value, const char *tag)
{
	if (element == NULL)
	{
		std::cerr << "Error: Could not find child element <name> for <function>" << std::endl;
		return false;
	}
	value = element->GetText();
	if (value.empty())
	{
		std::cerr << "Error: Empty child element" << tag << " found for <function>" << std::endl;
		return false;
	}
	return true;
}

Configuration::Intercepts::Intercepts()
{
}

Configuration::Intercepts::~Intercepts()
{
}

// TODO Error checking
bool Configuration::Intercepts::load(const tinyxml2::XMLNode *intercepts_root)
{
	const tinyxml2::XMLElement				*element;
	const tinyxml2::XMLElement				*paramaters;
	const tinyxml2::XMLElement				*child;

	Configuration::InterceptFunction		intercept;
	Configuration::InterceptFunction::Param	param;

	if (intercepts_root == NULL)
		return false;
	_root = intercepts_root;
	element = _root->FirstChildElement("function");
	while (element != NULL)
	{
		if (!_verifyElement(element->FirstChildElement("name"), intercept._name, "name"))
			return false;
		if (!_verifyElement(element->FirstChildElement("return_type"), intercept._return_type, "return_type"))
			return false;

		child = element->FirstChildElement("paramaters");
		if (child == NULL)
		{
			std::cerr << "Warning: <paramaters> is missing from <function> " << intercept._name
				<< ". Assuming the function does not have paramaters (void)." << std::endl;
		}
		else
		{
			paramaters = child->FirstChildElement("param");
			while (paramaters != NULL)
			{
				param.type = std::string(paramaters->Attribute("type"));
				param.name = std::string(paramaters->GetText());

				if (param.type.empty())
				{
					std::cerr << "Error: Attribute \"type\" must be set for <param>" << std::endl;
					return false;
				}
				if (param.name.empty())
				{
					std::cerr << "Error: Value must be set for <param>. It is used as the name." << std::endl;
					return false;
				}
				intercept._paramaters.push_back(param);
				paramaters = paramaters->NextSiblingElement("param");
			}
		}
		_functions.push_back(intercept);
		element = element->NextSiblingElement("function");
	}
	return true;
}

const std::vector<Configuration::InterceptFunction>& Configuration::Intercepts::getFunctions() const
{
	return _functions;
}

bool Configuration::Intercepts::contain(const std::string & value) const
{
	for (auto itr = _functions.begin(); itr != _functions.end(); ++itr)
	{
		if (value == itr->_name)
			return true;
	}
	return false;
}
