#pragma once

#include "GlobalVariables.C"

#include <g4calo/RawTowerBuilderByHitIndex.h>
#include <g4calo/RawTowerDigitizer.h>
#include <g4detectors/PHG4CrystalCalorimeterSubsystem.h>
#include <g4detectors/PHG4ForwardCalCellReco.h>

#include <g4eval/CaloEvaluator.h>

#include <g4main/PHG4Reco.h>

#include <caloreco/RawClusterBuilderFwd.h>
#include <caloreco/RawTowerCalibration.h>

#include <fun4all/Fun4AllServer.h>

R__LOAD_LIBRARY(libcalo_reco.so)
R__LOAD_LIBRARY(libg4calo.so)
R__LOAD_LIBRARY(libg4detectors.so)
R__LOAD_LIBRARY(libg4eval.so)

namespace Enable
{
  static bool EEMC = false;
}

namespace G4EEMC
{
  const int use_projective_geometry = 0;
  const double Gdz = 18. + 0.0001;
  const double Gz0 = -170.;
}  // namespace G4EEMC

void EEMCInit()
{
  BlackHoleGeometry::max_radius = std::max(BlackHoleGeometry::max_radius, 65.6);  // from towerMap_EEMC_v006.txt
  BlackHoleGeometry::min_z = std::min(BlackHoleGeometry::min_z, G4EEMC::Gz0 - G4EEMC::Gdz / 2.);
}

void EEMCSetup(PHG4Reco *g4Reco, const int absorberactive = 0)
{
  /** Use dedicated EEMC module */
  PHG4CrystalCalorimeterSubsystem *eemc = new PHG4CrystalCalorimeterSubsystem("EEMC");

  /* path to central copy of calibrations repositry */
  ostringstream mapping_eemc;

  /* Use non-projective geometry */
  if (!G4EEMC::use_projective_geometry)
  {
    mapping_eemc << getenv("CALIBRATIONROOT") << "/CrystalCalorimeter/mapping/towerMap_EEMC_v006.txt";
    eemc->SetTowerMappingFile(mapping_eemc.str());
  }

  /* use projective geometry */
  else
  {
    ostringstream mapping_eemc_4x4construct;

    mapping_eemc << getenv("CALIBRATIONROOT") << "/CrystalCalorimeter/mapping/crystals_v005.txt";
    mapping_eemc_4x4construct << getenv("CALIBRATIONROOT") << "/CrystalCalorimeter/mapping/4_by_4_construction_v005.txt";
    eemc->SetProjectiveGeometry(mapping_eemc.str(), mapping_eemc_4x4construct.str());
  }

  eemc->OverlapCheck(overlapcheck);

  // SetAbsorberActive method not implemented
  //  if (absorberactive)  eemc->SetAbsorberActive();

  /* register Ecal module */
  g4Reco->registerSubsystem(eemc);
}

void EEMC_Cells(int verbosity = 0)
{
  Fun4AllServer *se = Fun4AllServer::instance();

  PHG4ForwardCalCellReco *hc = new PHG4ForwardCalCellReco("EEMCCellReco");
  hc->Detector("EEMC");
  se->registerSubsystem(hc);

  return;
}

void EEMC_Towers(int verbosity = 0)
{
  Fun4AllServer *se = Fun4AllServer::instance();

  ostringstream mapping_eemc;
  mapping_eemc << getenv("CALIBRATIONROOT") << "/CrystalCalorimeter/mapping/towerMap_EEMC_v006.txt";

  RawTowerBuilderByHitIndex *tower_EEMC = new RawTowerBuilderByHitIndex("TowerBuilder_EEMC");
  tower_EEMC->Detector("EEMC");
  tower_EEMC->set_sim_tower_node_prefix("SIM");
  tower_EEMC->GeometryTableFile(mapping_eemc.str());

  se->registerSubsystem(tower_EEMC);

  /* Calorimeter digitization */

  // CMS lead tungstate barrel ECAL at 18 degree centrigrade: 4.5 photoelectrons per MeV
  const double EEMC_photoelectron_per_GeV = 4500;

  RawTowerDigitizer *TowerDigitizer_EEMC = new RawTowerDigitizer("EEMCRawTowerDigitizer");
  TowerDigitizer_EEMC->Detector("EEMC");
  TowerDigitizer_EEMC->Verbosity(verbosity);
  TowerDigitizer_EEMC->set_raw_tower_node_prefix("RAW");
  TowerDigitizer_EEMC->set_digi_algorithm(RawTowerDigitizer::kSimple_photon_digitization);
  TowerDigitizer_EEMC->set_pedstal_central_ADC(0);
  TowerDigitizer_EEMC->set_pedstal_width_ADC(8);  // eRD1 test beam setting
  TowerDigitizer_EEMC->set_photonelec_ADC(1);     //not simulating ADC discretization error
  TowerDigitizer_EEMC->set_photonelec_yield_visible_GeV(EEMC_photoelectron_per_GeV);
  TowerDigitizer_EEMC->set_zero_suppression_ADC(16);  // eRD1 test beam setting

  se->registerSubsystem(TowerDigitizer_EEMC);

  /* Calorimeter calibration */

  RawTowerCalibration *TowerCalibration_EEMC = new RawTowerCalibration("EEMCRawTowerCalibration");
  TowerCalibration_EEMC->Detector("EEMC");
  TowerCalibration_EEMC->Verbosity(verbosity);
  TowerCalibration_EEMC->set_calib_algorithm(RawTowerCalibration::kSimple_linear_calibration);
  TowerCalibration_EEMC->set_calib_const_GeV_ADC(1. / EEMC_photoelectron_per_GeV);
  TowerCalibration_EEMC->set_pedstal_ADC(0);

  se->registerSubsystem(TowerCalibration_EEMC);
}

void EEMC_Clusters(int verbosity = 0)
{
  Fun4AllServer *se = Fun4AllServer::instance();

  RawClusterBuilderFwd *ClusterBuilder = new RawClusterBuilderFwd("EEMCRawClusterBuilderFwd");
  ClusterBuilder->Detector("EEMC");
  ClusterBuilder->Verbosity(verbosity);
  se->registerSubsystem(ClusterBuilder);

  return;
}

void EEMC_Eval(std::string outputfile, int verbosity = 0)
{
  Fun4AllServer *se = Fun4AllServer::instance();

  CaloEvaluator *eval = new CaloEvaluator("EEMCEVALUATOR", "EEMC", outputfile.c_str());
  eval->Verbosity(verbosity);
  se->registerSubsystem(eval);

  return;
}
