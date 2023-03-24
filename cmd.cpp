#include <unordered_map>
#include <iostream>
#include <string>
#include <sstream>
#include "cmd.h"

struct Cmd
{
	bool        (*fn)(cc0::cmd::Params);
	std::string doc;
	uint32_t    param_count;
	bool        halt_on_fail;
};

static std::unordered_map<std::string, Cmd> &Cmds( void )
{
	static std::unordered_map<std::string, Cmd> cmds;
	return cmds;
}

static Cmd *Command(const std::string &cmd_name)
{
	auto cmd = Cmds().find(cmd_name);
	return cmd != Cmds().end() ? &((*cmd).second) : nullptr;
}

struct Info
{
	std::string app_name;
	std::string version;
	uint64_t    longest_cmd;
};

static Info &Information( void )
{
	static Info info = { "", "", 0 };
	return info;
}

cc0::cmd::Param::Param(const char *param) : m_param(param)
{}

cc0::cmd::Param::operator const char*( void ) const
{
	return m_param;
}

cc0::cmd::Param::Parse<int64_t> cc0::cmd::Param::Int( void ) const
{
	Parse<int64_t> p;
	std::istringstream sin;
	sin.str(m_param);
	p.result = bool(sin >> p.value);
	return p;
}

cc0::cmd::Param::Parse<double> cc0::cmd::Param::Real( void ) const
{
	Parse<double> p;
	std::istringstream sin;
	sin.str(m_param);
	p.result = bool(sin >> p.value);
	return p;
}

cc0::cmd::Param::Parse<bool> cc0::cmd::Param::Bool( void ) const
{
	Parse<bool> p;
	std::istringstream sin;
	sin.str(m_param);
	p.result = bool(sin >> p.value);
	return p;
}

cc0::cmd::Params::Params(char **params, uint32_t param_count) : m_params(params), m_param_count(param_count)
{}

uint32_t cc0::cmd::Params::GetCount( void ) const
{
	return m_param_count;
}

cc0::cmd::Param cc0::cmd::Params::operator[](uint32_t i) const
{
	return i < m_param_count ? Param(m_params[i]) : Param(nullptr);
}

bool cc0::cmd::CmdBase::Run(const cc0::cmd::Params&)
{
	return true;
}

void cc0::cmd::Init(const char *app_name, const char *version)
{
	Information().app_name = app_name;
	Information().version  = version;
}

bool cc0::cmd::Register(const char *cmd_name, bool (*fn)(cc0::cmd::Params), uint32_t param_count, const char *doc, bool halt_on_fail)
{
	std::string cmd = cmd_name;
	for (size_t i = 0; i < cmd.size(); ++i) {
		if (!((cmd[i] >= 'a' && cmd[i] <= 'z') || (cmd[i] >= 'A' && cmd[i] <= 'Z') || (cmd[i] >= '0' && cmd[i] <= '9'))) {
			cmd[i] = '-';
		}
	}
	Cmds()[cmd] = Cmd{ fn, doc, param_count, halt_on_fail };
	Information().longest_cmd = cmd.size() + 3 > Information().longest_cmd ? cmd.size() + 3 : Information().longest_cmd;
	return true;
}

int cc0::cmd::Process(int argc, char **argv, bool halt_on_unrecognized)
{
	bool success = true;
	for (int i = 1; i < argc; ++i) {
		Cmd *a = Command(argv[i]);
		if (a == nullptr) {
			std::cout << "unrecognized command: " << argv[i] << std::endl;
			if (halt_on_unrecognized) {
				return 1;
			}
		} else {
				if (i + a->param_count >= argc) {
				std::cout << "too few parameters: " << argv[i] << std::endl;
			} else if (!a->fn(Params(a->param_count > 0 ? argv + i + 1 : nullptr, a->param_count))) {
				success = false;
				if (a->halt_on_fail) { return 1; }
			}
			i += a->param_count;
		}
	}
	return success ? 0 : 1;
}

CC0_CMD_BEGIN(version)
{
	std::cout << Information().app_name << " " << Information().version << std::endl;
	return true;
}
CC0_CMD_END(version, 0, "Print version.", false)

CC0_CMD_BEGIN(help)
{
	version().Run(cc0::cmd::Params(nullptr, 0));
	for (auto i = Cmds().begin(); i != Cmds().end(); ++i) {
		std::cout << i->first;
		for (uint64_t j = 0; j < Information().longest_cmd - i->first.size(); ++j) {
			std::cout << " ";
		}
		std::cout << i->second.doc << std::endl;
	}
	return true;
}
CC0_CMD_END(help, 0, "Print help.", false)
