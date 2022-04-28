module;
#include <string>
#include <array>
#include <map>
#include <concepts>
#include <ranges>
#include <algorithm>
#include <regex>
#include <iostream>
#include <format>
export module cli;

namespace hexpress {

  template<class T>
  inline constexpr bool is_char_pointer_array_in_v = false;

  template<std::size_t Size>
  inline constexpr bool is_char_pointer_array_in_v<std::array<const char*, Size>> = true;

  template<class T>
  inline constexpr bool is_char_pointer_array_v = is_char_pointer_array_in_v< std::remove_cvref_t<T>>;

  export template<class T>
  concept CharPointerArray = is_char_pointer_array_v<T>;

  export template<class T>
  concept CLIProvider =
  requires(
    std::map<std::string, std::string> const& arg,
    std::ostream & output) {
      { T::Prototype } -> CharPointerArray;
      { T::ExecuteCommand(output, arg) } -> std::same_as<bool>;
  };

  export template<class T>
  concept CLIProviderHasOptional = requires() {
    { T::Optional } -> CharPointerArray;
  };

  export class CommandLineManager {
    std::istream& input;
    std::ostream& output;

    void IgnoreUtilNextInput() {
      const std::streamsize MAX_LINE = 1000000;
      input.clear();
      input.ignore(MAX_LINE, '\n');
    }

    bool ParseLine(
      std::string& command_name,
      std::map<std::string, std::string>& command_arg_list) {

      std::string line;
      std::getline(input, line);

      if (input.fail()) {
        return false;
      }

      auto first_split = line.find_first_of(' ');

      if (first_split == line.npos) {
        command_name = line;
        return true;
      }

      command_name = line.substr(0, first_split);

      std::regex param_regex{ "(\\w+)=\"(\\w*)\"" };
      std::sregex_iterator param_begin{
        line.begin() + first_split + 1,
        line.end(),
        param_regex
      }, param_end{};

      for (auto& it = param_begin; it != param_end; ++it) {
        command_arg_list[it->str(1)] = it->str(2);
      }

      return true;
    }

    template<CLIProvider P, bool OutputError = false>
    bool CheckPrototype(
      std::string const& cmd,
      std::map<std::string, std::string> const& args) {

      if (P::Prototype[0] != cmd) {
        if constexpr (OutputError) {
          output << std::format("ERROR: invalid command name {}.\n", cmd.c_str());
        }
        return false;
      }
      for (auto it = std::next(std::begin(P::Prototype)); it != std::end(P::Prototype); ++it) {
        if (!args.count(*it)) {
          //if constexpr (OutputError) {
          output << std::format("ERROR: parameter \"{}\" missing!\n", *it);
          //}
          return false;
        }
      }
      return true;
    }

  public:

    CommandLineManager(std::istream& input, std::ostream& output)
      : input(input), output(output) { }

    template<CLIProvider P>
    bool ProcessLine(
      std::string const &name,
      std::map<std::string, std::string> const &arg) {

      if (CheckPrototype<P>(name, arg)
        && P::ExecuteCommand(output, arg)) {
        return true;
      }
      return false;
    }

    template<CLIProvider P, CLIProvider Q, CLIProvider... Ps>
    bool ProcessLine(
      std::string const &name,
      std::map<std::string, std::string> const &arg) {
      if (ProcessLine<P>(name, arg)) return true;
      if (ProcessLine<Q, Ps...>(name, arg)) return true;
      return false;
    }

    template<CLIProvider... Ps>
    void ProcessStream() {
      while (!input.eof()) {
        std::string name;
        std::map<std::string, std::string> arg;
        while (input.fail() || !ParseLine(name, arg)) {
          IgnoreUtilNextInput();
        }
        ProcessLine<Ps...>(name, arg);
      }
    }

    std::istream& GetInputStream() { return input; }
    std::ostream& GetOutputStream() { return output; }
  };

  template<CLIProvider P>
  static void OutputCommandList(std::ostream& output) {
    output << std::format("{}: ", P::Prototype[0]);
    for (auto it = std::next(std::begin(P::Prototype));
      it != std::end(P::Prototype); ++it) {

      output << std::format("<{}> ", *it);
    }

    if constexpr (CharPointerArray<P>) {
      for (auto& val : P::Optional) {
        output << std::format("[{}] ", val);
      }
    }

    output << std::endl;
  }

  template<CLIProvider P, CLIProvider Q, CLIProvider... Ps>
  static void OutputCommandList(std::ostream& output) {
    OutputCommandList<P>(output);
    OutputCommandList<Q, Ps...>(output);
  }

  template<CLIProvider P>
  static void OutputCommandHelp(std::ostream& output, std::string const& command) {
    if (command != P::Prototype[0]) return;

    output << std::format("{}: ", P::Prototype[0]);
    for (auto it = std::next(std::begin(P::Prototype));
      it != std::end(P::Prototype); ++it) {

      output << std::format("<{}> ", *it);
    }

    if constexpr (CharPointerArray<P>) {
      for (auto& val : P::Optional) {
        output << std::format("[{}] ", val);
      }
    }

    output << std::endl;
  }

  template<CLIProvider P, CLIProvider Q, CLIProvider... Ps>
  static void OutputCommandHelp(std::ostream& output, std::string const& command) {
    OutputCommandHelp<P>(output, command);
    OutputCommandHelp<Q, Ps...>(output, command);
  }

  template<CLIProvider... Ps>
  struct HelpCommandProvider {

    constexpr static std::array Prototype{
      "help",
    };

    constexpr static std::array Optional{
      "command"
    };

    static bool ExecuteCommand(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      if (auto it = args.find("command"); it != args.end() && it->second != "help") {

        if constexpr (sizeof...(Ps) > 0)
          OutputCommandHelp<Ps...>(output, it->second);
        output << std::endl;

      } else {

        output << R"(
HExpress Management System [ Version 0.10 ]
===========================================

Command syntax: <Command Name> <ParameterA>="<ValueA>" [OptionalB]="<ValueB>"...

help [command]
)";
        if constexpr (sizeof...(Ps) > 0)
          OutputCommandList<Ps...>(output);

        output << std::endl;
      }

      return true;
    }
  };

  export template<hexpress::CLIProvider... Ps>
  void Startup() {
    hexpress::CommandLineManager cli{ std::cin, std::cout };

    cli.ProcessStream<hexpress::HelpCommandProvider<Ps...>, Ps...>();
  }

}