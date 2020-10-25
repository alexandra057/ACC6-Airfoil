#include "EntropyViscosity.h"
#include <dolfin/common/ArrayView.h>

using namespace dolfin;

void compute_entropy_viscosity(Mesh& mesh, Function& res, Function& u, Function& mu)
{
  double Cm = 0.25;
  double Ce = 1.0;

  std::vector<dolfin::la_index> res_dofs = res.function_space()->dofmap()->dofs();
  std::vector<dolfin::la_index> u_dofs = u.function_space()->dofmap()->dofs();
  std::vector<dolfin::la_index> mu_dofs = mu.function_space()->dofmap()->dofs();

  std::vector<double> res_arr(res.vector()->local_size());
  std::vector<double> u_arr(u.vector()->local_size());
  std::vector<double> mu_arr(mu.vector()->local_size());

  res.vector()->get_local(res_arr);
  u.vector()->get_local(u_arr);
  
  Function u0(u[0]);
  Function u1(u[1]);
  double u0_avg = u0.vector()->sum();
  double u1_avg = u1.vector()->sum();
  u0_avg /= u0.vector()->size();
  u1_avg /= u1.vector()->size();
  *u0.vector() -= u0_avg;
  *u1.vector() -= u1_avg;

  double u0_inf = u0.vector()->norm("linf");
  double u1_inf = u1.vector()->norm("linf");
  double u_inf = sqrt(u0_inf * u0_inf + u1_inf * u1_inf);

  const GenericDofMap& dofmap_u = *u.function_space()->dofmap();
  std::size_t dofsize_u = dofmap_u.max_cell_dimension();
  ufc::cell ufc_cell;
  std::vector<double> vertex_coordinates0;

  int nsdim = mesh.topology().dim();
  dolfin::la_index local_size = (dolfin::la_index)((double)dofsize_u/(double)nsdim);

  for (CellIterator cell(mesh); !cell.end(); ++cell)
    {
      uint cid = (*cell).index();
      double vloc_K = 0.0;

      cell->get_vertex_coordinates(vertex_coordinates0);
      cell->get_cell_data(ufc_cell);
      auto cell_dofs_u = dofmap_u.cell_dofs((*cell).index());

      for (dolfin::la_index i = 0; i < local_size; i++)
	{
	  double normvel = 0.;
	  for (uint j = 0; j < nsdim; j++)
	    {
	      double velval = 0.;
	      velval   = u_arr[cell_dofs_u[i + local_size*j]];
	      normvel += velval * velval;
	    }

	  vloc_K = std::max(vloc_K, sqrt(normvel));
	}
      
      double h = 1000.;
      for(EdgeIterator ed(*cell); !ed.end(); ++ed)
        h = std::min(h, (*ed).length());
      
      mu_arr[cid] = std::min(Cm * (vloc_K + 1.0) * h, 
      			     Ce * h*h * fabs(res_arr[cid]) / (u_inf + 1e-6)); 
    }
  mu.vector()->set_local(mu_arr);
};
