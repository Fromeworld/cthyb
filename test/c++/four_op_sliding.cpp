#include <triqs/test_tools/arrays.hpp> // FIXME move definition of EXPECT_CLOSE somehwere else?
#include "./measure_four_body_corr.hpp"

using namespace cthyb;

int main() {

// template <class T1, class T2> 
// void assert_close(T1 const& A, T2 const& B, double precision) {
//  if (std::abs(A - B) > precision) TRIQS_RUNTIME_ERROR << "assert_close error : " << A << "\n" << B;
// }
// const double PRECISION = 1.e-6;

 // define lambdas
 double lamb1 = -1.1;
 double lamb2 = 0.001;
 double lamb3 = 0.5;
 double lamb4 = -2.3;
 double lamb5 = 5.9;
 double dt = 0.5;

 // n = 1 tests
 EXPECT_CLOSE(measure_four_body_corr::compute_evolution_integral(lamb1, lamb2), -0.6068387070559753 )
 EXPECT_CLOSE(measure_four_body_corr::compute_evolution_integral(lamb1, lamb1), -0.33287108369807955)

 EXPECT_CLOSE(-dt * measure_four_body_corr::compute_evolution_integral(dt * lamb1, dt * lamb2), 0.6664422278730481)
 EXPECT_CLOSE(-dt * measure_four_body_corr::compute_evolution_integral(dt * lamb1, dt * lamb1), 0.8666265089336976)

 // n = 2 tests
 EXPECT_CLOSE(measure_four_body_corr::compute_evolution_integral(lamb1, lamb2, lamb3), 0.43199931827716453)
 EXPECT_CLOSE(measure_four_body_corr::compute_evolution_integral(lamb1, lamb1, lamb2), 0.2488352619054457 )
 EXPECT_CLOSE(measure_four_body_corr::compute_evolution_integral(lamb1, lamb1, lamb1), 0.16643554184903978)

 dt2 = dt * dt;
 EXPECT_CLOSE(dt2 * measure_four_body_corr::compute_evolution_integral(dt * lamb1, dt * lamb2, dt * lamb3), 0.14009936097305434)
 EXPECT_CLOSE(dt2 * measure_four_body_corr::compute_evolution_integral(dt * lamb1, dt * lamb1, dt * lamb2), 0.18182041876534927)
 EXPECT_CLOSE(dt2 * measure_four_body_corr::compute_evolution_integral(dt * lamb1, dt * lamb1, dt * lamb1), 0.2166566272334244 )

 // n = 4 tests
 EXPECT_CLOSE(measure_four_body_corr::compute_evolution_integral(lamb1, lamb2, lamb3, lamb4, lamb5), 0.17638333857571462 )
 EXPECT_CLOSE(measure_four_body_corr::compute_evolution_integral(lamb1, lamb1, lamb2, lamb3, lamb4), 0.020244899523797544)
 EXPECT_CLOSE(measure_four_body_corr::compute_evolution_integral(lamb1, lamb1, lamb2, lamb2, lamb5), 0.030709162276787143)
 EXPECT_CLOSE(measure_four_body_corr::compute_evolution_integral(lamb1, lamb1, lamb1, lamb2, lamb3), 0.0247729599423155  )
 EXPECT_CLOSE(measure_four_body_corr::compute_evolution_integral(lamb1, lamb1, lamb1, lamb2, lamb2), 0.022082528548217618)
 EXPECT_CLOSE(measure_four_body_corr::compute_evolution_integral(lamb1, lamb1, lamb1, lamb1, lamb2), 0.017586090258800525)
 EXPECT_CLOSE(measure_four_body_corr::compute_evolution_integral(lamb1, lamb1, lamb1, lamb1, lamb1), 0.013869628487419981)

 dt4 = dt2 * dt2;
 EXPECT_CLOSE(dt4 * measure_four_body_corr::compute_evolution_integral(dt * lamb1, dt * lamb2, dt * lamb3, dt * lamb4, dt * lamb5), 0.002228035849416145)
 EXPECT_CLOSE(dt4 * measure_four_body_corr::compute_evolution_integral(dt * lamb1, dt * lamb1, dt * lamb2, dt * lamb3, dt * lamb4), 0.003962922812390968)
 EXPECT_CLOSE(dt4 * measure_four_body_corr::compute_evolution_integral(dt * lamb1, dt * lamb1, dt * lamb2, dt * lamb2, dt * lamb5), 0.0031132577613928645)
 EXPECT_CLOSE(dt4 * measure_four_body_corr::compute_evolution_integral(dt * lamb1, dt * lamb1, dt * lamb1, dt * lamb2, dt * lamb3), 0.003478034499745782)
 EXPECT_CLOSE(dt4 * measure_four_body_corr::compute_evolution_integral(dt * lamb1, dt * lamb1, dt * lamb1, dt * lamb2, dt * lamb2), 0.00364336019678325)
 EXPECT_CLOSE(dt4 * measure_four_body_corr::compute_evolution_integral(dt * lamb1, dt * lamb1, dt * lamb1, dt * lamb1, dt * lamb2), 0.004058965987701827)
 EXPECT_CLOSE(dt4 * measure_four_body_corr::compute_evolution_integral(dt * lamb1, dt * lamb1, dt * lamb1, dt * lamb1, dt * lamb1), 0.004513679734029675)
}
