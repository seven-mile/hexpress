#include <iostream>
#include <format>
#include <algorithm>
#include <execution>
#include <thread>
#include <ranges>
#include <concepts>

import cli;
import login_cmd;
import register_cmd;

using namespace hexpress;

int main()
{
  Startup<LoginCommandProvider, RegisterCommandProvider>();

  return 0;
}

