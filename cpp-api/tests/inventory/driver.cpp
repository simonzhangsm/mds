/*
 *
 *  Managed Data Structures
 *  Copyright © 2016 Hewlett Packard Enterprise Development Company LP.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  As an exception, the copyright holders of this Library grant you permission
 *  to (i) compile an Application with the Library, and (ii) distribute the 
 *  Application containing code generated by the Library and added to the 
 *  Application during this compilation process under terms of your choice, 
 *  provided you also meet the terms and conditions of the Application license.
 *
 */

#include <getopt.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <signal.h>
#include <cstring>

using namespace std;

int init(int argc, char *argv[],
         const string &inv_name,
         function<void()> usage);
int run(int argc, char *argv[],
        const string &inv_name,
        function<void()> usage);

void show_usage(const string &prog) {
  cerr << "usage: " << prog << " [options] (init|run) [args]" << endl
       << endl
       << "Options:" << endl
       << "-n, --name\t\t Name of the inventory" << endl
       << "-h, --help\t\t Display this message" << endl;
}

void hdl_segv(int sig, siginfo_t *siginfo, void *context) {
  kill(0, SIGSTOP);
}


void stop_on_seg_fault() {
  struct sigaction act;
  std::memset(&act, '\0', sizeof(act));
  act.sa_flags = SA_SIGINFO | SA_RESTART;
  act.sa_sigaction = &hdl_segv;
  if (sigaction(SIGSEGV, &act, NULL) < 0) {
    std::abort();
  }
}

int main(int argc, char *argv[]) {
  stop_on_seg_fault();

  struct option long_options[] = {
    {"name",         required_argument,  0, 'n'},
    {"help",         no_argument,        0, 'h'},
    {0,              0,            0,  0 }
  };

  string prog = argv[0];
  string inv_name = "OurInventory";

  while (true) {
    int c = getopt_long(argc, argv, "+n:h", long_options, nullptr);
    if (c == -1) {
      break;
    }

    switch(c) {
    case 'h':
      show_usage(prog);
      return 0;

    case 'n':
      inv_name = optarg;
      break;

    case '?':
      show_usage(prog);
      return -1;
    }
  }

  if (optind >= argc) {
    cerr << "Command not specified" << endl << endl;
    show_usage(prog);
    return -1;
  }

  string cmd = argv[optind];
  argc -= optind;
  argv += optind;

  if (cmd == "init") {
    return init(argc, argv, inv_name, [&prog](){ show_usage(prog); });
  }
  if (cmd == "run") {
    return run(argc, argv, inv_name, [&prog](){ show_usage(prog); });
  }

  cerr << "Unknown command '" << cmd << "'" << endl << endl;
  show_usage(prog);
  return -1;
    
  
}