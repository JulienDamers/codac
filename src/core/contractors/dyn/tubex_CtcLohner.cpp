/** 
 *  CtcLohner class
 * ----------------------------------------------------------------------------
 *  \date       2020
 *  \author     Auguste Bourgois
 *  \copyright  Copyright 2020 Tubex Team
 *  \license    This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 */

#include "tubex_CtcLohner.h"

#include <tubex_CtcLohner.h>
#include <Eigen/QR>
#include <ibex.h>
#include <tubex_Eigen.h>
#include <tubex_DomainsTypeException.h>


using namespace ibex;
using namespace std;

namespace tubex {

// todo: to be placed in header file

/**
 * \class GlobalEnclosureError
 *
 * \brief Encapsulates runtime error for global enclosure estimation failure
 */
struct GlobalEnclosureError : public std::runtime_error {
  GlobalEnclosureError() : std::runtime_error(
      "Exceeded loop maximum range while looking for a global enclosure for the system.") {}
};

/**
 * \class LohnerAlgorithm
 *
 * \brief Simple first order Lohner algorithm to perform guaranteed integration of a system
 * \f$\dot{\mathbf{x}}=\mathbf{f}(\mathbf{x})\f$
 */
class LohnerAlgorithm {
public:
  constexpr static const double FWD = 1, BWD = -1;

  /**
   * \brief Creates a Lohner algorithm object
   *
   * \param f function defining the system \f$\dot{\mathbf{x}}=\mathbf{f}(\mathbf{x})\f$
   * \param h time step of the integration method
   * \param u0 initial condition of the system
   * \param contractions number of contractions of the global enclosure by the estimated local enclosure
   * \param eps inflation parameter for the global enclosure
   */
  LohnerAlgorithm(const ibex::Function *f,
                  double h,
                  const ibex::IntervalVector &u0 = ibex::IntervalVector::empty(1),
                  int contractions = 1,
                  double eps = 0.1);
  /**
   * \brief integrate the system over a given number of steps
   *
   * \param steps number of steps to integrate
   * \param H parameter to overwrite the integration time step
   * \return enclosure of the system's state
   */
  const ibex::IntervalVector &integrate(unsigned int steps, double H = -1);

  /**
   * \brief contract the global & local enclosure of the previous integration step
   *
   * \param x estimation of the global enclosure
   */
  void contractStep(const ibex::IntervalVector &x);

  /**
   * \brief Returns the current global enclosure, i.e. the box enclosing the trajectories between times \f$k-1\f$ and
   * \f$k\f$
   *
   * \return global enclosure
   */
  const ibex::IntervalVector &getLocalEnclosure() const;

  /**
   * \brief Returns the current local enclosure, i.e. the box enclosing the trajectories at time \f$k\f$
   *
   * \return local enclosure \f$[\mathbf{x}_{k}]\f$
   */
  const ibex::IntervalVector &getGlobalEnclosure() const;
private:
  /**
   * \brief Computes an estimation of the global enclosure of the system
   *
   * \param initialGuess initial guess for the global enclosure
   * \param direction forward or backward in time
   * \return estimation of the global enclosure
   */
  ibex::IntervalVector globalEnclosure(const ibex::IntervalVector &initialGuess, double direction);

