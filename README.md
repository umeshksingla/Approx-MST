1. Files: 
	Our implementation of the algorithm - amst.cpp
	
	Exact MST implementation to compare - exact_mst.cpp
	(by Sri Aurobindo Mungala and Shruti Gupta, DS Monsoon 2017)
	
	Generate random sample graph test cases - generate_graph.cpp
	// Takes number of vertices and number of edges as input (not argument)
 
	sample_inputs folder - contains input graphs used in report

2. How to run:

	Compile:
	$ (sudo) make

	Generate input:

	$./gen_graph > input.txt
	 N e

	Run:

	$ mpirun -n N ./amst < input.txt,             
	or
	$ mpirun -n N -hosts master ./amst < input.txt
	or
	$ mpirun -n N -hosts client,master ./amst < input.txt

	where N is the number of vertices.

