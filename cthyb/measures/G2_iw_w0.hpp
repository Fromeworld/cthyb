/*******************************************************************************
 *
 * TRIQS: a Toolbox for Research in Interacting Quantum Systems
 *
 * Copyright (C) 2016, P. Seth, I. Krivenko, H. U.R. Strand, M. Ferrero and O. Parcollet
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

#include <vector>
#include <triqs/mpi/base.hpp>
#include <triqs/statistics/histograms.hpp>
#include <triqs/experimental/nfft_array.hpp>

#include "../qmc_data.hpp"

#include "util.hpp"
#include "G2_iw.hpp"

// DEBUG
#include <triqs/utility/timer.hpp>

namespace cthyb {

  namespace G2_iw {
    using M_diag_block_t = block_gf<imfreq, matrix_valued>;
    using M_diag_t       = M_diag_block_t::g_t;
    using M_diag_mesh_t = M_diag_block_t::g_t::mesh_t;

    using M_diag_arr_t = array<std::complex<double>, 3>;
    using M_diag_block_arr_t = std::vector<M_diag_arr_t>;
  }

  using namespace triqs::arrays;
  using namespace triqs::experimental;

  using namespace G2_iw;

  // Measure the two-particle Green's function in Matsubara frequency
  template <G2_channel Channel> struct measure_G2_iw_w0 {

    public:
    measure_G2_iw_w0(std::optional<G2_iw_t> &G2_iw_opt, qmc_data const &data, G2_measures_t const &G2_measures);
    void accumulate(mc_weight_t s);
    void collect_results(triqs::mpi::communicator const &c);

    private:

    qmc_data const &data;
    G2_iw_t::view_type G2_iw;
    mc_weight_t average_sign;
    block_order order;
    G2_measures_t G2_measures;

    M_block_t M;
    M_mesh_t M_mesh;
    M_diag_block_arr_t M_block_arr;

    triqs::utility::timer timer_M_ww_fill;
    triqs::utility::timer timer_M_arr_fill;
    triqs::utility::timer timer_MM_prod;    
    
  };

} // namespace cthyb
