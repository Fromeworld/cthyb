/*******************************************************************************
 *
 * TRIQS: a Toolbox for Research in Interacting Quantum Systems
 *
 * Copyright (C) 2013, P. Seth, I. Krivenko, M. Ferrero and O. Parcollet
 *
 * TRIQS is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * TRIQS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * TRIQS. If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#include "./solver_core.hpp"
#include <triqs/utility/callbacks.hpp>
#include <triqs/utility/exceptions.hpp>
#include <triqs/gfs.hpp>
#include <fstream>
#include <boost/variant.hpp>

#include "move_insert.hpp"
#include "move_remove.hpp"
#include "move_double_insert.hpp"
#include "move_double_remove.hpp"
#include "move_shift.hpp"
#include "measure_g.hpp"
#include "measure_g_legendre.hpp"
#include "measure_perturbation_hist.hpp"

namespace cthyb {

struct index_visitor : public boost::static_visitor<> {
  std::vector<std::string> indices;
  void operator()(int i) { indices.push_back(std::to_string(i)); }
  void operator()(std::string s) { indices.push_back(s); }
};

solver_core::solver_core(double beta_, std::map<std::string, indices_type> const & gf_struct_, int n_iw, int n_tau, int n_l):
  beta(beta_), gf_struct(gf_struct_) {

  if ( n_tau < 2*n_iw ) TRIQS_RUNTIME_ERROR << "Must use as least twice as many tau points as Matsubara frequencies: n_iw = " << n_iw << " but n_tau = " << n_tau << ".";

  std::vector<std::string> block_names;
  std::vector<gf<imfreq>> g0_iw_blocks;
  std::vector<gf<imtime>> g_tau_blocks;
  std::vector<gf<legendre>> g_l_blocks;
  std::vector<gf<imtime>> delta_tau_blocks;

  for (auto const& bl : gf_struct) {
    block_names.push_back(bl.first);
    int n = bl.second.size();

    index_visitor iv;
    for (auto & ind: bl.second) { boost::apply_visitor(iv, ind); }
    std::vector<std::vector<std::string>> indices{{iv.indices,iv.indices}};

    g0_iw_blocks.push_back(gf<imfreq>{{beta, Fermion, n_iw}, {n, n}, indices});
    g_tau_blocks.push_back(gf<imtime>{{beta, Fermion, n_tau}, {n, n}, indices});
    g_l_blocks.push_back(gf<legendre>{{beta, Fermion, static_cast<size_t>(n_l)}, {n,n}, indices});
    delta_tau_blocks.push_back(gf<imtime>{{beta, Fermion, n_tau}, {n, n}, indices});
  }

  _G0_iw = make_block_gf(block_names, g0_iw_blocks);
  _G_tau = make_block_gf(block_names, g_tau_blocks);
  _G_l = make_block_gf(block_names, g_l_blocks);
  _Delta_tau = make_block_gf(block_names, delta_tau_blocks);

}

/// -------------------------------------------------------------------------------------------

void solver_core::solve(solve_parameters_t const & params) { 

  _last_solve_parameters = params;

  // determine basis of operators to use
  fundamental_operator_set fops;
  std::map<std::pair<int,int>,int> linindex;
  int block_index = 0;
  for (auto const & bl: gf_struct) {
    int inner_index = 0;
    for (auto const & a: bl.second) {
      fops.insert(bl.first, a);
      linindex[std::make_pair(block_index, inner_index)] = fops[{bl.first,a}];
      inner_index++;
    }
    block_index++;
  }

  // Make list of block sizes
  std::vector<int> n_inner;
  for (auto const& bl : gf_struct) {
    n_inner.push_back(bl.second.size());
  }

  // Calculate imfreq quantities
  auto G0_iw_inv = map([](gf_const_view<imfreq> x){return triqs::gfs::inverse(x);}, _G0_iw);
  auto Delta_iw = G0_iw_inv;
  auto h_loc = params.h_loc;

  // Add quadratic terms to h_loc
  int b = 0;
  for (auto const & bl: gf_struct) {
    int n1 = 0;
    for (auto const & a1: bl.second) {
      int n2 = 0;
      for (auto const & a2: bl.second) {
        h_loc = h_loc + _G0_iw[b].singularity()(2)(n1,n2).real() * c_dag(bl.first,a1)*c(bl.first,a2);
        n2++;
      }
      n1++;
    }
    b++;
  }

  // Determine terms Delta_iw from G0_iw and ensure that the 1/iw behaviour of G0_iw is correct
  b = 0;
  triqs::clef::placeholder<0> iw_;
  for (auto const & bl: gf_struct) {
    Delta_iw[b](iw_) << G0_iw_inv[b].singularity()(-1)*iw_ + G0_iw_inv[b].singularity()(0);
    Delta_iw[b] = Delta_iw[b] - G0_iw_inv[b];
    _Delta_tau[b]() = inverse_fourier(Delta_iw[b]); 
    _G0_iw[b](iw_) << iw_ + G0_iw_inv[b].singularity()(0) ;
    _G0_iw[b] = _G0_iw[b] - Delta_iw[b];
    _G0_iw[b]() = triqs::gfs::inverse(_G0_iw[b]);
    b++;
  }

  // Report what h_loc we are using
  if (params.verbosity >= 2) std::cout << "The local Hamiltonian of the problem:" << std::endl << h_loc << std::endl;

  // Determine block structure
  if (params.partition_method == "autopartition") {
   if (params.verbosity >= 2) std::cout << "Using autopartition algorithm to partition the local Hilbert space" << std::endl;
   sosp = {h_loc, fops};
  } else if (params.partition_method == "quantum_numbers") {
   if (params.quantum_numbers.empty()) TRIQS_RUNTIME_ERROR << "No quantum numbers provided.";
   if (params.verbosity >= 2) std::cout << "Using quantum numbers to partition the local Hilbert space" << std::endl;
   sosp = {h_loc, params.quantum_numbers, fops};
  } else if (params.partition_method == "none") { // give empty quantum numbers list
   std::cout << "Not partitioning the local Hilbert space" << std::endl;
   sosp = {h_loc, std::vector<real_operator_t>{}, fops};
  } else
   TRIQS_RUNTIME_ERROR << "Partition method " << params.partition_method << " not recognised.";

  if (params.verbosity >= 2) std::cout << "Found " << sosp.n_subspaces() << " subspaces." << std::endl;

  if (params.make_histograms) std::ofstream("impurity_blocks.dat") << sosp;

  // If one is interested only in the atomic problem
  if (params.n_warmup_cycles == 0 && params.n_cycles == 0) return;

  // Delta_tau_current will be viewed by qmc_data
  // Delta_tau_average is the starting guess for annealed Delta calculations -- average of tau = 0 and beta.
  block_gf<imtime,matrix_valued,no_tail> Delta_tau_current = _Delta_tau, Delta_tau_average = _Delta_tau, Delta_tau = _Delta_tau;

  for (auto & g: Delta_tau_average) g(iw_) << 0.5*(g(0.0) + g(beta));
  if (params.n_annealing_steps != 0) Delta_tau_current = Delta_tau_average;

  qmc_data data(beta, params, sosp, linindex, Delta_tau_current, n_inner);
  auto qmc = mc_tools::mc_generic<mc_sign_type>(params.n_cycles, params.length_cycle, params.n_warmup_cycles, params.random_name,
                                                params.random_seed, params.verbosity);

  // Define after_cycle_duty function for mc class
  if (params.n_annealing_steps != 0) {
   auto after_cycle_duty = [&]() mutable {
    if (qmc.thermalized()) return;
    int r = params.n_warmup_cycles/params.n_annealing_steps;
    if (qmc.current_cycle_number() % r != 0) return;
    // On the last of the n_annealing steps, Delta_tau_current will be true Delta_tau
    auto frac = std::max(double(qmc.current_cycle_number())/(r*(params.n_annealing_steps-1)),1.0);
    Delta_tau_current = (1.0-frac)*Delta_tau_average + frac*Delta_tau;
    for (auto& d : data.dets) d.regenerate();
   };
   qmc.set_after_cycle_duty(after_cycle_duty);
  }

  // Moves
  using move_set_type = mc_tools::move_set<mc_sign_type>;
  move_set_type inserts(qmc.rng());
  move_set_type removes(qmc.rng());
  move_set_type double_inserts(qmc.rng());
  move_set_type double_removes(qmc.rng());

  auto& delta_names = _Delta_tau.domain().names();
  for (size_t block = 0; block < _Delta_tau.domain().size(); ++block) {
   int block_size = _Delta_tau[block].data().shape()[1];
   auto const& block_name = delta_names[block];
   auto f = params.proposal_prob.find(block_name);
   double prop_prob = (f != params.proposal_prob.end() ? f->second : 1.0);
   inserts.add(move_insert_c_cdag(block, block_size, data, qmc.rng(), params.make_histograms), "Insert Delta_" + block_name, prop_prob);
   removes.add(move_remove_c_cdag(block, block_size, data, qmc.rng(), params.make_histograms), "Remove Delta_" + block_name, prop_prob);
   if (params.move_double) {
    for (size_t block2 = 0; block2 < _Delta_tau.domain().size(); ++block2) {
     int block_size2 = _Delta_tau[block2].data().shape()[1];
     double_inserts.add(move_insert_c_c_cdag_cdag(block, block2, block_size, block_size2, data, qmc.rng(), params.make_histograms),
                 "Insert Delta_" + delta_names[block] + "_" + delta_names[block2], 1.0);
     double_removes.add(move_remove_c_c_cdag_cdag(block, block2, block_size, block_size2, data, qmc.rng()),
                 "Remove Delta_" + delta_names[block] + "_" + delta_names[block2], 1.0);
    }
   }
  }

  qmc.add_move(inserts, "Insert two operators", 1.0);
  qmc.add_move(removes, "Remove two operators", 1.0);
  if (params.move_double) {
   qmc.add_move(double_inserts, "Insert four operators", 1.0);
   qmc.add_move(double_removes, "Remove four operators", 1.0);
  }
  if (params.move_shift) qmc.add_move(move_shift_operator(data, qmc.rng(), params.make_histograms), "Shift one operator", 1.0);
 
  // Measurements
  if (params.measure_g_tau) {
   auto& g_names = _G_tau.domain().names();
   for (size_t block = 0; block < _G_tau.domain().size(); ++block) {
    qmc.add_measure(measure_g(block, _G_tau[block], data), "G measure (" + g_names[block] + ")");
   }
  }
  if (params.measure_g_l) {
   auto& g_names = _G_l.domain().names();
   for (size_t block = 0; block < _G_l.domain().size(); ++block) {
    qmc.add_measure(measure_g_legendre(block, _G_l[block], data), "G_l measure (" + g_names[block] + ")");
   }
  }
  if (params.measure_pert_order) {
   auto& g_names = _G_tau.domain().names();
   for (size_t block = 0; block < _G_tau.domain().size(); ++block) {
    qmc.add_measure(measure_perturbation_hist(block, data, "histo_pert_order_" + g_names[block] + ".dat"), "Perturbation order (" + g_names[block] + ")");
   }
  }

  // Run! The empty configuration has sign = 1
  qmc.start(1.0, triqs::utility::clock_callback(params.max_time));
  qmc.collect_results(_comm);

  // Get the average sign
  _average_sign = qmc.average_sign();

}

}
