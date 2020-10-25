#include <dolfin.h>
#include "src/Momentum.h"
#include "src/Continuity.h"
#include "src/EntropyResidual.h"
#include "src/EntropyViscosity.h"
#include "src/Functionals.h"
#include <fstream>

using namespace dolfin;

double xmin = 0.0;
double xmax = 3.0;
double ymin = 0.0;
double ymax = 1.0;
double eps_ = 1e-8;

// Define noslip domain
class NoslipDomain : public SubDomain
{
  bool inside(const Array<double>& x, bool on_boundary) const
  {
    return (
	    on_boundary &&
	    pow(x[0], 2.) + pow(x[1], 2.) < pow(2., 2.) );
  }
};

// 
class Wing : public SubDomain
{
  bool inside(const Array<double>& x, bool on_boundary) const
  {
    return (
	    on_boundary &&
	    pow(x[0], 2.) + pow(x[1], 2.) < pow(2., 2.) );
  }
};

// Define inflow domain
class InflowDomain : public SubDomain
{
  bool inside(const Array<double>& x, bool on_boundary) const
  { 
    return  (
	     on_boundary && x[0] < eps_ &&
	     pow(x[0], 2.) + pow(x[1], 2.) > pow(2., 2.) );
  }
};

// Define outflow domain
class OutflowDomain : public SubDomain
{
  bool inside(const Array<double>& x, bool on_boundary) const
  { 
    return  (
	     on_boundary && x[0] > - eps_ &&
	     pow(x[0], 2.) + pow(x[1], 2.) > pow(2., 2.) );
  }
};

// Function for inflow boundary condition for velocity
class Inflow : public Expression
{
public:

  Inflow(double& speed) : speed(speed), Expression(2)  {}

  void eval(Array<double>& values, const Array<double>& x) const
  {
    values[0] = speed;
    values[1] = 0.0;
  }
private:
  double& speed;
};

