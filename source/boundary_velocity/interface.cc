/*
  Copyright (C) 2011 - 2016 by the authors of the ASPECT code.

  This file is part of ASPECT.

  ASPECT is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  ASPECT is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ASPECT; see the file LICENSE.  If not see
  <http://www.gnu.org/licenses/>.
*/


#include <aspect/global.h>
#include <aspect/boundary_velocity/interface.h>
#include <aspect/simulator_access.h>

#include <deal.II/base/exceptions.h>
#include <deal.II/base/std_cxx11/tuple.h>

#include <list>


namespace aspect
{
  namespace BoundaryVelocity
  {
    template <int dim>
    Interface<dim>::~Interface ()
    {}


    template <int dim>
    void
    Interface<dim>::initialize ()
    {}



    template <int dim>
    void
    Interface<dim>::update ()
    {}



    template <int dim>
    Tensor<1,dim>
    Interface<dim>::boundary_velocity (const Point<dim> &) const
    {
      /**
       * We can only get here if the new-style boundary_velocity function (with
       * two arguments) calls it. This means that the derived class did not override
       * the new-style boundary_velocity function, and because we are here, it also
       * did not override this old-style boundary_velocity function (with one argument).
       */
      Assert (false, ExcMessage ("A derived class needs to override either the "
                                 "boundary_velocity(position) (deprecated) or "
                                 "boundary_velocity(types::boundary_id,position) "
                                 "function."));

      return Tensor<1,dim>();
    }


    DEAL_II_DISABLE_EXTRA_DIAGNOSTICS
    template <int dim>
    Tensor<1,dim>
    Interface<dim>::boundary_velocity (const types::boundary_id ,
                                       const Point<dim> &position) const
    {
      // Call the old-style function without the boundary id to maintain backwards
      // compatibility. Normally the derived class should override this function.
      return this->boundary_velocity(position);
    }
    DEAL_II_ENABLE_EXTRA_DIAGNOSTICS


    template <int dim>
    void
    Interface<dim>::
    declare_parameters (dealii::ParameterHandler &)
    {}


    template <int dim>
    void
    Interface<dim>::parse_parameters (dealii::ParameterHandler &)
    {}


// -------------------------------- Deal with registering boundary_velocity models and automating
// -------------------------------- their setup and selection at run time

    namespace
    {
      std_cxx11::tuple
      <void *,
      void *,
      aspect::internal::Plugins::PluginList<Interface<2> >,
      aspect::internal::Plugins::PluginList<Interface<3> > > registered_plugins;
    }



    template <int dim>
    void
    register_boundary_velocity (const std::string &name,
                                const std::string &description,
                                void (*declare_parameters_function) (ParameterHandler &),
                                Interface<dim> *(*factory_function) ())
    {
      std_cxx11::get<dim>(registered_plugins).register_plugin (name,
                                                               description,
                                                               declare_parameters_function,
                                                               factory_function);
    }


    template <int dim>
    Interface<dim> *
    create_boundary_velocity (const std::string &name)
    {
      Interface<dim> *plugin = std_cxx11::get<dim>(registered_plugins).create_plugin (name,
                                                                                      "Boundary velocity model::Plugin name");
      return plugin;
    }



    template <int dim>
    std::string
    get_names ()
    {
      return std_cxx11::get<dim>(registered_plugins).get_pattern_of_names ();
    }


    template <int dim>
    void
    declare_parameters (ParameterHandler &prm)
    {
      std_cxx11::get<dim>(registered_plugins).declare_parameters (prm);
    }



    template <int dim>
    void
    write_plugin_graph (std::ostream &out)
    {
      std_cxx11::get<dim>(registered_plugins).write_plugin_graph ("Boundary velocity interface",
                                                                  out);
    }
  }
}

// explicit instantiations
namespace aspect
{
  namespace internal
  {
    namespace Plugins
    {
      template <>
      std::list<internal::Plugins::PluginList<BoundaryVelocity::Interface<2> >::PluginInfo> *
      internal::Plugins::PluginList<BoundaryVelocity::Interface<2> >::plugins = 0;
      template <>
      std::list<internal::Plugins::PluginList<BoundaryVelocity::Interface<3> >::PluginInfo> *
      internal::Plugins::PluginList<BoundaryVelocity::Interface<3> >::plugins = 0;
    }
  }

  namespace BoundaryVelocity
  {
#define INSTANTIATE(dim) \
  template class Interface<dim>; \
  \
  template \
  void \
  register_boundary_velocity<dim> (const std::string &, \
                                   const std::string &, \
                                   void ( *) (ParameterHandler &), \
                                   Interface<dim> *( *) ()); \
  \
  template  \
  void \
  declare_parameters<dim> (ParameterHandler &); \
  \
  template \
  void \
  write_plugin_graph<dim> (std::ostream &); \
  \
  template  \
  std::string \
  get_names<dim> (); \
  \
  template \
  Interface<dim> * \
  create_boundary_velocity<dim> (const std::string &);

    ASPECT_INSTANTIATE(INSTANTIATE)
  }
}
