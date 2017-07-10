#include "stdafx.h"
#include "configuration.h"
#include <iostream>

Configuration::Configuration::Configuration()
{
}

Configuration::Configuration::~Configuration()
{
}

bool Configuration::Configuration::load(const std::string path)
{
	tinyxml2::XMLElement	*xml_element = NULL;
	char					drive[_MAX_DRIVE];
	char					dir[_MAX_DIR];
	char					fname[_MAX_FNAME];
	char					ext[_MAX_EXT];

	_config_path = path;
	if (_config_doc.LoadFile(_config_path.c_str()) != tinyxml2::XML_SUCCESS)
	{
		std::cerr << "Couldn't load file <" << _config_path << ">" << std::endl;
		return false;
	}
	
	_root = _config_doc.FirstChildElement("configuration");
	if (_root == NULL)
	{
		std::cerr << "Error: Invalid XML Configuration. Could not find root element <configuration>" << std::endl;
		return false;
	}

	xml_element = _root->FirstChildElement("target");
	if (xml_element == NULL)
	{
		std::cerr << "Error: Invalid XML Configuration. Could not find child element <target>" << std::endl;
		return false;
	}

	// TODO: Relative paths shoould be handled. Maybe memset the buffers and check if they begin with 0?
	_target_path = xml_element->GetText();
	if (_splitpath_s(_target_path.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT) != 0)
	{
		std::cerr << "Invalid target path: " << _target_path << std::endl;
		return false;
	}
	_target_name = std::string(fname) + std::string(ext);

	xml_element = _root->FirstChildElement("rename_target");
	if (xml_element == NULL)
	{
		std::cerr << "Error: Invalid XML Configuration. Could not find child element <rename_target>" << std::endl;
		return false;
	}
	_target_rename = xml_element->GetText();

	xml_element = _root->FirstChildElement("output_dir");
	if (xml_element == NULL)
	{
		std::cerr << "Error: Invalid XML Configuration. Could not find child element <output_dir>" << std::endl;
		return false;
	}
	_output_dir = xml_element->GetText();

	tinyxml2::XMLNode	*intercepts_root = _root->FirstChildElement("intercepts");
	return _intercepts.load(intercepts_root);
}

const std::string & Configuration::Configuration::getTargetPath() const
{
	return _target_path;
}

const std::string & Configuration::Configuration::getTargetName() const
{
	return _target_name;
}

const std::string & Configuration::Configuration::getTargetRename() const
{
	return _target_rename;
}

const std::string & Configuration::Configuration::getOutputDir() const
{
	return _output_dir;
}

const Configuration::Intercepts & Configuration::Configuration::getInterceptFunctions() const
{
	return _intercepts;
}
