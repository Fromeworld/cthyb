#!/bin/env pytriqs

import pytriqs.utility.mpi as mpi
from pytriqs.archive import HDFArchive
from pytriqs.operators import *
from pytriqs.applications.impurity_solvers.cthyb import *
from pytriqs.gf.local import *

spin_names = ("up","dn")
mkind = lambda spin: (spin,0) if use_blocks else ("tot",spin)

# Input parameters
beta = 10.0
U = 2.0
mu = 1.0
h = 0.0
V = 0.5
epsilon = 2.3

# Use block structure of G
use_blocks = True
# Use quantum numbers
use_qn = False

n_iw = 1025
n_tau = 10001

N   = n(*mkind("up")) + n(*mkind("dn"))
Nup = n(*mkind("up"))
Ndn = n(*mkind("dn"))

p = {}
p["max_time"] = -1
p["random_name"] = ""
p["random_seed"] = 1235 * mpi.rank + 567
p["length_cycle"] = 1 # 50
p["n_warmup_cycles"] = 10 # 50000
p["n_cycles"] = 100 # 10000000
#p["measure_four_body_correlator"] = (n(*mkind("up")),False)
p["measure_four_body_correlator"] = (n(*mkind("up")) + n(*mkind("dn")),False)
#p["measure_density_matrix"] = True
#p["use_norm_as_weight"] = True

results_file_name = "anderson_%s"%V
if use_blocks: results_file_name += ".block"
if use_qn: results_file_name += ".qn"
results_file_name += ".h5"

mpi.report("Welcome to Anderson (1 correlated site + symmetric bath) test.")

H = U*n(*mkind("up"))*n(*mkind("dn"))

QN = []
if use_qn:
    for spin in spin_names: QN.append(n(*mkind(spin)))
    p["quantum_numbers"] = QN
    p["partition_method"] = "quantum_numbers"

gf_struct = {}
for spin in spin_names:
    bn, i = mkind(spin)
    gf_struct.setdefault(bn,[]).append(i)

mpi.report("Constructing the solver...")

# Construct the solver
S = Solver(beta=beta, gf_struct=gf_struct, n_tau=n_tau, n_iw=n_iw)

mpi.report("Preparing the hybridization function...")

# Set hybridization function
delta_w = GfImFreq(indices = [0], beta=beta)
delta_w << (V**2) * inverse(iOmega_n - epsilon) + (V**2) * inverse(iOmega_n + epsilon)
for spin in spin_names:
    bn, i = mkind(spin)
    S.G0_iw[bn][i,i] << inverse(iOmega_n + mu - {'up':h,'dn':-h}[spin] - delta_w)

mpi.report("Running the simulation...")

# Solve the problem
S.solve(h_int=H, **p)

corr_tau = GfImTime(indices = [0], beta = beta)
corr_tau << InverseFourier(S.correlator)

# Save the results
if mpi.is_master_node():
    with HDFArchive(results_file_name,'w') as Results:
        Results['G_tau'] = S.G_tau
        Results['G_iw'] = S.G_iw
        Results['correlator_iw'] = S.correlator
        Results['correlator_tau'] = corr_tau
        static_observables = {'N' : N,
                              'Nup' : Nup,
                              'Ndn' : Ndn, 
                              'N*N' : N*N,
                              'Nup^2' : Nup*Nup,
                              'Ndn^2' : Ndn*Ndn,
                              'Nup*Ndn' : Nup*Ndn} 
        dm = S.density_matrix
        for oname in static_observables.keys():
            val = trace_rho_op(dm,static_observables[oname],S.h_loc_diagonalization)
            print oname, val
