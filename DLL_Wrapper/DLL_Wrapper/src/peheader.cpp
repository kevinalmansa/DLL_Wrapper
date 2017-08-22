#include "stdafx.h"
#include "peheader.h"
#include <DbgHelp.h>

PEHeader::PEHeader(const char *filename)
	: _file_name(filename)
{
}


PEHeader::~PEHeader()
{
}

PIMAGE_NT_HEADERS	PEHeader::_imageNtHeader(LPVOID base)
{
	PIMAGE_NT_HEADERS	ret;
	
	ret = reinterpret_cast<PIMAGE_NT_HEADERS>(
		reinterpret_cast<unsigned char*>(base) +
		static_cast<PIMAGE_DOS_HEADER>(base)->e_lfanew);
	return ret;
}

PIMAGE_EXPORT_DIRECTORY	PEHeader::_imageExportDirectory(
	const PIMAGE_DOS_HEADER base, const PIMAGE_NT_HEADERS ntHeader)
{
	PIMAGE_EXPORT_DIRECTORY	ret;
	PIMAGE_DATA_DIRECTORY pe_data_directory;
	
	pe_data_directory = &ntHeader->OptionalHeader
		.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	ret = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
		ImageRvaToVa(ntHeader, base, pe_data_directory->VirtualAddress, NULL));
	return ret;
}

void PEHeader::_setExportEntry(PEHeader::Exports & exports, char * name, WORD ordinal, DWORD function)
{
	// Check if a forward export: function address is in the exports directory if so
	if (function >= _data->VirtualAddress &&
		function < (_data->VirtualAddress + _data->Size))
	{
		// IS FORWARD EXPORT
		if (name == NULL)
			exports.forward_ordinal.push_back(
		{
			ordinal,
			static_cast<char*>(ImageRvaToVa(_nt_header, _dos_header, function, NULL))
		});
		else
			exports.forward_named.push_back(
		{
			ordinal,
			name,
			static_cast<char*>(ImageRvaToVa(_nt_header, _dos_header, function, NULL))
		});
	}
	else
	{
		//IS NOT FORWARD EXPORT
		if (name == NULL)
			exports.ordinal.push_back({ ordinal, function });
		else
			exports.named.push_back({ ordinal, name, function });
	}
}


