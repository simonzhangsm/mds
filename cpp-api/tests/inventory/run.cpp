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

#include <vector>
#include <string>
#include <functional>
#include <ostream>
#include <getopt.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include "ruts/util.h"
#include "inventory.h"
#include "experiment.h"


using namespace std;
using namespace mds;
using namespace std::chrono;

namespace {
  int find_max_product(const mds_ptr<Inventory> &inv) {
    mds_ptr<Product> last = inv->products->find_last();
    string name = last->name.read().utf8_copy();
    size_t as_int = stoul(name.substr(1));
    cout << "Max product: "
         << name
         << " (" << as_int << ")"
         << endl;
    return as_int;
  }

  string choose_product(size_t np, double phot, int nhot) {
    bernoulli_distribution d{phot};
    bool choose_hot = d(tl_rand());
    size_t bound = choose_hot ? nhot : np;
    size_t which = uniform_int_distribution<size_t>{1, bound+1}(tl_rand());
    string name = Product::to_name(which);
    return name;
  }
}

struct pub_trace : publish_report {
  static atomic<size_t> n_success_first_try;
  static atomic<size_t> n_success_on_retry;
  static atomic<size_t> n_failure;
  static time_point<system_clock> start_time;

  const size_t _txn;
  const string _desc;
  const size_t _max_tries;
  bool _succeeded;
  size_t _tries = 0;
  const string _pname;

  pub_trace(size_t txn, const string &desc, size_t max_tries, const string &pname = "unknown")
    : _txn{txn}, _desc{desc}, _max_tries{max_tries}, _pname{pname}
  {
  }

  static size_t thread_num() {
    static atomic<size_t> next{0};
    static thread_local size_t num = next++;
    return num;
  }

  void msg(const string &s) const {
    static mutex m;
    lock_guard<mutex> lock(m);

    auto d = system_clock::now()-start_time;
    duration<double> secs = d;
    size_t n_done = n_success_first_try+n_success_on_retry+n_failure;
    double rate = n_done/secs.count();
    

    cout << format([this](auto &os){ os << setw(5) << _txn; }) << ":"
         << " " << _desc
         << " (try " << _tries << " of " << _max_tries << ")"
         << " [thread " << thread_num() << "]"
         << ": " << s
         << "  [" << n_done << " done in " << secs.count() << "s"
         << ", " << rate << "/sec]"
         << endl;
  }

  void before_run(iso_ctxt &ctxt) override {
    static once_flag once;
    call_once(once, [](){
        start_time = system_clock::now();
      });
    if (_tries > 0) {
      msg("conflict detected, will retry");
    }
    _tries++;
    //    msg("start");
  }

  void note_success() override {
    publish_report::note_success();
    if (_tries == 1) {
      msg("succeeded");
      n_success_first_try++;
    } else {
      msg("succeeded after retry");
      n_success_on_retry++;
    }
  }

  void note_failure() override {
    publish_report::note_failure();
    msg("conflict detected, giving up");
    n_failure++;
  }

  static void report_final_result() {
    cout << "Final result:" << endl
         << "    succeeded first try: " << n_success_first_try << endl
         << "  succeeded after retry: " << n_success_on_retry << endl
         << "                 failed: " << n_failure << endl;
  }
    
    
  

};

atomic<size_t> pub_trace::n_success_first_try{0};
atomic<size_t> pub_trace::n_success_on_retry{0};
atomic<size_t> pub_trace::n_failure{0};
time_point<system_clock> pub_trace::start_time;


