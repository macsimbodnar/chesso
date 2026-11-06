// S124's phase 2a, 2026-11-06: K at a vector, the reading DEC-352 asks of
// every screen -- each part read at K refit at its fit, iterated, with K at
// the identity beside it. tools/tuner fits K once, at the constants it starts
// from, which are the ones it was built with; a fit's vector is not one, so
// this reads K there: the corpus loaded the tuner's way (tools/tuner_trace.hpp
// load(), every row held to evaluate() at the build's weights), the vector's
// integers in place of the build's, and tools/tuner.cpp's fit_k() -- its
// ternary search over [0.1, 5.0], 40 steps, against the game result, over
// every corpus row in file order, the tuner's own index under --holdout --
// with the same error_range() at the same thread count. Without --vector it
// reads the build's own weights, the identity, which is the tuner's own
// `fitted K` line of a zero-epoch run on the same corpus and threads.
//
//   g++ -std=c++20 -O2 -march=native -DNDEBUG -DCHESSO_TRACE -I src -I tools
//       adocs/data/S124_k_refit.cpp build/src/libchesso_engine_trace.a
//       -lpthread -o S124_k_refit
//   S124_k_refit --data CORPUS [--vector FIT.vector] [--threads 12]
//
// Prints `K <value>` to four places, the tuner's precision, and the training
// error at it. Exits 1 on a load the identity refuses or a vector of another
// width. A reading: it fits no weight.
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "bitboard.hpp"
#include "evaluation.hpp"
#include "tuner_layout.hpp"
#include "tuner_trace.hpp"

namespace
{

// tools/margin_census.cpp's reader of tools/tuner --vector's file: `#` lines,
// then one integer a line, PARAM_COUNT of them.
bool read_vector(const std::string& path,
                 std::vector<double>* weights,
                 std::string* error)
{
  std::ifstream in(path);
  if (!in) {
    *error = "cannot read " + path;
    return false;
  }

  weights->clear();
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#') { continue; }
    char* end = nullptr;
    const long long value = strtoll(line.c_str(), &end, 10);
    if (end == line.c_str() || *end != '\0') {
      *error = "not an integer in " + path + ": " + line;
      return false;
    }
    weights->push_back(static_cast<double>(value));
  }

  if (weights->size() != tuner_layout::PARAM_COUNT) {
    *error = path + " holds " + std::to_string(weights->size()) +
             " values, not " + std::to_string(tuner_layout::PARAM_COUNT);
    return false;
  }
  return true;
}


// tools/tuner.cpp's fit_k(), as it stands at S124's phase 2a.
double fit_k(const tuner_trace::dataset_t& data,
             const double* params,
             const std::vector<uint32_t>& index,
             size_t first,
             size_t last,
             unsigned threads,
             const std::vector<double>& target)
{
  double low = 0.1;
  double high = 5.0;

  for (int step = 0; step < 40; ++step) {
    const double a = low + (high - low) / 3.0;
    const double b = high - (high - low) / 3.0;

    const double ea = tuner_trace::error_range(data, params, a, index, first,
                                               last, threads, target);
    const double eb = tuner_trace::error_range(data, params, b, index, first,
                                               last, threads, target);

    if (ea < eb) {
      high = b;
    } else {
      low = a;
    }
  }

  return (low + high) / 2.0;
}

}  // namespace


int main(int argc, char** argv)
{
  std::string data_path;
  std::string vector_path;
  unsigned threads = std::max(1u, std::thread::hardware_concurrency());

  for (int i = 1; i + 1 < argc; i += 2) {
    const std::string arg = argv[i];
    if (arg == "--data") {
      data_path = argv[i + 1];
    } else if (arg == "--vector") {
      vector_path = argv[i + 1];
    } else if (arg == "--threads") {
      threads = static_cast<unsigned>(atoi(argv[i + 1]));
    } else {
      fprintf(stderr, "unknown option %s\n", arg.c_str());
      return 2;
    }
  }
  if (data_path.empty() || (argc % 2) == 0) {
    fprintf(stderr,
            "S124_k_refit --data CORPUS [--vector FIT.vector] [--threads N]\n");
    return 2;
  }

  tuner_trace::dataset_t data;
  tuner_trace::load_report_t report;
  std::string error;
  if (!tuner_trace::load(data_path, &data, &report, &error, threads)) {
    fprintf(stderr, "could not load %s: %s\n", data_path.c_str(),
            error.c_str());
    return 1;
  }
  if (report.mismatches > 0) {
    fprintf(stderr, "the trace disagrees with evaluate() on %" PRIu64
                    " rows; nothing read\n",
            report.mismatches);
    return 1;
  }

  std::vector<double> params(tuner_layout::PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());
  if (!vector_path.empty() && !read_vector(vector_path, &params, &error)) {
    fprintf(stderr, "%s\n", error.c_str());
    return 1;
  }

  std::vector<uint32_t> index(data.size());
  std::vector<double> target(data.size());
  for (size_t i = 0; i < data.size(); ++i) {
    index[i] = static_cast<uint32_t>(i);
    target[i] = data.result[i];
  }

  const double k =
      fit_k(data, params.data(), index, 0, data.size(), threads, target);
  const double train = tuner_trace::error_range(
      data, params.data(), k, index, 0, data.size(), threads, target);

  printf("S124 K refit: %s, %zu rows, %" PRIu64 " differ from evaluate()\n",
         data_path.c_str(), data.size(), report.mismatches);
  printf("weights   %s\n",
         vector_path.empty() ? "the build's own, the identity"
                             : vector_path.c_str());
  printf("threads   %u\n", threads);
  printf("train     %.6f at K\n", train);
  printf("K %.4f\n", k);
  return 0;
}
