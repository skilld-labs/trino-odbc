# performance-test-odbc
Performance test framework for testing ODBC driver.

# Preparing the test data
1. Either setup the test database system under the `timestream-iam` DSN or provide values to construct a connection string, instructions to do so are documented below in ["How to run with executable"](#how-to-run-with-executable).
2. Load data into your database using the [Timestream continuous ingestor](https://github.com/awslabs/amazon-timestream-tools/tree/mainline/tools/continuous-ingestor). It is recommended to run the continuous ingestor for around an hour, this will generate around 8.5 million rows.

NOTE: The continuous ingestor is non-deterministic, the performance tests are hardcoded to check values in an already-existing test database that will not match the data you generate for your test database. The only difference will be the number of rows returned by each query. The current dataset uses a very large time frame for the data (`WHERE time BETWEEN now() - 100y AND now()`) and the tests can be ran with no overhead apart from initial setup. If you wanted to run tests with "live" data you would need to change the time frame for tests to be within one hour (`WHERE time BETWEEN now() - 1h AND now()`) for all queries and run the continuous ingestor at the same time.

# How to run with executable
1. Build the performance tool, from the project root:
    - For Windows x86-64: run `.\build_performance_win64.ps1`.
    - For Linux: run `./build_performance_linux.sh`.
    - For macOS: run `./build_performance_mac64.sh`. Note: building is supported on macOS but running the performance tests is not currently supported on macOS.
2. Run executable: 
    - For Windows x86-64: run `.\performance\build\PTODBCResults\Release\performance_results.exe`.
    - For Linux: run `./performance/bin/performance_results`.
    - If you wish to run the "large test," which queries 1,500,000 rows 10 times, add `--large-test` as an argument. Be warned that this causes the runtime of the performance tests to increase from ~12 minutes to ~11 hours.
    - Instead of preconfiguring the `timestream-iam` DSN you can provide `performance_results` with values to construct a connection string. To provide your access key ID and secret key use the arguments `--access-key-id <access key ID> --secret-key <secret key>`. If either your access key ID or secret key are provided both must be provided. You can also set the region to use for testing with `--region <region>`; if provided then an access key ID and secret key must also be provided. Region defaults to `us-west-2` if the access key ID and secret key are provided but region is not. The `Driver` value for the connection string will default to `Amazon Timestream ODBC Driver` and `LogLevel` to `0`. Arguments provided to create the connection string can be in any order. The DSN and any passed-in arguments will be used without validation by the performance testing tool and are the responsibility of the user to check for correctness.

e.g. output console:
```
[ RUN      ] TestPerformance.Time_Execute
%%__PARSE__SYNC__START__%%
%%__QUERY__%% SELECT * FROM performance.employer limit 1
%%__CASE__%% 1: Rows retrieved 1
%%__MIN__%% 62 ms
%%__MAX__%% 81 ms
%%__MEAN__%% 67 ms
%%__MEDIAN__%% 66 ms
%%__90TH_PERCENTILE__%% 72 ms
%%__AVERAGE_MEMORY_USAGE__%% 128 KB
%%__PEAK_MEMORY_USAGE__%% 632 KB
%%__PARSE__SYNC__END__%%
Time dump: 232 ms
[       OK ] TestPerformance.Time_Execute (798 ms)
```
# Performance report
Results are written to `performance_results_report.csv` in the location that `performance_results` was ran from, overwriting any file with the same name.

The output columns are as follows:  
`Test Round,test_name,query,loop_count,Average Time (ms),Max Time (ms),Min Time (ms),Median Time (ms),90th Percentile (ms),Average Memory Usage (KB),Peak Memory Usage (KB)`.