bool	PEHeader::parse()
{
	_hFile = CreateFileA(_file_name, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (_hFile == INVALID_HANDLE_VALUE)
	{
		std::cout << "Couldn't open file " << _file_name << std::endl;
		return false;
	}

	_hFile_mapping = CreateFileMapping(_hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (_hFile_mapping == NULL)
	{
		CloseHandle(_hFile);
std::cout << "Couldn't open file mapping with CreateFileMapping" << std::endl;
return false;
	}

	_file_base = MapViewOfFile(_hFile_mapping, FILE_MAP_READ, 0, 0, 0);
	if (_file_base == NULL)
	{
		CloseHandle(_hFile_mapping);
		CloseHandle(_hFile);
		std::cout << "Couldn't map view of file with MapViewOfFile" << std::endl;
		return false;
	}

	_dos_header = static_cast<PIMAGE_DOS_HEADER>(_file_base);
	_nt_header = ImageNtHeader(_dos_header);
	_data = &(_nt_header->OptionalHeader
		.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);
	_exports = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
		ImageRvaToVa(_nt_header, _dos_header, _data->VirtualAddress, NULL));

	/*
		Debug
	*/
	printf("DOS Header: %p\n", _dos_header);
	printf("RVA to NT Header: %lu\n", _dos_header->e_lfanew);
	printf("NT Header: %p\n", _nt_header);
	printf("Exports: %p\n", _exports);

	return true;
}

PEHeader::Exports		PEHeader::parseExports()
{
	PEHeader::Exports	ret;
	DWORD				*functions;		//rva to array of rva's of function pointers
	DWORD				*names;			//rva to array of rva's of string pointers (char *)
	WORD				*namedordinals;	//rva to array of ordinals (WORD)
	DWORD				j;
	char				*name;

	if (_dos_header == NULL || _nt_header == NULL || _data == NULL || _exports == NULL)
		throw std::logic_error("PE_Parser Class Used Without Being Loaded.");

	functions = static_cast<DWORD*>(ImageRvaToVa(_nt_header, _dos_header,
		_exports->AddressOfFunctions, NULL));
	names = static_cast<DWORD*>(ImageRvaToVa(_nt_header, _dos_header,
		_exports->AddressOfNames, NULL));
	namedordinals = static_cast<WORD*>(ImageRvaToVa(_nt_header, _dos_header,
		_exports->AddressOfNameOrdinals, NULL));

	printf("Number of Functions: %lu\n", _exports->NumberOfFunctions);
	printf("Number of Names: %lu\n", _exports->NumberOfNames);

	// Note: should there also be a check for i < max size of ordinal?
	// Headers show ordinals as being a WORD.
	for (DWORD i = 0; i < _exports->NumberOfFunctions; i++)
	{
		// Double check function pointers RVA
		if (functions[i] == NULL)
			printf("WARNING: Function ordinal %lu not exported!\n", i + _exports->Base);

		// Get name of export, this determines if it's exported by name or ordinal
		// AddressOfNameOrdinals is used to lookup the index of the name used in
		// AddressOfNames array.
		j = 0;
		while (j < _exports->NumberOfNames && namedordinals[j] != i)
			++j;
		if (j != _exports->NumberOfNames)
			name = static_cast<char*>(ImageRvaToVa(_nt_header, _dos_header, names[j], NULL));
		else
			name = NULL;

		// Set export entry
		// note: if Forward Export, than the function will point inside the
		// DATA EXPORT DIRECTORY
		_setExportEntry(ret, name, static_cast<WORD>(_exports->Base + i), functions[i]);
	}

	return ret;
}

std::stringstream PEHeader::Exports::toVsLinkerExport(const char *targetName, 
	const Configuration::Intercepts &intercepts) const
{
	std::stringstream	ret;

	for (std::vector<OrdinalExport>::const_iterator itr = ordinal.begin(); itr != ordinal.end(); ++itr)
	{
		//WARNING: Intercepting by ordinal requires following format in config: ord123
		if (!intercepts.contain(std::string("ord") + std::to_string(itr->ordinal)))
		{
			//#pragma comment(linker, "export:ord243=user33.#243,@243,NONAME")
			ret << "#pragma comment(linker, \"/export:ord" << itr->ordinal << "="
				<< targetName << ".#" << itr->ordinal << ",@" << itr->ordinal
				<< ",NONAME\")" << std::endl;
		}
	}
	for (std::vector<NamedExport>::const_iterator itr = named.begin(); itr != named.end(); ++itr)
	{
		if (!intercepts.contain(itr->name))
		{
			//#pragma comment(linker, "export:MessageBox=user33.MessageBox,@243")
			ret << "#pragma comment(linker, \"/export:" << itr->name << "="
				<< targetName << "." << itr->name << ",@" << itr->ordinal
				<< "\")" << std::endl;
		}
	}

	//THESE CAN BE FORWARDED TO TARGET DLL OR DIRECTLY TO WHAT TARGET FORWARDS TO.
	//THIS IMPLEMENTATION WILL FORWARD DIRECTLY, BYPASSING TARGET.
	//CAN BE MANUALLY CHANGED AFTERWARDS.
	for (std::vector<ForwardOrdinalExport>::const_iterator itr = forward_ordinal.begin(); itr != forward_ordinal.end(); ++itr)
	{
		if (!intercepts.contain(std::to_string(itr->ordinal))) {
			//pragma comment(linker, "export:ord243=NTDLL.#243,@243,NONAME")
			//Note: forward_name is from binary, ex: Target_Ordinal_ForwardTo.#1
			ret << "#pragma comment(linker, \"/export:ord" << itr->ordinal << "="
				<< itr->forward_name << ",@" << itr->ordinal
				<< ",NONAME\")" << std::endl;
		}
	}
	for (std::vector<ForwardNamedExport>::const_iterator itr = forward_named.begin(); itr != forward_named.end(); ++itr)
	{
		if (!intercepts.contain(itr->name)) {
			//pragma comment(linker, "export:MessageBox=NTDLL.QMessageBox,@243")
			ret << "#pragma comment(linker, \"/export:" << itr->name << "="
				<< itr->forward_name << ",@" << itr->ordinal
				<< "\")" << std::endl;
		}
	}


	return ret;
}

std::stringstream	&PEHeader::Exports::defIntercepts(std::stringstream &stream,
	const Configuration::Intercepts & intercepts) const
{
	for (std::vector<OrdinalExport>::const_iterator itr = ordinal.begin(); itr != ordinal.end(); ++itr)
	{
		//WARNING: Intercepting by ordinal requires following format: ord123
		if (intercepts.contain(std::string("ord") + std::to_string(itr->ordinal)))
		{
			//ord123=ord123 @123 NONAME
			stream << "\tord" << itr->ordinal << "=" << "ord" << itr->ordinal << " @"
				<< itr->ordinal << " NONAME" << std::endl;
		}
	}
	for (std::vector<NamedExport>::const_iterator itr = named.begin(); itr != named.end(); ++itr)
	{
		if (intercepts.contain(itr->name))
		{
			//name=name @123
			stream << "\t" << itr->name << "=" << itr->name << " @" << itr->ordinal
				<< std::endl;
		}
	}

	// TODO intercepting forwards? sounds like a strange nich need. Won't implement for now.
	return stream;
}

std::stringstream					&PEHeader::Exports::defForwards(std::stringstream &stream,
	const std::string &targetname, const Configuration::Intercepts & intercepts) const
{
	for (std::vector<OrdinalExport>::const_iterator itr = ordinal.begin(); itr != ordinal.end(); ++itr)
	{
		//WARNING: Intercepting by ordinal requires following format: ord123
		if (!intercepts.contain(std::string("ord") + std::to_string(itr->ordinal)))
		{
			//ord123=target.#123 @123 NONAME
			stream << "\tord" << itr->ordinal << "=" << targetname << "." << "#"
				<< itr->ordinal << " @" << itr->ordinal << " NONAME" << std::endl;
		}
	}
	for (std::vector<NamedExport>::const_iterator itr = named.begin(); itr != named.end(); ++itr)
	{
		if (!intercepts.contain(itr->name))
		{
			//name=target.name @123
			stream << "\t" << itr->name << "=" << targetname << "." << itr->name << " @"
				<< itr->ordinal << std::endl;
		}
	}
	//THESE CAN BE FORWARDED TO TARGET DLL OR DIRECTLY TO WHAT TARGET FORWARDS TO.
	//THIS IMPLEMENTATION WILL FORWARD DIRECTLY, BYPASSING TARGET.
	//CAN BE MANUALLY CHANGED AFTERWARDS.
	for (std::vector<ForwardOrdinalExport>::const_iterator itr = forward_ordinal.begin(); itr != forward_ordinal.end(); ++itr)
	{
		if (!intercepts.contain(std::to_string(itr->ordinal))) {
			//ord243=NTDLL.#243 @243 NONAME
			// Note: forward_name is from binary, ex: Target_Ordinal_ForwardTo.#1
			stream << "\tord" << itr->ordinal << "=" << itr->forward_name << " @"
				<< itr->ordinal << " NONAME" << std::endl;
		}
	}
	for (std::vector<ForwardNamedExport>::const_iterator itr = forward_named.begin(); itr != forward_named.end(); ++itr)
	{
		if (!intercepts.contain(itr->name)) {
			//name=NTDLL.name @123
			stream << "\t" << itr->name << "=" << itr->forward_name << " @"
				<< itr->ordinal << " NONAME" << std::endl;
		}
	}
	return stream;
}

std::stringstream PEHeader::Exports::toDefExport(const char * targetName,
	const Configuration::Intercepts & intercepts) const
{
	std::stringstream					ret;

	ret << "; INTERCEPT FUNCTIONS" << std::endl;
	defIntercepts(ret, intercepts);
	//ret << "; FORWARDS" << std::endl;
	//defForwards(ret, targetName, intercepts);
	return ret;
}