int main(int argc, char* argv[])
{
  // Print log messages only from the root process in parallel
  parameters["std_out_all_processes"] = false;

  // Load mesh from file
  auto mesh = std::make_shared<Mesh>(argv[5]);

  // Create function spaces
  auto V = std::make_shared<Momentum::FunctionSpace>(mesh);
  auto Q = std::make_shared<Continuity::FunctionSpace>(mesh);
  auto W = std::make_shared<EntropyResidual::FunctionSpace>(mesh);

  // Set parameter values
  int num_samples = atoi(argv[1]);
  double visc = atof(argv[2]);
  double speed = atof(argv[3]);
  double T = atof(argv[4]);

  double CFL = 1.0;
  double dt = CFL * mesh->hmin();
  dt = T/(double)((uint)(T/dt)); 

  // Define values for boundary conditions
  auto u_in = std::make_shared<Inflow>(speed);
  auto zero = std::make_shared<Constant>(0);
  auto zero_vector = std::make_shared<Constant>(0, 0);

  // Define subdomains for boundary conditions
  auto noslip_domain = std::make_shared<NoslipDomain>();
  auto inflow_domain = std::make_shared<InflowDomain>();
  auto outflow_domain = std::make_shared<OutflowDomain>();

  // Define boundary conditions
  DirichletBC noslip(V, zero_vector, noslip_domain);
  DirichletBC inflow(V, u_in, inflow_domain);
  DirichletBC outflow(Q, zero, outflow_domain);

  // Collect boundary condition of u:
  std::vector<DirichletBC*> bcu;
  bcu.push_back(&noslip);
  bcu.push_back(&inflow);

  // Collect boundary condition of p:
  std::vector<DirichletBC*> bcp;
  bcp.push_back(&outflow);

  // Create functions
  auto u0 = std::make_shared<Function>(V);
  auto u = std::make_shared<Function>(V);
  auto p = std::make_shared<Function>(Q);
  auto p0 = std::make_shared<Function>(Q);
  auto res = std::make_shared<Function>(W);
  auto mu = std::make_shared<Function>(W);

  // Create coefficients
  auto k = std::make_shared<Constant>(dt);
  auto nu = std::make_shared<Constant>(visc);
  auto f = std::make_shared<Constant>(0, 0);

  // Create forms
  Momentum::BilinearForm amom(V, V);
  Momentum::LinearForm Lmom(V);
  Continuity::BilinearForm acon(Q, Q);
  Continuity::LinearForm Lcon(Q);
  EntropyResidual::LinearForm Lres(W);
  
  // Functionals for lift and drag
  Functionals::Form_lift L(mesh, p);
  Functionals::Form_drag D(mesh, p);
  // Mark cylinder
  //FacetFunction<std::size_t> markers(mesh, 1);
  //auto markers = std::make_shared<FacetFunction<std::size_t>>(mesh, 1);
  auto markers = std::make_shared<MeshFunction<std::size_t>>(mesh, mesh->topology().dim()-1, 1);

  Wing wing;
  
  wing.mark(*markers, 1);
  // Attach markers to functionals
  L.ds = markers;
  D.ds = markers;

  // Set coefficients
  amom.k = k; amom.u0 = u0; 
  amom.nu = nu; 
  amom.mu = mu; 
  Lmom.k = k; Lmom.u0 = u0; 
  Lmom.f = f; Lmom.p = p; 
  Lmom.nu = nu;
  Lmom.mu = mu;
  Lcon.k = k; Lcon.u = u;
  Lres.u = u; Lres.u0 = u0; 
  Lres.p = p; Lres.k = k;
  
  // Assemble matrices
  Matrix Amom, Acon;
  assemble(Amom, amom);
  assemble(Acon, acon);

  // Create vectors
  Vector bmom, bcon;

  // Create files for storing solution
  File ufile("results/velocity.pvd");
  File pfile("results/pressure.pvd");
  File resfile("results/residual.pvd");
  File mufile("results/mu.pvd");
  double t_save = 0.0;

  std::string res_fname = "results/drag_ligt.m";
  std::ofstream resFile;
  resFile.open(res_fname.c_str(), std::ios::out);
  resFile << "% time" << "\t"
  	  << "lift" << "\t"
     	  << "drag" << "\n";
  resFile.flush();

  set_log_active(false);
  // Time-stepping
  double t = dt;
  while (t < T + eps_)
  {
    tic();

    // Compute velocity
    assemble(Amom, amom);
    assemble(bmom, Lmom);
    for (std::size_t i = 0; i < bcu.size(); i++)
      bcu[i]->apply(Amom, bmom);
    solve(Amom, *u->vector(), bmom, "gmres", "ilu");

    // Pressure correction
    assemble(bcon, Lcon);
    for (std::size_t i = 0; i < bcp.size(); i++)
      bcp[i]->apply(Acon, bcon);
    solve(Acon, *p->vector(), bcon, "cg");

    // Compute artificial viscosity
    assemble(*res->vector(), Lres);
    compute_entropy_viscosity(*mesh, *res, *u, *mu);

    // Save to file
    t_save += dt;
    if (t_save > T/(double)(num_samples) || t >= T-dt)
      {
	ufile << *u;
	pfile << *p;
	// resfile << *res;
	// mufile << *mu;
	t_save = 0.0;
      }

    // Assemble functionals over sub domain
    const double lift = assemble(L);
    const double drag = assemble(D);
    resFile << t << "\t"
	    << lift << "\t"
	    << drag << "\n";
    resFile.flush();

    // Time-stepping monitor
    Function res_u(V), res_p(Q);
    *res_u.vector() = *u->vector();
    *res_u.vector() -= *u0->vector();

    *res_p.vector() = *p->vector();
    *res_p.vector() -= *p0->vector();

    set_log_active(true);
    info("l2(u) = %e, l2(p) = %e, k = %lf, t = %lf, iter_time = %f sec", 
	 res_u.vector()->norm("l2"), res_p.vector()->norm("l2"), dt, t, toc() );
    set_log_active(false);

    // Move to next time step
    *u0->vector() = *u->vector();
    *p0->vector() = *p->vector();
    t += dt;

  }
  
  resFile.close();
  return 0;
}

