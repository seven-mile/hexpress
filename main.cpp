#include <iostream>

import cli;

import socket_stream;

// user module
import login_cmd;
import register_cmd;
import whoami_cmd;
import get_users_cmd;
import recharge_cmd;
import change_pass_cmd;
import delete_user_cmd;

// package module
import get_pkgs_cmd;
import get_my_pkgs_cmd;
import send_pkg_cmd;
import collect_pkg_cmd;
import sign_pkg_cmd;

// courier module
import assign_courier_cmd;
import create_courier_cmd;

import quit_cmd;

int main()
{
  using namespace hexpress;

  socketstream server, client;

  server.open("4567", 4);
  server.accept(client);

  std::cout << "Connection from " << client.remote_address() << std::endl;

  if (!client.is_open()) {
    std::cerr << "closed" << std::endl;
    return -1;
  }

  Startup<
    LoginCommandProvider,
    RegisterCommandProvider,
    WhoamiCommandProvider,
    GetUsersCommandProvider,
    RechargeCommandProvider,
    ChangePassCommandProvider,
    DeleteUserCommandProvider,

    GetPkgsCommandProvider,
    GetMyPkgsCommandProvider,
    SendPkgCommandProvider,
    CollectPkgCommandProvider,
    SignPkgCommandProvider,

    AssignCourierCommandProvider,
    CreateCourierCommandProvider,

    QuitCommandProvider
  >(client, client);

  client.shutdown(std::ios::out);
  client.close();
  server.close();

  return 0;
}
