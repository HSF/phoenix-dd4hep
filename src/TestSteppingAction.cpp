//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
//==========================================================================

// Framework include files
#include "DDG4/Geant4SteppingAction.h"

#include <G4Step.hh>
#include <G4RunManager.hh>
#include <G4TrackStatus.hh>
#include <G4Run.hh>
#include <G4Event.hh>
#// #include <DDG4/

#include <fmt/core.h>

/// Namespace for the AIDA detector description toolkit
namespace dd4hep {

    /// Namespace for the Geant4 based simulation part of the AIDA detector description toolkit
    namespace sim {

        /// Class to count steps and suspend tracks every 5 steps
        /** Class to count steps and suspens tracks every 5 steps
         * Count steps and suspend
         *
         *  \version 1.0
         *  \ingroup DD4HEP_SIMULATION
         */
        class TestSteppingAction : public Geant4SteppingAction {
            std::size_t m_calls_steps { 0UL };
            std::size_t m_calls_suspended { 0UL };
            std::size_t m_calls_kill { 0UL };
            std::string  m_file_name {"events_stepping.csv"};
            std::ofstream m_output_file;
            std::once_flag initFlag;

        public:
            /// Standard constructor
            TestSteppingAction(Geant4Context* context, const std::string& nam)
                    : Geant4SteppingAction(context, nam)
            {
                declareProperty("OutputFileName",   m_file_name);
            }
            /// Default destructor
            virtual ~TestSteppingAction()   {
                info("+++ Track Calls Steps: %ld", m_calls_steps);
                info("+++ Track Calls Suspended: %ld", m_calls_suspended);
                info("+++ Track Calls Killed: %ld", m_calls_kill);

                // Do we need it here? It should be done automatically
                if(m_output_file.is_open() && m_output_file.good()) {
                    m_output_file.close();
                }
            }

            /// Checks m_output_file is open and is ready for write or throws an exception
            void ensure_output_writable() {
                if(!m_output_file.is_open() || !m_output_file.good()) {
                    auto err_msg = fmt::format( "Failed to open the file or file stream is in a bad state. File name: '{}'", m_file_name);
                    error(err_msg.c_str());
                    throw std::runtime_error(err_msg);
                }
            }

            void write_point(G4int run_num, G4int event_num, G4StepPoint *point, G4Track *track){

                // Check we can write in file
                ensure_output_writable();

                auto row = fmt::format("{}, {}, {}, {}, \"{}\", {}, {}, {}, {}, {}",
                            run_num,
                            event_num,
                            track->GetTrackID(),
                            track->GetParticleDefinition()->GetPDGEncoding(),
                            track->GetParticleDefinition()->GetParticleName(),
                            point->GetCharge(),
                            point->GetPosition().x(),
                            point->GetPosition().y(),
                            point->GetPosition().z(),
                            point->GetProperTime()
                            );
                m_output_file<<row<<std::endl;

            }

/// stepping callback
            virtual void operator()(const G4Step* step, G4SteppingManager*) {

                // Get run and event number
                auto run_num = context()->run().run().GetRunID();
                auto event_num = context()->event().event().GetEventID();

                // First time in this function. Open a file
                std::call_once(initFlag, [this, run_num, event_num, step](){
                    m_output_file = std::ofstream(m_file_name);

                    ensure_output_writable();
                    m_output_file<<"run_num,event_num,track_id,pdg,name,charge,point_x,point_y,point_z,point_t"<<std::endl;

                    // We always write GetPostStepPoint so the first time we have to write GetPreStepPoint
                    write_point(run_num, event_num, step->GetPreStepPoint(), step->GetTrack());
                });

                // Write post point info
                write_point(run_num, event_num, step->GetPostStepPoint(), step->GetTrack());

                // Statistics
                ++m_calls_steps;
            }
        };
    }    // End namespace sim
}      // End namespace dd4hep

#include "DDG4/Factories.h"
DECLARE_GEANT4ACTION_NS(dd4hep::sim,TestSteppingAction)
