#pragma once

#include <Windows.h>
#include <iostream>
#include <vector>
#include <sstream>
#include "intercepts.h"

class PEHeader
{
public:
	// NESTED STRUCT AND CLASS DEFINITIONS
	struct OrdinalExport {
		WORD		ordinal;
		DWORD		function; //rva
	};

	struct NamedExport {
		WORD		ordinal;
		const char	*name;
		DWORD		function; //rva
	};

	struct ForwardOrdinalExport {
		WORD		ordinal;
		const char	*forward_name;
	};

	struct ForwardNamedExport {
		WORD		ordinal;
		const char	*name;
		const char	*forward_name;
	};

	struct Exports {
		std::vector<OrdinalExport>			ordinal;
		std::vector<NamedExport>			named;
		std::vector<ForwardOrdinalExport>	forward_ordinal;
		std::vector<ForwardNamedExport>		forward_named;
	
		std::stringstream					toVsLinkerExport(const char *targetName,
			const Configuration::Intercepts &intercepts) const;
		std::stringstream					toDefExport(const char * targetName,
			const Configuration::Intercepts & intercepts) const;
		std::stringstream					&defIntercepts(std::stringstream &stream,
			const Configuration::Intercepts & intercepts) const;
		std::stringstream					&defForwards(std::stringstream &stream,
			const std::string &targetname, const Configuration::Intercepts & intercepts) const;
	};

private:
	//PRIVATE ATTRIBUTES
	const char				*_file_name;
	HANDLE					_hFile;
	HANDLE					_hFile_mapping;
	LPVOID					_file_base;
	PIMAGE_DOS_HEADER		_dos_header;
	PIMAGE_NT_HEADERS		_nt_header;
	PIMAGE_DATA_DIRECTORY	_data;
	PIMAGE_EXPORT_DIRECTORY	_exports;

	// STATIC HELPER FUNCTIONS
	static PIMAGE_NT_HEADERS		_imageNtHeader(LPVOID base);
	static PIMAGE_EXPORT_DIRECTORY  _imageExportDirectory(
		const PIMAGE_DOS_HEADER base, const PIMAGE_NT_HEADERS nt_header);

	// PRIVATE METHODS
	void							_setExportEntry(PEHeader::Exports &exports, char *name, WORD ordinal, DWORD function);

public:
	// COMPILER OVERLOADS
	PEHeader(const char *filename);
	~PEHeader();

	// PUBLIC METHODS
	bool			parse();
	Exports			parseExports();
};

