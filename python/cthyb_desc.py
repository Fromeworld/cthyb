from wrap_generator import *

# The cthyb module
module = module_(full_name = "pytriqs.applications.cthyb", doc = "The cthyb matrix solver")
module.use_module('gf')
module.use_module('parameters')
module.use_module('operators2')
module.add_include("c++/ctqmc.hpp")
module.add_using("namespace cthyb")

# The Solver class
cthyb = class_(
        py_type = "SolverCore",
        c_type = "ctqmc"
        )

module.add_class(cthyb)

cthyb.add_constructor(signature = "(double beta, std::map<std::string, std::vector<int>> gf_struct, int n_iw=5000, int n_tau=10001, int n_l=50)", doc = """ """)

cthyb.add_method(name = "solve",
             signature = "void(triqs::utility::many_body_operator<double> h_loc, params::parameters params, std::vector<triqs::utility::many_body_operator<double>> quantum_numbers = std::vector<triqs::utility::many_body_operator<double>>{}, bool use_quantum_numbers = false)",
             doc = """ """)

cthyb.add_method(name = "solve_parameters",
                 signature = "parameters()",
                 is_static = True,
                 doc = """Get the form of solve parameters""")

cthyb.add_property(name = "G0_iw",
                   getter = cfunction("block_gf_view<imfreq> G0_iw_view()"),
                   doc = "G0(iw) in imaginary frequencies")

cthyb.add_property(name = "Delta_tau",
                   getter = cfunction("block_gf_view<imtime> Delta_tau_view()"),
                   doc = "Delta(tau) in imaginary time")

cthyb.add_property(name = "G_tau",
                   getter = cfunction("block_gf_view<imtime> G_tau_view()"),
                   doc = "G(tau) in imaginary time")

cthyb.add_property(name = "G_l",
                   getter = cfunction("block_gf_view<legendre> G_l_view()"),
                   doc = "G_l in Legendre polynomials representation")

# generate the module code
module.generate_code()