int run(int argc, char *argv[], const string &inv_name, function<void()> usage) {
  struct option long_options[] = {
    {"phot",             required_argument, 0, 'H' },
    {"nhot",             required_argument, 0, 'h' },
    {"shop",             required_argument, 0, 'n' },
    {"nthreads",         required_argument, 0, 'N' },
    {"minq",             required_argument, 0, 'q' },
    {"maxq",             required_argument, 0, 'Q' },
    {"restock",          required_argument, 0, 'R' },
    {"pblock",           required_argument, 0, 'B' },
    {"blockms",          required_argument, 0, 'b' },
    {"mean_delay_ms",    required_argument, 0, 'm' },
    {"sd_delay_ms",      required_argument, 0, 's' },
    {"max_retries",      required_argument, 0, 'r' },
    {"stop_after",       required_argument, 0, 'S' },
    {"max_transactions", required_argument, 0, 'T' },
    {0,              0,                 0,  0 }
  };

  optind = 0;

  double p_hot_transactions = 0.1;
  size_t n_hot_products = 3;
  string shop_name = "shop1";
  size_t n_threads = 5;
  size_t min_quant = 1;
  size_t max_quant = 20;
  size_t restock_amt = 100;
  double p_block = 0.1;
  milliseconds block = 1000ms;
  milliseconds mean_delay = 100ms;
  milliseconds sd_delay = 10ms;
  size_t max_retries = 3;
  milliseconds run_length = 300s;
  size_t max_transactions = 100000;

  // using fast defaults
  p_block = 0;
  mean_delay = 0ms;
  sd_delay = 0ms;
  block = 0ms;


  while (true) {
    int c = getopt_long(argc, argv, "+", long_options, nullptr);
    if (c == -1) {
      break;
    }

    try {

      switch(c) {
      case 'H':
        p_hot_transactions = stod(optarg);
        break;
      case 'h':
        n_hot_products = stoul(optarg);
        break;
      case 'n':
        shop_name = optarg;
        break;
      case 'N':
        n_threads = stoul(optarg);
        break;
      case 'q':
        min_quant = stoul(optarg);
        break;
      case 'Q':
        max_quant = stoul(optarg);
        break;
      case 'R':
        restock_amt = stoul(optarg);
        break;
      case 'B':
        p_block = stod(optarg);
        break;
      case 'b':
        block = milliseconds(stoul(optarg));
        break;
      case 'm':
        mean_delay = milliseconds(stoul(optarg));
        break;
      case 's':
        sd_delay = milliseconds(stoul(optarg));
        break;
      case 'r':
        max_retries = stoul(optarg);
        break;
      case 'S':
        run_length = seconds(stoul(optarg));
        break;
      case 'T':
        max_transactions = stoul(optarg);
        break;
      case '?':
        usage();
        return -1;
      }
    } catch(invalid_argument &ex) {
      cerr << "Couldn't parse: " << optarg << endl << endl;
      usage();
      return -1;
    }
  }
  if (optind != argc) {
    cerr << "Unexpected argument: '" << argv[optind] << "' for " << argv[0] << endl <<endl;
    usage();
    return -1;
  }

  atomic<size_t> next_txn{0};

  mds_ptr<Inventory> inv = Inventory::lookup(inv_name);
  int max_product = find_max_product(inv);
  shared_ptr<Experiment> experiment = make_shared<Experiment>()
    ->add_action(1, [&](){
        // restock
        string pname = choose_product(max_product, p_hot_transactions, n_hot_products);
        string desc = format([&](auto &os) {
            os << " IN: " << setw(4) << restock_amt << " of " << pname;
          });
        auto trace = make_shared<pub_trace>(++next_txn, desc, max_retries, shop_name);
        try {
          isolated(rerun(max_tries(max_retries)),
                   report_to(trace),
                   [&](){
                     inv->stock_in_name(restock_amt, pname);
                     if (bernoulli_distribution{p_block}(tl_rand())) {
                       this_thread::sleep_for(block);
                     }
                   });
        } catch (publish_failed) {
          // ignored
        }
      })
    ->add_action(10, [&](){
        // sell
        string pname = choose_product(max_product, p_hot_transactions, n_hot_products);
        string desc = format([&](auto &os) {
            os << "OUT: " << setw(4) << restock_amt << " of " << pname;
          });
        size_t units = uniform_int_distribution<size_t>{min_quant, max_quant}(tl_rand());
        // size_t txn = ++next_txn;
        auto trace = make_shared<pub_trace>(++next_txn, desc, max_retries, shop_name);
        try {
          isolated(rerun(max_tries(max_retries)),
                   report_to(trace),
                   [&](){
                     inv->order_out_name(units, pname);
                   });
        } catch (publish_failed) {
          // ignored
        }
      })
    ->set_delay(mean_delay, sd_delay)
    ->stop_after(run_length)
    ->stop_after_n(max_transactions);
  experiment->start(n_threads);
  pub_trace::report_final_result();
  return 0;
}
