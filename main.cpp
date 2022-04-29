#include <iostream>

import cli;

// user module
import login_cmd;
import register_cmd;
import whoami_cmd;
import get_users_cmd;
import recharge_cmd;

// package module
import get_pkgs_cmd;
import send_pkg_cmd;
import sign_pkg_cmd;

import quit_cmd;

int main()
{
  using namespace hexpress;

  Startup<
    LoginCommandProvider,
    RegisterCommandProvider,
    WhoamiCommandProvider,
    GetUsersCommandProvider,
    RechargeCommandProvider,

    GetPkgsCommandProvider,
    SendPkgCommandProvider,
    SignPkgCommandProvider,

    QuitCommandProvider
  >();

  return 0;
}