  unsigned int dim; //!< dimension of the system's state
  double h; //!< integration time step
  double eps; //!< inflation parameter for the global enclosure
  int contractions; //!< number of contractions of the global enclosure by the estimated local enclosure
  ibex::IntervalVector u; //!< local enclosure
  ibex::IntervalVector z; //!< Taylor-Lagrange remainder (order 2)
  ibex::IntervalVector r; //!< enclosure of uncertainties in the frame given by the matrix B
  ibex::IntervalVector u_tilde; //!< global enclosure
  ibex::Matrix B, Binv;
  ibex::Vector u_hat; //!< center of the box u
  const ibex::Function *f; //!< litteral function of the system \f$\dot{\mathbf{x}}=\mathbf{f}(\mathbf{x})\f$
};

// --

LohnerAlgorithm::LohnerAlgorithm(const ibex::Function *f,
                                 double h,
                                 const ibex::IntervalVector &u0,
                                 int contractions,
                                 double eps)
    : dim(f->nb_var()),
      h(h),
      eps(eps),
      contractions(contractions),
      u(u0),
      z(u0 - u0.mid()),
      r(z),
      B(ibex::Matrix::eye(dim)),
      Binv(ibex::Matrix::eye(dim)),
      u_hat(u0.mid()),
      f(f) {}

const ibex::IntervalVector &LohnerAlgorithm::integrate(unsigned int steps, double H) {
  if (H > 0) h = H;
  for (unsigned int i = 0; i < steps; ++i) {
    auto z1 = z, r1 = r, u1 = u;
    auto B1 = B, B1inv = Binv;
    auto u_hat1 = u_hat;
    ibex::IntervalVector u_t = globalEnclosure(u, FWD);
    for (int j = 0; j < contractions; ++j) {
      z1 = 0.5 * h * h * f->jacobian(u_t) * f->eval_vector(u_t);
      ibex::Vector m1 = z1.mid();
      ibex::IntervalMatrix A = ibex::Matrix::eye(dim) + h * f->jacobian(u);
      Eigen::HouseholderQR<Eigen::MatrixXd> qr(EigenHelpers::i2e((A * B).mid()));
      B1 = EigenHelpers::e2i(qr.householderQ());
      B1inv = ibex::real_inverse(B1);
      r1 = (B1inv * A * B) * r + B1inv * (z1 - m1);
      u_hat1 = u_hat + h * f->eval_vector(u_hat).mid() + m1;
      u1 = u_hat1 + B1 * r1;
      if (j < contractions - 1) {
        u_t = u_t & globalEnclosure(u1, BWD);
      }
    }
    z = z1, r = r1, u = u1, B = B1, Binv = B1inv, u_hat = u_hat1, u_tilde = u_t;
  }
  return u;
}

ibex::IntervalVector LohnerAlgorithm::globalEnclosure(const ibex::IntervalVector &initialGuess, double direction) {
  ibex::IntervalVector u_0 = initialGuess;
  for (unsigned int i = 0; i < 30; ++i) {
    ibex::IntervalVector u_1 = initialGuess + direction * ibex::Interval(0, h) * f->eval_vector(u_0);
    if (u_0.is_superset(u_1)) {
      return u_0;
    } else {
      u_0 = (1 + eps) * u_1 - eps * u_1;
    }
  }
  throw GlobalEnclosureError();
}

void LohnerAlgorithm::contractStep(const ibex::IntervalVector &x) {
  u = x & u;
  u_hat = u.mid();
  r = r & (Binv * (u - u_hat));
}

const ibex::IntervalVector &LohnerAlgorithm::getLocalEnclosure() const {
  return u;
}

const ibex::IntervalVector &LohnerAlgorithm::getGlobalEnclosure() const {
  return u_tilde;
}

CtcLohner::CtcLohner(const ibex::Function &f, int contractions, double eps)
    : DynCtc(),
      _x_(ExprSymbol::new_(Dim::col_vec(f.nb_var()))),
      m_f(f),
      m_f_1(_x_, -m_f(_x_)),
      contractions(contractions),
      dim(f.nb_var()),
      eps(eps) {}

void CtcLohner::contract(tubex::TubeVector &tube, TimePropag t_propa) {
  assert((!tube.is_empty())&&(tube.size() == dim));
  IntervalVector input_gate(dim, Interval(0)), output_gate(dim, Interval(0)), slice(dim, Interval(0));
  double h;
  if (t_propa & TimePropag::FORWARD) {
    for (int j = 0; j < dim; ++j) {
      input_gate[j] = tube[j].slice(0)->input_gate();
    }
    LohnerAlgorithm lo(&m_f, 0.1, input_gate, contractions, eps);
    // Forward loop
    for (int i = 0; i < tube.nb_slices(); ++i) {
      h = tube[0].slice(i)->tdomain().diam();
      for (int j = 0; j < dim; ++j) {
        output_gate[j] = tube[j].slice(i)->output_gate();
        slice[j] = tube[j].slice(i)->codomain();
      }
      lo.integrate(1, h);
      lo.contractStep(output_gate);
      tube.set(slice & lo.getGlobalEnclosure(), i);
    }
    tube.set(output_gate & lo.getLocalEnclosure(), tube.tdomain().ub());
  }
  if (t_propa & TimePropag::BACKWARD) {
    for (int j = 0; j < dim; ++j) {
      input_gate[j] = tube[j].last_slice()->output_gate();
    }
    LohnerAlgorithm lo2(&m_f_1, 0.1, input_gate, contractions, eps);
    // Backward loop
    for (int i = tube.nb_slices() - 1; i >= 0; --i) {
      h = tube[0].slice(i)->tdomain().diam();
      for (int j = 0; j < dim; ++j) {
        output_gate[j] = tube[j].slice(i)->input_gate();
        slice[j] = tube[j].slice(i)->codomain();
      }
      lo2.integrate(1, h);
      lo2.contractStep(output_gate);
      tube.set(slice & lo2.getGlobalEnclosure(), i); // tube update
    }
    tube.set(output_gate & lo2.getLocalEnclosure(), tube.tdomain().lb());
  }
}

void CtcLohner::contract(tubex::Tube &tube, TimePropag t_propa) {
  assert(not tube.is_empty());
  tubex::TubeVector tubeVector(1, tube);
  contract(tubeVector, t_propa);
  tube = tubeVector[0];
}

// Static members for contractor signature (mainly used for CN Exceptions)
const string CtcLohner::m_ctc_name = "CtcLohner";
vector<string> CtcLohner::m_str_expected_doms(
    {
        "Tube",
        "TubeVector",
    });

void CtcLohner::contract(vector<Domain *> &v_domains) {

  // todo: manage the contractor on a slice level
  
  if (v_domains.size() != 1)
    throw DomainsTypeException(m_ctc_name, v_domains, m_str_expected_doms);

  if (v_domains[0]->type() == Domain::Type::T_TUBE)
    contract(v_domains[0]->tube(), TimePropag::FORWARD | TimePropag::BACKWARD);

  else if (v_domains[0]->type() == Domain::Type::T_TUBE_VECTOR)
    contract(v_domains[0]->tube_vector(), TimePropag::FORWARD | TimePropag::BACKWARD);

  else
    throw DomainsTypeException(m_ctc_name, v_domains, m_str_expected_doms);
}

} // namespace tubex