/* -------------------------------------------------------------------
 * LHOTSE: Toolbox for adaptive statistical models
 * -------------------------------------------------------------------
 * Project source file
 * Module: eptools
 * Desc.:  Header class EPPotLaplace
 * ------------------------------------------------------------------- */

#ifndef EPTOOLS_EPPOTLAPLACE_H
#define EPTOOLS_EPPOTLAPLACE_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "src/eptools/potentials/EPPotQuantileRegress.h"
#include "src/eptools/potentials/quad/QuadPotProximal.h"

//BEGINNS(eptools)
  /**
   * Laplace (double exponential) potential:
   *   t(s)  = (tau/2) exp( -tau |y-s| )
   * Parameters: y, tau>0.
   * <p>
   * NOTE: Special case of 'EPPotQuantileRegress', just calls static method
   * there.
   * We implement 'QuadPotProximal' here in order to support debugging
   * quadrature code.
   *
   * @author  Matthias Seeger
   * @version %I% %G%
   */
  class EPPotLaplace : public EPScalarPotential,public QuadPotProximal
  {
  protected:
    // Members

    double yscal,tau;

  public:
    // Public methods

    EPPotLaplace(double py=0.0,double ptau=1.0) {
      setY(py); setTau(ptau);
    }

    virtual double getTau() const {
      return tau;
    }

    virtual void setTau(double ptau) {
      if (ptau<1e-12)
	throw InvalidParameterException(EXCEPT_MSG(""));
      tau=ptau;
    }

    virtual double getY() const {
      return yscal;
    }

    virtual void setY(double py) {
      yscal=py;
    }

    int numPars() const {
      return 2;
    }

    void getPars(double* pv) const {
      pv[0]=getY(); pv[1]=getTau();
    }

    void setPars(const double* pv) {
      setY(pv[0]); setTau(pv[1]);
    }

    bool isValidPars(const double* pv) const {
      return (pv[1]>=1e-12);
    }

    bool suppFractional() const {
      return true;
    }

    bool isLogConcave() const {
      return true;
    }

    /*
     * t(s)^eta corresponds to C times 'EPPotQuantileRegress' with
     *   kappa = 1/2, xi = 2 eta tau, C = (tau/2)^eta.
     */
    bool compMoments(const double* inp,double* ret,double* logz=0,
		     double eta=1.0) const {
      double cmu=inp[0],crho=inp[1];

      if (crho<1e-14 || eta<1e-10 || eta>1.0)
	throw InvalidParameterException(EXCEPT_MSG(""));
      bool rstat =
	EPPotQuantileRegress::compMomentsInt(cmu,crho,2.0*eta*tau,yscal,0.5,
					     ret[0],ret[1],logz);
      if (rstat && logz!=0)
	(*logz) += eta*log(0.5*tau);

      return rstat;
    }

    // 'QuadPotProximal' methods

    bool hasFirstDerivatives() const {
      return true;
    }

    bool hasSecondDerivatives() const {
      return true;
    }

    bool hasWayPoints() const {
      return true;
    }

    /**
     * The integration interval is all of R. y is a waypoint, l(s) is not
     * differentiable there.
     */
    void getInterval(double& a,bool& aInf,double& b,bool& bInf,
		     ArrayHandle<double>& wayPts) const {
      aInf=bInf=true;
      if (wayPts.size()!=1)
	wayPts.changeRep(1);
      wayPts[0]=yscal;
    }

    double eval(double s,double* dl=0,double* ddl=0) const {
      double ret;
      // NOTE: Does not complain if s==yscal, treats it like s>yscal
      if (s>=yscal) {
	ret=tau*(s-yscal)-log(0.5*tau);
	if (dl!=0) *dl=tau;
      } else {
	ret=tau*(yscal-s)-log(0.5*tau);
	if (dl!=0) *dl=-tau;
      }
      if (ddl!=0) *ddl=0.0;

      return ret;
    }

    /*
     * This is the usual l_1 proximal map:
     * If x = s-y: argmin_x kappa |x| + 0.5 (x-mu)^2,
     * kappa = rho*tau, mu = h-y.
     * The solution x_* is soft shrinkage of mu by kappa.
     * NOTE: This maps s_* = y for all h close to y, so we sit on the
     * waypoint then (where l(s) is not differentiable).
     */
    bool proximal(double h,double rho,double& sstar) const {
      double mu=h-yscal,kap=rho*tau;

      sstar=yscal+((mu>kap)?(mu-kap):((mu<-kap)?(mu+kap):0.0));
      return true;
    }
  };
//ENDNS

#endif
