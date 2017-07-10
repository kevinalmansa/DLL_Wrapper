#pragma once
#include <string>
#include "intercepts.h"
#include "tinyxml2.h"

namespace Configuration
{
	class Configuration
	{
		std::string				_config_path;
		tinyxml2::XMLDocument	_config_doc;
		tinyxml2::XMLNode		*_root;

		std::string				_target_path;
		std::string				_target_name;
		std::string				_target_rename;
		std::string				_output_dir;
		Intercepts				_intercepts;

	public:
		Configuration();
		~Configuration();

		bool				load(const std::string path);
		
		const std::string	&getTargetPath() const;
		const std::string	&getTargetName() const;
		const std::string	&getTargetRename() const;
		const std::string	&getOutputDir() const;
		const Intercepts	&getInterceptFunctions() const;
	};
}