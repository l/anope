/* Build bumper
 *
 * (C) 2003-2012 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>

static std::string get_git_hash(const std::string &git_dir)
{
	std::fstream fd;
	std::string filebuf;

	fd.open((git_dir + "/HEAD").c_str(), std::ios::in);
	if (!fd.is_open())
		return "";
	if (!getline(fd, filebuf) || filebuf.find("ref: ") != 0)
	{
		fd.close();
		return "";
	}
	
	fd.close();

	filebuf = filebuf.substr(5);
	fd.open((git_dir + "/" + filebuf).c_str(), std::ios::in);
	if (!fd.is_open())
		return "";
	if (!getline(fd, filebuf))
	{
		fd.close();
		return "";
	}
	fd.close();

	return "g" + filebuf.substr(0, 7);
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		std::cerr << "Syntax: " << argv[0] << " <base> <version.h>" << std::endl;
		return 1;
	}

	std::string version_sh = std::string(argv[1]) + "/src/version.sh";
	std::string git_dir = std::string(argv[1]) + "/.git";

	std::fstream fd;

	fd.clear();
	fd.open(version_sh.c_str(), std::ios::in);
	if (!fd.is_open())
	{
		std::cerr << "Error: Unable to open src/version.sh for reading: " << version_sh << std::endl;
		return 1;
	}

	std::string filebuf;
	std::list<std::pair<std::string, std::string> > versions;
	while (getline(fd, filebuf))
	{
		if (!filebuf.find("VERSION_"))
		{
			size_t eq = filebuf.find('=');

			std::string type = filebuf.substr(0, eq);
			std::string value = filebuf.substr(eq + 2, filebuf.length() - eq - 3);
			versions.push_back(std::make_pair(type, value));
		}
	}

	fd.close();

	std::string git_version = get_git_hash(git_dir);
	if (!git_version.empty())
		versions.push_back(std::make_pair("VERSION_GIT", git_version));

	fd.clear();
	fd.open(argv[2], std::ios::in);

	std::string build = "#define BUILD	1";
	if (fd.is_open())
	{
		while (getline(fd, filebuf))
		{
			if (!filebuf.find("#define BUILD"))
			{
				size_t tab = filebuf.find('	');

				int ibuild = atoi(filebuf.substr(tab + 1).c_str()) + 1;

				std::stringstream ss;
				ss << "#define BUILD	" << ibuild;
				build = ss.str();
			}
		}

		fd.close();
	}

	fd.clear();
	fd.open(argv[2], std::ios::out);

	if (!fd.is_open())
	{
		std::cerr << "Error: Unable to include/version.h for writing: " << argv[2] << std::endl;
		return 1;
	}

	fd << "/* This file is automatically generated by version.cpp - do not edit it! */" << std::endl;

	for (std::list<std::pair<std::string, std::string> >::iterator it = versions.begin(), it_end = versions.end(); it != it_end; ++it)
	{
		if (it->first == "VERSION_EXTRA" || it->first == "VERSION_GIT")
			fd << "#define " << it->first << " \"" << it->second << "\"" << std::endl;
		else
			fd << "#define " << it->first << " " << it->second << std::endl;
	}

	fd << build << std::endl;

	fd.close();

	return 0;
}
