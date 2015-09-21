/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Manuel Requena <manuel.requena@cttc.es>
 */


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>
//#include "ns3/gtk-config-store.h"
#include "ns3/config-store.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // to save a template default attribute file run it like this:
  // ./waf --command-template="%s --ns3::ConfigStore::Filename=input-defaults.txt --ns3::ConfigStore::Mode=Save --ns3::ConfigStore::FileFormat=RawText" --run src/lte/examples/lena-first-sim
  //
  // to load a previously created default attribute file
  // ./waf --command-template="%s --ns3::ConfigStore::Filename=input-defaults.txt --ns3::ConfigStore::Mode=Load --ns3::ConfigStore::FileFormat=RawText" --run src/lte/examples/lena-first-sim

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // Parse again so you can override default values from the command line
  cmd.Parse (argc, argv);

  // HELPER to setup common objects
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  // Uncomment to enable logging
//  lteHelper->EnableLogComponents ();

  // Create Nodes: eNodeB and UE
  NodeContainer femtoNodes;
  NodeContainer ueNodes1,ueNodes2;
  femtoNodes.Create (2);
  ueNodes1.Create (1);
  ueNodes2.Create (1);

  ///////////////// Positions ///////////////////

  // Install Mobility Model
  // Femtocells
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (femtoNodes);
  BuildingsHelper::Install (femtoNodes);

  double enbDist = 100; // Distance between Femtocells
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (enbDist, 0.0, 0.0));
  MobilityHelper enbMobility;
  enbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbMobility.SetPositionAllocator (positionAlloc);
  enbMobility.Install (femtoNodes);

  // UEs
  double radius = 10;
  // Position of UEs attached to eNB 1
  MobilityHelper ue1mobility;
  ue1mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                    "X", DoubleValue (0.0),
                                    "Y", DoubleValue (0.0),
                                    "rho", DoubleValue (radius));
  ue1mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  ue1mobility.Install (ueNodes1);
  // Add building
  //BuildingsHelper::Install (ueNodes1);

  // Position of UEs attached to eNB 2
  MobilityHelper ue2mobility;
  ue1mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                    "X", DoubleValue (0.0),
                                    "Y", DoubleValue (0.0),
                                    "rho", DoubleValue (radius));
  ue2mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  ue2mobility.Install (ueNodes2);
  // Add building
  //BuildingsHelper::Install (ueNodes2);

  /////////////// Net Devices //////////////////

  // Create Devices and install them in the Nodes (eNB and UE)
  // aka Protocol stacks
  NetDeviceContainer femtoDevs;
  NetDeviceContainer ueDevs1, ueDevs2;
  // Default scheduler is PF, uncomment to use RR
  //lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");

  femtoDevs = lteHelper->InstallEnbDevice (femtoNodes);
  ueDevs1 = lteHelper->InstallUeDevice (ueNodes1);
  ueDevs2 = lteHelper->InstallUeDevice (ueNodes2);

  // Attach a UEs to a eNBs
  lteHelper->Attach (ueDevs1, femtoDevs.Get(0));
  lteHelper->Attach (ueDevs2, femtoDevs.Get(1));

  // Activate a data radio bearer
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE; // VoIP
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs1, bearer);
  lteHelper->ActivateDataRadioBearer (ueDevs2, bearer);

  // Insert RLC Performance Calculator
  // std::string dlOutFname = "DlRlcStats";
  // dlOutFname.append (tag.str ());
  // std::string ulOutFname = "UlRlcStats";
  // ulOutFname.append (tag.str ());

  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();

  Simulator::Stop (Seconds (1.05));

  Simulator::Run ();

  // GtkConfigStore config;
  // config.ConfigureAttributes ();

  Simulator::Destroy ();
  return 0;
}
