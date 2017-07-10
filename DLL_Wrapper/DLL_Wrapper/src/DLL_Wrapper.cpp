// DLL_Wrapper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "peheader.h"
#include "configuration.h"
#include "codegenerator.h"
#include <fstream>

int main(int ac, char **av)
{
	PEHeader						*pe;
	Configuration::Configuration	*config;

	if (ac < 2)
	{
		std::cout << "Usage: " << av[0] << " configuration.xml" << std::endl;
		return -1;
	}
	// Load configuration file
	config = new Configuration::Configuration();
	if (!config->load(av[1]))
		return -1;

	// Parse target PE Header and retrieve Exports
	pe = new PEHeader(config->getTargetPath().c_str());
	if (!pe->parse())
		return -1;
	PEHeader::Exports peExports = pe->parseExports();

	// Generate intercepted functions stub code and linker commands for
	// forward exports
	CodeGenerator	generator = CodeGenerator(*config);
	generator.generate(pe->parseExports());

	return 0;
}
