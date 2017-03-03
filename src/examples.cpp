// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

#include <Rcpp.h>

#include "geiger/geiger.h"

//' Simple microbenchmarking examples
//'
//' @title Simple 'Geiger' micro-benchmarking examples 
//' @return None
// [[Rcpp::export]]
void simple() {

    geiger::init();

    // A benchmark suite that does only time measurement
    geiger::suite<> s;

    s.add("R::runif",
          []() {
              //std::rand();
              R::runif(0, RAND_MAX);
          });

    s.add("vector push_back",
          []() {
              std::vector<int> v;
              v.push_back(1000);
          });

    // Redirection of each test result to the "console" printer
    s.set_printer<geiger::printer::console<>>();

    // Run all benchmarks
    s.run();

    //return 0;
}



static const int size = 1024 * 1024 * 16;
static const int batch = 64;
static const int mask = size - 1;
static const int prime = 7919;
static int sum = 0;

auto linear_walk() {
    std::vector<char> v(size, 'a');

    return  [v = std::move(v)]() {
        static std::size_t pos = /*std::rand()*/ static_cast<int>(R::runif(0, RAND_MAX)) & mask;

        for (int i = 0; i < batch; ++i) {
            sum += v[pos];
            pos = (pos + 1) & mask;
        }
    };
}

auto random_walk() {
    std::vector<char> v(size, 'a');

    return  [v = std::move(v)]() {
        static std::size_t pos = /*std::rand()*/ static_cast<int>(R::runif(0, RAND_MAX)) & mask;

        for (int i = 0; i < batch; ++i) {
            sum += v[pos];
            pos = (pos * prime) & mask;
        }
    };
}


//' @rdname simple
// [[Rcpp::export]]
void walk() {
    geiger::init();

#ifdef USE_PAPI
    geiger::suite<geiger::cache_profiler> s;
#else
    geiger::suite<> s;
#endif

    s.add("linear walk", linear_walk())
     .add("random walk", random_walk())
     .set_printer<geiger::printer::console<>>()
     .run(size / batch);

    //return 0;
}
