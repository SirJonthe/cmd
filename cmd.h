/// @file
/// @author github.com/SirJonthe
/// @date 2023
/// @copyright Public domain.
/// @license CC0 1.0

#ifndef CC0_CMD_H_INCLUDED__
#define CC0_CMD_H_INCLUDED__

#include <cstdint>

/// @brief Begins the definition of a CLI command.
/// @param cmd_class The name of the command class wrapping the function to trigger.
/// @note command block must be ended using CC0_CMD_END using the same class name as CC0_CMD_BEGIN.
/// @sa CC0_CMD_END
#define CC0_CMD_BEGIN(cmd_class) \
	class cmd_class : public cc0::cmd::CmdBase { \
	public: \
		bool Run(cc0::cmd::Params params)

/// @brief Ends a user-defined CLI command.
/// @param cmd_class The name of the command class wrapping the function to trigger.
/// @param param_count The number of parameters this command takes.
/// @param doc A string describing what the command does.
/// @param halt_on_fail If true, tells processing of CLI commands to stop if this function returns false when triggered.
/// @note command block must be ended using CC0_CMD_END using the same class name as CC0_CMD_BEGIN.
/// @sa CC0_CMD_BEGIN
#define CC0_CMD_END(cmd_class, param_count, doc, halt_on_fail) \
}; \
static bool run_##cmd_class(cc0::cmd::Params params) { return cmd_class().Run(params); } \
const static bool cmd_class##_fn_added = cc0::cmd::Register(#cmd_class, run_##cmd_class, param_count, doc, halt_on_fail);

namespace cc0
{
	namespace cmd
	{
		/// @brief A parameter for an command.
		class Param
		{
		public:
			/// @brief A value parsed from the current parameter.
			/// @tparam type_t The requested type.
			template < typename type_t >
			struct Parse
			{
				type_t value;  // The extracted value. Undefined if the parameter can not be converted to the requested type.
				bool   result; // True if the parameter could be converted to the requested type.
			};

		private:
			const char *m_param;

		public:
			/// @brief Converts a string parameter to a wrapper class.
			/// @param param The parameter as a string.
			explicit Param(const char *param);

			/// @brief Returns the parameter as a plain string.
			/// @return The parameter as a plain string.
			operator const char*( void ) const;

			/// @brief Converts the parameter string to an integer.
			/// @return An integer if conversion was successful.
			Parse<int64_t> Int( void ) const;

			/// @brief Converts the parameter string to a real value.
			/// @return A real value if the conversion was successful.
			Parse<double> Real( void ) const;

			/// @brief Converts the parameter string to a boolean value.
			/// @return A boolean value if the conversion was successful.
			Parse<bool> Bool( void ) const;
		};

		/// @brief A series of parameters for an command.
		class Params
		{
		private:
			char     **m_params;
			uint32_t   m_param_count;
		
		public:
			/// @brief Wraps a number of parameters in a wrapper class.
			/// @param params The parameters.
			/// @param param_count The number of parameters.
			Params(char **params, uint32_t param_count);

			/// @brief Returns the number of parameters.
			/// @return The number of parameters.
			uint32_t GetCount( void ) const;

			/// @brief Returns a parameter in a wrapper class.
			/// @param i The index.
			/// @return The parameter at the given index. Returns a parameter referencing null if out of range.
			Param operator[](uint32_t i) const;
		};

		/// @brief A base class for an command.
		class CmdBase
		{
		public:
			/// @brief Executes the command.
			/// @param params The input parameters.
			/// @return True if successful.
			virtual bool Run(const Params &params);
		};

		/// @brief Initializes the cmd library.
		/// @param app_name The name of the application.
		/// @param version The version of the application.
		void Init(const char *app_name, const char *version);

		/// @brief Registers a function to be triggered on a given CLI command.
		/// @param cmd_name The name of the CLI command.
		/// @param fn The function to trigger given the specified CLI command.
		/// @param param_count The number of parameters that the command takes.
		/// @param doc A string documenting the function of the command.
		/// @param halt_on_fail Halts execution of subsequent commands if this command is executed and fails.
		/// @return Always returns TRUE.
		/// @sa CC0_CMD_BEGIN
		/// @sa CC0_CMD_END
		/// @warning Do not use this function directly. Just define your commands using CC0_CMD_BEGIN and CC0_CMD_END.
		bool Register(const char *cmd_name, bool (*fn)(Params), uint32_t param_count, const char *doc, bool halt_on_fail = false);
		
		/// @brief Runs through the CLI commands and triggers the appropriate functions.
		/// @param argc The number of CLI commands passed.
		/// @param argv The CLI commands.
		/// @param halt_on_unrecognized If true, tells the processing to halt if an unknown command is encountered.
		/// @return 0 if all triggered functions return true, 1 otherwise.
		int Process(int argc, char **argv, bool halt_on_unrecognized = false);
	}
}

#endif
