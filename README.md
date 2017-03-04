
## RcppGeiger

C++14 Micro-Benchmarking for R via Rcpp

## About

This package brings the [geiger](https://github.com/david-grs/geiger) C++14
micro-benchmarking library to R.

Note that using it requires a C++14 compiler. It may go to [CRAN](https://cram.r-project.org) once R 3.4.0
is released as this will bring support for C++14 compilation to R.

It can make use of the PAPI interface via e.g. `libpapi-dev` on Debian or Ubuntu.  The
default is to build without PAPI unless enabled.  We plan to make this an automatic
detection via `configure`.

## Examples

In its first release, RcppGeiger contains two of the examples from geiger:

#### simple

The fist example contains two simple lambda function with an single operation each.  We
replaced the call to `std::rand()` with one to R's RNG:

```c++
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
}

```

Running this from R shows the following outpu on my laptop:

```r
R> simple()
Test                    Iterations    ns / iteration
----------------------------------------------------
R::runif               109,254,138                 9
vector push_back        18,143,368                55
R> 
```

#### walk

The second example compares a linear walk through a vector with a random walk. We once
again replace `std::rand` with `R::runif`.  The example show how calls to the
`geiger::suite` instance can be chained.

```c++
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
}

```

It produces the following output on the laptop:

```r
R> walk()
Test               Iterations    ns / iteration
-----------------------------------------------
linear walk           262,144                57
random walk           262,144               800
R> 
```

On a server with both PAPI support and an appropriate cpu, we see

```r
R> walk()
Test               Iterations    ns / iteration       L1 DCM       L2 DCM       L3 TCM
--------------------------------------------------------------------------------------
linear walk           262,144                78         1.00         0.03         0.03
random walk           262,144               322        63.94        63.48        25.55
R> 
```


### Installation

The package is not yet on [CRAN](https://cran.r-project.org) and must
be installed from the repository.  For now cloning and installing
locally may be easiest.  

Of course 

```r
R> remotes::install_github("eddelbuettel/rcppgeiger")
```

works as well.

### Status

The pacage is pretty new and raw. Expect changes. 

There are several TODO items (such as collecting data and bringing it back to R).

### Author

Dirk Eddelbuettel 

### License

GPL (>= 2)
