#pragma once

#include "GlobalVariables.C"

#include <g4mvtx/PHG4MvtxDefs.h>
#include <g4mvtx/PHG4MvtxSubsystem.h>

#include <g4main/PHG4Reco.h>

#include <fun4all/Fun4AllServer.h>

#include <cmath>
#include <vector>

R__LOAD_LIBRARY(libg4mvtx.so)

namespace Enable
{
  static bool MVTX = false;
}

namespace MVTX
{
  const int n_maps_layer = 3;  // must be 0-3, setting it to zero removes Mvtx completely, n < 3 gives the first n layers
  const int N_staves[MVTX::n_maps_layer] = {18, 24, 30};
  const double nom_radius[MVTX::n_maps_layer] = {36.4, 48.1, 59.8};
}  // namespace MVTX

void MvtxInit(int verbosity = 0)
{
  BlackHoleGeometry::max_radius = std::max(BlackHoleGeometry::max_radius, MVTX::nom_radius[MVTX::n_maps_layer - 1] / 10. + 0.7);
  BlackHoleGeometry::max_z = std::max(BlackHoleGeometry::max_z, 16.);
  BlackHoleGeometry::min_z = std::min(BlackHoleGeometry::min_z, -17.);
}

double Mvtx(PHG4Reco* g4Reco, double radius,
            const int absorberactive = 0,
            int verbosity = 0)
{
  bool maps_overlapcheck = false;  // set to true if you want to check for overlaps

  // Update EIC MAPS layer structure based on inner two layers of U. Birmingham tracker

  PHG4MvtxSubsystem* mvtx = new PHG4MvtxSubsystem("MVTX");
  mvtx->Verbosity(verbosity);

  // H?kan Wennl?f <hwennlof@kth.se> :
  //    Without time-stamping layer:
  //    Stave type  Length  Overlap Radius [mm] Tilt  Radiation length  Number of staves
  //    ALICE inner 270 mm  2 mm  36.4  12.0 deg  0.3 % X0  18
  //    ALICE inner 270 mm  2 mm  59.8  12.0 deg  0.3 % X0  30
  //    ALICE outer 840 mm  4 mm  133.8 6.0 deg 0.8 % X0  16
  //    ALICE outer 840 mm  4 mm  180 6.0 deg 0.8 % X0  21

  //    int N_staves[MVTX::n_maps_layer] = {18, 24, 30};
  //    double nom_radius[MVTX::n_maps_layer] = {36.4, 48.1,  59.8};
  for (int ilyr = 0; ilyr < MVTX::n_maps_layer; ilyr++)
  {
    mvtx->set_int_param(ilyr, "active", 1);  //non-automatic initialization in PHG4DetectorGroupSubsystem
    mvtx->set_int_param(ilyr, "layer", ilyr);
    mvtx->set_int_param(ilyr, "N_staves", MVTX::N_staves[ilyr]);
    mvtx->set_double_param(ilyr, "layer_nominal_radius", MVTX::nom_radius[ilyr]);  // mm
    mvtx->set_double_param(ilyr, "phitilt", 12.0 * 180. / M_PI + M_PI);
    mvtx->set_double_param(ilyr, "phi0", 0);
  }

  mvtx->set_string_param(PHG4MvtxDefs::GLOBAL, "stave_geometry_file", string(getenv("CALIBRATIONROOT")) + string("/Tracking/geometry/mvtx_stave_v1.gdml"));
  mvtx->SetActive(1);
  mvtx->OverlapCheck(maps_overlapcheck);
  g4Reco->registerSubsystem(mvtx);
  return MVTX::nom_radius[MVTX::n_maps_layer - 1] / 10.;  // return cm
}

// Central detector cell reco is disabled as EIC setup use the fast tracking sim for now
void Svtx_Cells(int verbosity = 0)
{
  // runs the cellularization of the energy deposits (g4hits)
  // into detector hits (g4cells)

  //---------------
  // Load libraries
  //---------------

  gSystem->Load("libfun4all.so");
  gSystem->Load("libg4detectors.so");

  //---------------
  // Fun4All server
  //---------------

  Fun4AllServer* se = Fun4AllServer::instance();

  //-----------
  // SVTX cells
  //-----------

  return;
}

// Central detector  reco is disabled as EIC setup use the fast tracking sim for now
void Svtx_Reco(int verbosity = 0)
{
  //---------------
  // Load libraries
  //---------------

  gSystem->Load("libfun4all.so");

  //---------------
  // Fun4All server
  //---------------

  Fun4AllServer* se = Fun4AllServer::instance();

  return;
}
