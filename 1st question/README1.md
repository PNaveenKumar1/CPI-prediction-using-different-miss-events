Setup:

    Clone ChampSim from GitHub:

    $ git clone https://github.com/ChampSim/ChampSim

    Extract tar file provided using command:

    $ sudo apt -y install curl zip unzip tar 

    Download the dependencies in the extracted folder:

   $ git submodule update --init
     vcpkg/bootstrap-vcpkg.sh
     vcpkg/vcpkg install

   Remove the files in ChampSim/branch and copy the files from subfolder of 1stquestion in ChampSim/branch.
   Remove btb/basic_btb file from ChampSim and copy basic_btb.cc file from 1stquestion folder in ChampSim/btb. 

   Make changes champsim_config.json as follows:

   1) In champsim_config
      change "branchpredictor" : "predictor"
      Here the "predictor" is the branch predictor you want to run.
   
   
Parameter Changes:
  The parameter values changes in each respective files for each respective simulation were mentioned in the report according the given storage budget. 



Compilation:

  make sure you compile every time you do simulation.

  $ ./config.sh <configuration file>
  $ make 

Download DPC-3 traces:
  These are the traces provided to us
  https://dpc3.compas.cs.stonybrook.edu/champsim-traces/speccpu/602.gcc_s-2375B.champsimtrace.xz
  https://dpc3.compas.cs.stonybrook.edu/champsim-traces/speccpu/605.mcf_s-484B.champsimtrace.xz
  https://dpc3.compas.cs.stonybrook.edu/champsim-traces/speccpu/619.lbm_s-4268B.champsimtrace.xz

Run simulation:
 
 $ bin/champsim --warmup_instructions 200000000 --simulation_instructions 500000000 </path/to/traces/>




