# TCP-westwood-nest-validation
Steps for Ns3 
1. ns3 installation using NS3 allinone (make sure that the ns3 iinstallation is the latest available dev version)
2. create the required topology (here dumbell 1-1 topology is used) in scratch folder
3. run the topology to obtain the data files for appropriate network parameters.
4. plot the graphs using gnuplot.

Steps for NEST
1. install using "pip3 install nitk-nest" or by following the install.md file in the nitk-nest repository.
2. verify installation by running the examples in the root/examples folder in the git repository.
3. modify the dumbell.py file in the examples folder to match the specifications of the topology used in the NS3 simulation (here 1-1 dumbell topology was used)
4. run the file (Superuser permission required), graphs would be plotted and saved automatically.

By default the scales are different for the Ns3 and Nest  plots, here we have changed the scale while extracting the data point files from the "ss" output for Nest and then scaled it down and then plotted the graps via GnuPlot.
