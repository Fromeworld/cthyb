/*******************************************************************************
 *
 * TRIQS: a Toolbox for Research in Interacting Quantum Systems
 *
 * Copyright (C) 2014, P. Seth, I. Krivenko, M. Ferrero and O. Parcollet
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
#pragma once
#include <triqs/gfs.hpp>
#include <triqs/clef.hpp>
#include "./qmc_data.hpp"
#include "./impurity_trace.hpp"

namespace cthyb {

using namespace triqs::gfs;
using triqs::clef::placeholder;
using mc_sign_type = double;
using node = impurity_trace::node;
using trace_t = impurity_trace::trace_t;
using block_and_matrix = impurity_trace::block_and_matrix;

// Measure the two body correlator C_ab = < (c+_a c_b) >
struct measure_two_body_corr {

 qmc_data const& data;
 gf_view<imfreq, scalar_valued> correlator;
 gf<imfreq, scalar_valued, no_tail> correlator_accum;
 mc_sign_type z;
 qmc_data::trace_t new_atomic_weight, new_atomic_reweighting;
 arrays::array<dcomplex, 2> coefficients; // Coefficients of op*op, where op is a quadratic operator
 bool anticommute;                        // Do the cdag and c operators anticommute?
 impurity_trace& imp_tr;
 impurity_trace::rb_tree_t const& tree;

 dcomplex cdagc;
 dcomplex ccdag;

 measure_two_body_corr(qmc_data const& data, gf_view<imfreq, scalar_valued> correlator, fundamental_operator_set const& fops,
                       many_body_operator const& A, bool anticommute);
 void accumulate(mc_sign_type s);
 void collect_results(triqs::mpi::communicator const& c);

 block_and_matrix compute_normalization_integral(int b_i, time_pt tau_i, time_pt tau_f, op_desc const& op1);
 block_and_matrix compute_normalization_integral(int b_i, time_pt tau_i, time_pt tau_f, op_desc const& op1, op_desc const& op2);
 block_and_matrix compute_normalization_integral(int b_i, time_pt tau_i, time_pt tau_f, op_desc const& op1, op_desc const& op2,
                                                 op_desc const& op3, op_desc const& op4);
 void compute_sliding_trace_integral(std::vector<node> const& flat_config, int index_node, std::vector<int> const& blocks,
                                     gf<imfreq, scalar_valued, no_tail>& correlator_accum);
 dcomplex compute_fourier_sliding_trace(int case_num, int b_i, time_pt tau1, time_pt tau2, op_desc const& op1, op_desc const& op2,
                                        block_and_matrix const& M_outer, matsubara_freq iwn_) const;
 TRIQS_CLEF_IMPLEMENT_LAZY_METHOD(measure_two_body_corr, compute_fourier_sliding_trace);
};
}
