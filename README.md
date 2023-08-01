# cmd
## Copyright
Public domain, 2023

github.com/SirJonthe

## About
`cmd` is a minimalist framework for C++11 that processes incoming commands from the CLI.

## Design
`cmd` is designed to be as minimalist as possible while still achieving the goal of being a handy tool. 

## Usage
`cmd` provides only a few contact points for the programmer to use. Once a commands are correctly defined using the provided library functionality the commands are triggered by the command line when calling a single `Process` function in the program.

## Macros
In order to create a command, the user needs to make use of two macros; `CC0_CMD_BEGIN(cmd_class)` and `CC0_CMD_END(cmd_class, param_count, doc, halt_on_fail)`. These macros add boilerplate code which sets up the command as well as ensures that the command is automatically added to the list of commands to be run when calling `Process` (if triggered from the CLI). When naming commands, use snake case to describe the command as `cmd` automatically detects the wording and automatically sets up the name of the command when triggering it from the command line.

## Functions
`Process` runs all user-defined commands, assuming they have been created correctly.

## Building
No special adjustments need to be made to build `cmd` except enabling C++11 compatibility or above. Simply include the relevant headers in your code and make sure the headers and source files are available in your compiler search paths. Using `g++` as an example, building is no harder than:

```
g++ -std=c++11 code.cpp cmd/cmd.cpp
```

...where `code.cpp` is an example source file containing the user-defined commands as well as the entry point for the program.

## Examples
### Creating a command:
Creating a command is as easy as using two macros inside an implementation file.

```
#include <iostream>
#include "cmd/cmd.h"

CC0_CMD_BEGIN(hello_cmd)
{
	std::cout << "Hello, cmd!" << std::endl;
	return true;
}
CC0_CMD_END(hello_cmd, 0, "Prints \"Hello, cmd!\"", false)
```

The return value, `true`, is used for `cmd` to determine the success of running the command. This command always succeeds. This ties into the the final boolean parameter in `CC0_CMD_END`, which tells `cmd` to keep processing incoming commands regardless of the success status from running the current command.

Commands should be created inside an implementation file, such as `.c` and `.cpp` files. This reduces naming pollution and exposing internal functionality to the rest of the program.

### Preventing further execution on command failure:
The final boolean parameter in `CC0_CMD_END` controls whether execution of the command suite should abort if the current command fails. By setting this to true, the below command will never run the `another_cmd` command:

```
#include <iostream>
#include "cmd/cmd.h"

// must_pass will always fail.
CC0_CMD_BEGIN(must_pass)
{
	return false;
}
CC0_CMD_END(must_pass, 0, "Running this command will stop processing further commands.", true)

// If this command is specified in the command line after must_pass, then it will never run, since must_pass will always fail and halt further processing of commands.
CC0_CMD_BEGIN(another_cmd)
{
	std::cout << ":(" << std::endl;
}
CC0_CMD_END(another_cmd, 0, "Just another command.", false)
```

Commands execute in the order of occurrence in the command line.

### Processing parameters inside commands:
Commands may take input parameters. The command needs to specify that it takes a certain number of input parameters upon its definition (see the second parameter in `CC0_CMD_END below`). Then, the parameters are accessable via a `params` variable:

```
CC0_CMD_BEGIN(add_two_values)
{
	Parse<int64_t> p1 = params[0].Int();
	Parse<int64_t> p2 = params[1].Int();

	if (p1.result && p2.result) {
		std::cout << p1.value + p2.value << std::endl;
		return true;
	}
	return false;
}
CC0_CMD_END(add_two_values, 2, "Adds two specified values together and prints the result.", false)
```

The `params` variable is a wrapper class around an array of plain C strings, but can convert said strings to three different types; integer, real, and boolean via methods `Int()`, `Real()`, and `Bool()` respectively (conversion to C string is done implicitly via operator overloading). The returned value from a conversion is an object that stores the success or failure of the conversion in a `result` variable and the value resulting from the conversion in a `value` variable. If the result is false, the value is undefined.

