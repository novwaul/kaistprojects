all:	gaussian	dense	sparse

gaussian:	gaussian.cu
	nvcc	-O3	-std=c++11	-o	gaussian	gaussian.cu

dense:	dense.cu
	nvcc	-O3	-std=c++11	-o	dense	dense.cu

sparse:	sparse.cu	mmreader.cpp
	nvcc	-O3	-std=c++11	-o	sparse	sparse.cu	mmreader.cpp

check:	check-dense	check-gaussian	check-sparse

check-dense:
	./dense	5000	
	./dense	3000
	./dense	1000

check-gaussian:
	./gaussian	5000
	./gaussian	3000
	./gaussian	1000

check-sparse:
	./sparse	matrix/cant.mtx		500
	./sparse	matrix/2cubes_sphere.mtx	500
	./sparse	matrix/consph.mtx	500
	./sparse	matrix/hood.mtx		500
	./sparse	matrix/majorbasis.mtx	500
	./sparse	matrix/offshore.mtx	500
	./sparse	matrix/poisson3Da.mtx	500
	./sparse	matrix/scircuit.mtx	500
	./sparse	matrix/cage12.mtx	500
	./sparse	matrix/cop20k_A.mtx	500
	./sparse	matrix/m133-b3.mtx	500  
	./sparse	matrix/mario002.mtx	500
	./sparse	matrix/patents_main.mtx	500
	./sparse	matrix/pwtk.mtx		500
	./sparse	matrix/shipsec1.mtx	500          
	./sparse	matrix/filter3D.mtx	500
	./sparse	matrix/mac_econ_fwd500.mtx	500
	./sparse	matrix/mc2depi.mtx	200
	./sparse	matrix/pdb1HYS.mtx	500
	./sparse	matrix/rma10.mtx	500
	./sparse	matrix/webbase-1M.mtx	200

clean:
	rm	gaussian
	rm	dense
	rm	sparse
