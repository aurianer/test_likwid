#include <hpx/config.hpp>
#include <hpx/hpx.hpp>
#include <hpx/local/init.hpp>
#include <hpx/include/datapar.hpp>
#include <hpx/include/compute.hpp>

#include <string>
#include <type_traits>
#include <vector>
#include <fstream>
#include <cmath>
#include "likwid.h"
std::size_t threads;
using std::sin;
using std::cos;

// Actual test function object
struct test_t
{
    template <typename T>
    void operator()(T &x)
    {
        for (int i = 0; i < 100; i++)
            x = 5 * sin(x) + 6 * cos(x);
    }
} test_{};

// #define SIMD_TEST_WITH_FLOAT
#define SIMD_TEST_WITH_DOUBLE

template <typename ExPolicy, typename T, typename Gen>
auto test(ExPolicy policy, std::size_t n, Gen gen)
{
    static int i = 0;
    using allocator_type = hpx::compute::host::block_allocator<T>;
    using executor_type = hpx::compute::host::block_executor<>;

    auto numa_domains = hpx::compute::host::numa_domains();
    allocator_type alloc(numa_domains);
    executor_type executor(numa_domains);

    hpx::compute::vector<T, allocator_type> nums(n, 0.0, alloc);
    if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
        hpx::generate(hpx::execution::par.on(executor), nums.begin(), nums.end(), gen);
    }
    else
    {
        hpx::generate(hpx::execution::seq, nums.begin(), nums.end(), gen);
    }
    std::string kernel_name = std::string(typeid(policy).name()) + std::to_string(i);
    i++;
    auto t1 = std::chrono::high_resolution_clock::now();
        LIKWID_MARKER_START(kernel_name.c_str());
        if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
            hpx::for_each(policy.on(executor), nums.begin(), nums.end(), test_);
        }
        else
        {
            hpx::for_each(policy, nums.begin(), nums.end(), test_);
        }
        LIKWID_MARKER_STOP(kernel_name.c_str());
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}

template <typename ExPolicy, typename T, typename Gen>
auto test3(ExPolicy policy, std::size_t iterations, std::size_t n, Gen gen)
{
    double avg_time = 0.0;
    for (std::size_t i = 0; i < iterations; i++)
    {
        avg_time += test<ExPolicy, T, Gen>(policy, n, Gen{});
    }
    avg_time /= (double) iterations;
    return avg_time;
}

template <typename T, typename Gen>
void test4(std::string type,
        std::size_t start, std::size_t end,
        std::size_t iterations, Gen gen)
{
    std::string file_name = std::string("plots/") +
                            type +
                            std::string(".csv");
    std::ofstream fout(file_name.c_str());

    auto& seq_pol = hpx::execution::seq;
    auto& par_pol = hpx::execution::par;

    fout << "n,threads,seq,par\n";
    for (std::size_t i = start; i <= end; i++)
    {
        fout << i
            << ","
            << ","
            << threads
            << ","
            << test3<decltype(seq_pol), T, Gen>(
                seq_pol, iterations, std::pow(2, i), gen)
            << ","
            << test3<decltype(par_pol), T, Gen>(
                par_pol, iterations, std::pow(2, i), gen)
            << "\n";
        fout.flush();
    }
    fout.close();
}

struct gen_int_t{
    std::mt19937 mersenne_engine {42};
    std::uniform_int_distribution<int> dist_int {1, 1024};
    auto operator()()
    {
        return dist_int(mersenne_engine);
    }
} gen_int{};

struct gen_float_t{
    std::mt19937 mersenne_engine {42};
    std::uniform_real_distribution<float> dist_float {1, 1024};
    auto operator()()
    {
        return dist_float(mersenne_engine);
   }
} gen_float{};

struct gen_double_t{
    std::mt19937 mersenne_engine {42};
    std::uniform_real_distribution<double> dist_double {1, 1024};
    auto operator()()
    {
        return dist_double(mersenne_engine);
    }
} gen_double{};

int hpx_main(hpx::program_options::variables_map& vm)
{

    std::filesystem::create_directory("plots");
    threads = hpx::get_os_thread_count();
    std::uint64_t const iterations = vm["iterations"].as<std::uint64_t>();
    std::uint64_t const start = vm["start"].as<std::uint64_t>();
    std::uint64_t const end = vm["end"].as<std::uint64_t>();

    #if defined (SIMD_TEST_WITH_INT)
        test4<int, gen_int_t>("int", start, end, iterations, gen_int);
    #endif

    #if defined (SIMD_TEST_WITH_FLOAT)
        test4<float, gen_float_t>("float", start, end, iterations, gen_float);
    #endif

    #if defined (SIMD_TEST_WITH_DOUBLE)
        test4<double, gen_double_t>("double", start, end, iterations, gen_double);
    #endif

    return hpx::local::finalize();
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    namespace po = hpx::program_options;

    po::options_description desc_commandline;
    desc_commandline.add_options()
        ("iterations", po::value<std::uint64_t>()->default_value(1),
         "number of repititions")
        ("start", po::value<std::uint64_t>()->default_value(5),
         "start of number of elements in 2^x")
        ("end", po::value<std::uint64_t>()->default_value(10),
         "end of number of elements in 2^x")
    ;

    // Initialize and run HPX
    hpx::local::init_params init_args;
    init_args.desc_cmdline = desc_commandline;

    LIKWID_MARKER_INIT;
    auto ret = hpx::local::init(hpx_main, argc, argv, init_args);
    LIKWID_MARKER_CLOSE;
    return ret;
}
