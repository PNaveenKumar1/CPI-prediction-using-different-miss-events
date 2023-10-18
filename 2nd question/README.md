Input Benchmarks Programs
502.gcc_r
510.parest_r
519.lbm_r
531.deepsjeng_r

Collecting Performance Counter Data: 

Use hardware performance monitoring tool PAPI to collect performance counter values for your benchmark programs. 
Ensure that no other processes are running on your system during data collection, as external factors can influence counter values.

Installing  perf : 
  $ sudo apt-get install linux-tools-common linux-tools-generic linux-tools-`uname -r`

To get the event values type following command:
 
  $ sudo perf stat -x ',' -e branch-load-misses,branch-misses,iTLB-load-misses,dTLB-load-misses,dTLB-store-misses,L1-icache-load-misses,L1-dcache-load-misses,l1d_pend_miss.pending_cycles,l2_rqsts.code_rd_miss,l2_rqsts.all_demand_miss,l2_rqsts.demand_data_rd_miss,l1d_pend_miss.pending_cycles,l2_rqsts.code_rd_miss,l2_rqsts.rfo_miss,cache-misses,LLC-load-misses,LLC-store-misses,instructions,cycles -p "process ID" --interval-print "Interval width"  -d  & taskset -c 4  "path to running process"

The process ID for a running process can be get by command

  $ pidof "path to running process"


Data Preprocessing: 

Counter values obtained using perf stat are not in the suitable format, and contains additional infromation .

Make sure you have installed jupyter notebook in the system to run the program files provided respectively or instal using following command.

  $ pip install jupyter

Feature values in data collected are preprocessed and bought into structural format  using Q2_<benchmark name>_ca.ipynb provided in files) for each respective model.

Linear Regression Model
The linear regression model is build for each benchmark in respective LR_<benchmark name>_ca.ipynb.




Events for the different benchmarks get positive coefficient 
502.gcc_r           branch-load-misses,branch-misses,iTLB-load-misses,l2_rqsts.all_demand_miss,L1-icache-misses,LLC-store-misses,l_rqsts.rfo_miss
510.parest_r        branch-misses, iTLB-load-misses ,L1-ichache-misses, L1-dcache-misses ,dTLB-store-misses,l2_rqsts.code_rd_miss, l2_rqsts.rfo_miss
519.lbm_r           branch-load-misses,iTLB-load-misses,L1-dcache-misses,L1-ichache-misses,dTLB-load-misses,l2_rqsts.code_rd_miss,l2_rqsts.rfo_miss
531.deepsjeng_r     branch-load-misses,L1-dcache-load-misses,L1-dcache-store-misses,dTLB-load-misses,dTLB-load-misses,l2_rqsts.code_rd_miss,LLC-load-misses
