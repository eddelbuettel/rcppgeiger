#include <Rcpp.h>

#include "printer.h"
#include "benchmark.h"

#include <boost/algorithm/string.hpp>

#include <string>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <cctype>

namespace geiger
{

namespace printer
{

namespace detail
{
    template <typename _IntT>
    inline std::string to_string_with_commas(_IntT n)
    {
        std::string time_per_task = std::to_string(n);

        for (int pos = time_per_task.length() - 3; pos > 0; pos -= 3)
            time_per_task.insert(pos, ",");

        return time_per_task;
    }

    inline void papi_better_event_name(std::string& name)
    {
        using namespace boost;

        erase_first(name, "PAPI_");
        replace_all(name, "_", " ");
        erase_first(name, "TOT");
        trim(name);
    }

    template <typename _DurationT> struct to_str {};
    template <> struct to_str<std::chrono::nanoseconds>  { static constexpr const char* value = "ns"; };
    template <> struct to_str<std::chrono::microseconds> { static constexpr const char* value = "us"; };
    template <> struct to_str<std::chrono::milliseconds> { static constexpr const char* value = "ns"; };
    template <> struct to_str<std::chrono::seconds>      { static constexpr const char* value = " s"; };
    template <> struct to_str<std::chrono::minutes>      { static constexpr const char* value = "mn"; };
    template <> struct to_str<std::chrono::hours>        { static constexpr const char* value = " h"; };
}

template <typename _DurationT = std::chrono::nanoseconds>
struct console : public printer_base
{
    void on_start(const suite_base& s) override
    {
        std::vector<std::reference_wrapper<const std::string>> names = s.test_names();

        auto it = std::max_element(names.begin(), names.end(), [](const std::string& s1, const std::string& s2)
                                {
                                    return s1.size() < s2.size();
                                });

        m_first_col_width = it->get().size();
        const std::string time_header = detail::to_str<_DurationT>::value + std::string(" / iteration");

        //int width = std::fprintf(stdout, "%-*s %17s %17s", m_first_col_width, "Test", "Iterations", time_header.c_str());
        char txt[512];
        int width = std::snprintf(txt, 511, "%-*s %17s %17s", m_first_col_width, "Test", "Iterations", time_header.c_str());
        Rcpp::Rcout << txt;

#ifdef USE_PAPI
        std::vector<int> papi_events = s.papi_events();
        for (int event : papi_events)
        {
            std::string event_name = get_papi_event_name(event);
            detail::papi_better_event_name(event_name);

            width += std::fprintf(stdout, " %12s", event_name.c_str());
        }
#endif

        /*std::cout*/ Rcpp::Rcout << "\n" << std::string(width, '-') << std::endl;
    }

    void on_test_complete(const std::string& name, const test_report& r) override
    {
        std::string time_per_task = detail::to_string_with_commas(std::chrono::duration_cast<_DurationT>(r.time_per_task()).count());
        std::string iteration_count = detail::to_string_with_commas(r.iteration_count());

        //std::fprintf(stdout, "%-*s %17s %17s", m_first_col_width, name.c_str(), iteration_count.c_str(), time_per_task.c_str());
        char txt[512];
        std::snprintf(txt, 511, "%-*s %17s %17s", m_first_col_width, name.c_str(), iteration_count.c_str(), time_per_task.c_str());
        Rcpp::Rcout << txt;

        for (long long counter : r.papi_counters())
        {
            //std::fprintf(stdout, " %12.2f", counter / double(r.iteration_count()));
            std::snprintf(txt, 511, " %12.2f", counter / double(r.iteration_count()));
            Rcpp::Rcout << txt;
        }

        /*std::cout*/ Rcpp::Rcout << std::endl;
    }

   private:
    int m_first_col_width;
};

}

}
