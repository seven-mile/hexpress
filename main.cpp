#include <iostream>

import cli;
import login_cmd;
import register_cmd;
import whoami_cmd;
import quit_cmd;

int main()
{
  using namespace hexpress;

  Startup<
    LoginCommandProvider,
    RegisterCommandProvider,
    WhoamiCommandProvider,
    QuitCommandProvider
  >();

  return 0;
}
