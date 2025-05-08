# run all benchmarks
time /usr/bin/python3 ./scripts/trace_exec_training_list.py --trace_dir ./sample_traces/ --results_dir benchmark_results

# run restricted benchmarks
time /usr/bin/python3 ./scripts/trace_exec_training_list.py --trace_dir ./restricted_traces/ --results_dir benchmark_results