There is currently no type-checking other than what the user themselves enforce. However, the number of parameters is guaranteed to match the expected count.

### Triggering commands:
All commands created using the method as defined above are run using the `Process` function.

```
#include "cmd/cmd.h"

int main(int argc, char **argv)
{
	return cc0::cmd::Process(argc, argv);
}
```

It is also a good idea to specify some additional data using the `Init` function, although it is not mandatory to do so:

```
#include "cmd/cmd.h"

int main(int argc, char **argv)
{
	cc0::cmd::Init("cmd-test", "1.0");
	return cc0::cmd::Process(argc, argv);
}
```

By default, unrecognized commands are ignored and execution of subsequent commands are resumed. By specifying an additional boolean parameter, `halt_on_unrecognized`, in `Process`, `cmd` can be made to stop processing commands if an unrecognized command is encountered.

```
#include "cmd/cmd.h"

int main(int argc, char **argv)
{
	cc0::cmd::Init("cmd-test", "1.0");
	return cc0::cmd::Process(argc, argv, true);
}
```

`Process` returns `1` when some command that was run fails, and `0` otherwise.

Once commands are coded properly and the `Process` function is called in-code, commands can be triggered by providing the resulting executable with parameters in the command line.

Always use snake case when naming the commands in-code as they are automatically converted to strings that are used to trigger them from the command line (characters other than a-z, A-Z, and 0-9 are replaced by a hyphen). The following code will allow `just_some_command` to be triggered from the command line:

### Complete example:
First, we define the command we want to be able to run. We do this in a file called `add_two_values.cpp`:

```
#include <iostream>
#include "cmd/cmd.h"

CC0_CMD_BEGIN(add_two_values)
{
	auto p1 = params[0].Int();
	auto p2 = params[1].Int();

	if (p1.result && p2.result) {
		std::cout << p1.value + p2.value << std::endl;
		return true;
	}
	return false;
}
CC0_CMD_END(add_two_values, 2, "Adds two specified values together and prints the result.", false)
```

Then, in `main.cpp` we do:
```
#include "cmd/cmd.h"

int main(int argc, char **argv)
{
	cc0::cmd::Init("complete-example", "1.0");
	return cc0::cmd::Process(argc, argv, true);
}
```

To compile on Unix using g++, from the command line we do:
```
g++ -std=c++11 main.cpp add_two_values.cpp cmd/cmd.cpp -o app
```

When successful, we run the command from the command line like so:
```
app add-two-values 13 15
> 28
```

## Documentation and help
`cmd` defines two commands by default: `version` and `help`.

`version` prints the application name and the version number as specified by `Init`. Invoking it from the command line is as simple as follows (the output from the command line below are from a made-up example, where `app` is the name of the executable using `cmd`):

```
app version
> Super App 1.0
```

`help` also prints the application name and the version number as specified by `Init`, and also prints the available commands and their documentation strings. Invoking it from the command line is as simple as follows (the output from the command line below are from a made-up example, where `app` is the name of the executable using `cmd`):

```
app help
> Super App 1.0
> custom-command   This is a custom command. It does not do anything.
> help             Print help.
> version          Print version.

```

`help` is automatically populated when creating commands uing `CC0_CMD_BEGIN` and `CC0_CMD_END`. It is therefore important to use relatively descriptive command names and a good documentation string.

## Future work
What string gets exposed to the command line to trigger a command could use fine-tuning, such as preventing multiple hyphens next to each other.

There should also be some way for short names for the commands to be generated, either manually or automatically based on the long name.

Type-checking could be implemented in the internals of `cmd` where the user can specify what types arguments to commands are expected to have and throws an error if the type is not what is expected. This could be done by replacing the `param_count` variable in `CC0_CMD_END` with a string where each character represents the expected type (`i` for integer, `r` for real, `b` for boolean, and `s` for string) and the length of the string defines the number of expected parameters.

The implementation file is currently using STL. It is desirable to remove this dependency.
