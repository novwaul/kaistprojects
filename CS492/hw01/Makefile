all:	gaussian	dense	sparse

gaussian:	matrix.cpp	gaussian.cpp
	g++	-std=c++11	-o	gaussian	-pthread	gaussian.cpp	matrix.cpp

dense:	matrix.cpp	dense.cpp
	g++	-std=c++11	-o	dense	-pthread	dense.cpp	matrix.cpp

sparse:	sparse.cpp	mmreader.cpp
	g++	-std=c++11	-o	sparse	-pthread	sparse.cpp	mmreader.cpp

check:	check-dense	check-gaussian	check-sparse

check-dense:
	./dense	1000	
	./dense	3000
	./dense	5000

check-gaussian:
	./gaussian	1000
	./gaussian	3000
	./gaussian	5000

check-sparse:
	./sparse	matrix/cant.mtx	1000
	./sparse	matrix/2cubes_sphere.mtx	1000 
	./sparse	matrix/consph.mtx	1000
	./sparse	matrix/hood.mtx	1000
	./sparse	matrix/majorbasis.mtx	1000
	./sparse	matrix/offshore.mtx	1000
	./sparse	matrix/poisson3Da.mtx	1000
	./sparse	matrix/scircuit.mtx	1000
	./sparse	matrix/cage12.mtx	1000
	./sparse	matrix/cop20k_A.mtx	1000
	./sparse	matrix/m133-b3.mtx	1000    
	./sparse	matrix/mario002.mtx	1000
	./sparse	matrix/patents_main.mtx	1000 
	./sparse	matrix/pwtk.mtx	1000
	./sparse	matrix/shipsec1.mtx	1000          
	./sparse	matrix/filter3D.mtx	1000
	./sparse	matrix/mac_econ_fwd500.mtx	1000
	./sparse	matrix/mc2depi.mtx	1000
	./sparse	matrix/pdb1HYS.mtx	1000 
	./sparse	matrix/rma10.mtx	1000
	./sparse	matrix/webbase-1M.mtx	500

clean:
	rm	gaussian
	rm	dense
	rm	sparse
