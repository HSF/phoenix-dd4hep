//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Implementation of the TrajectoryWriterEventAction as a DD4hep/DDG4 plugin
//==========================================================================

// Framework include files
#include "DDG4/Geant4EventAction.h"
#include "DD4hep/Printout.h"
#include "DDG4/Geant4Kernel.h"

// Geant4 headers
#include "G4Event.hh"
#include "G4TrajectoryContainer.hh"
#include "G4VTrajectory.hh"
#include "G4VTrajectoryPoint.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4AttValue.hh"
#include "G4AttDef.hh"
#include "G4RichTrajectory.hh"
#include "G4RichTrajectoryPoint.hh"
#include "G4SmoothTrajectory.hh"
#include "G4SmoothTrajectoryPoint.hh"
#include "G4Trajectory.hh"
#include "G4TrajectoryPoint.hh"

// C/C++ headers
#include <fstream>
#include <iostream>

/// Namespace for the AIDA detector description toolkit
namespace dd4hep {

  /// Namespace for the Geant4 based simulation part of the AIDA detector description toolkit
  namespace sim {

    /**
     * \addtogroup Geant4EventAction
     * @{
     * \package TrajectoryWriterEventAction
     * \brief Writes out all trajectories to a CSV file
     *
     *  This action writes out trajectories collected during Geant4 simulation.
     *  Requires trajectories to be enabled via /tracking/storeTrajectory command.
     *
     * @}
     */

    /// Trajectory writer event action for dd4hep simulation
    /** This action writes all trajectories to a CSV file
     *
     *  \author  Your Namewe
     *  \version 1.0
     *  \ingroup DD4HEP_SIMULATION
     */
    class TrajectoryWriterEventAction : public Geant4EventAction {
    protected:
      /// Property: output file name
      std::string m_outputFile {"trajectories.csv"};

      /// Output file stream
      std::ofstream m_output;

      /// Write header flag
      bool m_writeHeader {true};

    public:
      /// Standard constructor
      TrajectoryWriterEventAction(Geant4Context* context, const std::string& name = "TrajectoryWriterEventAction")
        : Geant4EventAction(context, name)
      {
        declareProperty("OutputFile", m_outputFile);

        // Open the output file at initialization
        m_output.open(m_outputFile, std::ios::out);
        if (!m_output.is_open()) {
          fatal("+++ Failed to open trajectory output file: %s", m_outputFile.c_str());
          throw std::runtime_error("Failed to open trajectory output file");
        }
        info("+++ Successfully opened trajectory output file: %s", m_outputFile.c_str());
      }

      /// Destructor
      virtual ~TrajectoryWriterEventAction() {
        if (m_output.is_open()) {
          m_output.close();
        }
      }

      /// Begin-of-event callback
      virtual void begin(const G4Event* /* event */) override {
        // Nothing to do at begin of event
      }

      /// Write trajectory points for a specific trajectory with detailed information
      void writeTrajectoryPoints(G4VTrajectory* trajectory, G4int eventID, G4int trackID) {
        G4int n_points = trajectory->GetPointEntries();
        info("+++ Writing %d trajectory points for track %d", n_points, trackID);


        for (G4int i = 0; i < n_points; i++) {
          G4VTrajectoryPoint* point = trajectory->GetPoint(i);
          G4ThreeVector position = point->GetPosition();

          // Create a base record for this point
          std::stringstream pointRecord;
          pointRecord << "POINT,"
                     << eventID << ","
                     << trackID << ","
                     << i << ","
                     << position.x()/mm << ","
                     << position.y()/mm << ","
                     << position.z()/mm;

          // Additional information for different point types
          G4RichTrajectoryPoint* richPoint = dynamic_cast<G4RichTrajectoryPoint*>(point);
          if (richPoint) {
            // Rich trajectory point has detailed information
            std::vector<G4AttValue>* attValues = richPoint->CreateAttValues();

            // Add all available values from the rich point
            // Initialize variables for common attributes
            G4double energyDeposit = 0.0;
            G4double remainingEnergy = 0.0;
            std::string processName = "None";
            std::string processType = "None";
            std::string preStatus = "None";
            std::string postStatus = "None";
            G4double preTime = 0.0;
            G4double postTime = 0.0;
            G4double preWeight = 0.0;
            G4double postWeight = 0.0;

            // Extract values from AttValues
            for (const auto& attValue : *attValues) {
              std::string name = attValue.GetName();
              std::string value = attValue.GetValue();

              // Extract numeric values from G4BestUnit format
              if (name == "TED") {
                // TED = Total Energy Deposit
                std::istringstream iss(value);
                iss >> energyDeposit;
              }
              else if (name == "RE") {
                // RE = Remaining Energy
                std::istringstream iss(value);
                iss >> remainingEnergy;
              }
              else if (name == "PDS") {
                // PDS = Process Defined Step
                processName = value;
              }
              else if (name == "PTDS") {
                // PTDS = Process Type Defined Step
                processType = value;
              }
              else if (name == "PreStatus") {
                preStatus = value;
              }
              else if (name == "PostStatus") {
                postStatus = value;
              }
              else if (name == "PreT") {
                std::istringstream iss(value);
                iss >> preTime;
              }
              else if (name == "PostT") {
                std::istringstream iss(value);
                iss >> postTime;
              }
              else if (name == "PreW") {
                std::istringstream iss(value);
                iss >> preWeight;
              }
              else if (name == "PostW") {
                std::istringstream iss(value);
                iss >> postWeight;
              }
            }

            // Add rich information to point record
            pointRecord << ","
                        << energyDeposit/CLHEP::MeV << ","   // Energy deposit in MeV
                        << remainingEnergy/CLHEP::MeV << "," // Remaining energy in MeV
                        << processName << ","
                        << processType << ","
                        << preStatus << ","
                        << postStatus << ","
                        << preTime/CLHEP::ns << ","          // Pre-step time in ns
                        << postTime/CLHEP::ns << ","         // Post-step time in ns
                        << preWeight << ","
                        << postWeight;

            // Clean up
            delete attValues;
          }

          // Write the point record
          m_output << pointRecord.str() << std::endl;

          // Handle auxiliary points for smooth trajectories
          G4SmoothTrajectoryPoint* smoothPoint = dynamic_cast<G4SmoothTrajectoryPoint*>(point);
          if (smoothPoint) {
            const std::vector<G4ThreeVector>* auxPoints = smoothPoint->GetAuxiliaryPoints();
            if (auxPoints && !auxPoints->empty()) {
              info("+++ Writing %d auxiliary points for point %d of track %d",
                   (int)auxPoints->size(), i, trackID);

              for (size_t j = 0; j < auxPoints->size(); j++) {
                const G4ThreeVector& auxPos = (*auxPoints)[j];
                m_output << "AUXPOINT,"
                        << eventID << ","
                        << trackID << ","
                        << i << "." << j << ","
                        << auxPos.x()/mm << ","
                        << auxPos.y()/mm << ","
                        << auxPos.z()/mm
                        << std::endl;
              }
            }
          }
        }
      }

      /// End-of-event callback to write trajectories
      virtual void end(const G4Event* event) override {
        G4TrajectoryContainer* trajectoryContainer = event->GetTrajectoryContainer();
        if (!trajectoryContainer) return;

        G4int n_trajectories = trajectoryContainer->entries();
        if (n_trajectories == 0) return;

        // Write header if needed
        if (m_writeHeader) {
          m_output << "# Event,TrackID,ParentID,ParticleName,PDGEncoding,Charge,InitialKineticEnergy[MeV],"
                  << "InitialMomentumX[MeV],InitialMomentumY[MeV],InitialMomentumZ[MeV]"
                  << std::endl;

          // Header for trajectory points file - detailed format
          m_output << "# Point format for regular trajectories:" << std::endl;
          m_output << "# POINT,EventID,TrackID,PointIndex,X[mm],Y[mm],Z[mm]" << std::endl;
          m_output << "# Point format for rich trajectories:" << std::endl;
          m_output << "# POINT,EventID,TrackID,PointIndex,X[mm],Y[mm],Z[mm],EnergyDeposit[MeV],RemainingEnergy[MeV],"
                  << "ProcessName,ProcessType,PreStepStatus,PostStepStatus,PreTime[ns],PostTime[ns],PreWeight,PostWeight"
                  << std::endl;
          m_output << "# AuxPoint format for smooth trajectories:" << std::endl;
          m_output << "# AUXPOINT,EventID,TrackID,PointIndex.SubIndex,X[mm],Y[mm],Z[mm]"
                  << std::endl;

          m_writeHeader = false;
        }

        G4int eventID = event->GetEventID();
        info("+++ Writing %d trajectories for event %d", n_trajectories, eventID);

        // Process each trajectory
        for (G4int i = 0; i < n_trajectories; i++) {
          G4VTrajectory* trajectory = (*trajectoryContainer)[i];

          // Write trajectory summary info
          m_output << eventID << ","
                  << trajectory->GetTrackID() << ","
                  << trajectory->GetParentID() << ","
                  << trajectory->GetParticleName() << ","
                  << trajectory->GetPDGEncoding() << ","
                  << trajectory->GetCharge() << ",";

          // Get extended trajectory properties using the specific trajectory type
          G4double initialKE = 0.0;
          G4ThreeVector initialMomentum = trajectory->GetInitialMomentum();

          // G4Trajectory and derivatives define GetInitialKineticEnergy()
          if (G4Trajectory* trajNormal = dynamic_cast<G4Trajectory*>(trajectory)) {
            initialKE = trajNormal->GetInitialKineticEnergy();
          }
          else if (G4RichTrajectory* trajRich = dynamic_cast<G4RichTrajectory*>(trajectory)) {
            initialKE = trajRich->GetInitialKineticEnergy();
          }
          else if (G4SmoothTrajectory* trajSmooth = dynamic_cast<G4SmoothTrajectory*>(trajectory)) {
            initialKE = trajSmooth->GetInitialKineticEnergy();
          }

          // Write extended properties
          m_output << initialKE/CLHEP::MeV << ","
                  << initialMomentum.x()/CLHEP::MeV << ","
                  << initialMomentum.y()/CLHEP::MeV << ","
                  << initialMomentum.z()/CLHEP::MeV
                  << std::endl;

          // Write trajectory points
          writeTrajectoryPoints(trajectory, eventID, trajectory->GetTrackID());
        }
      }
    };
  }
}

// Register the plugin with DD4hep
#include "DDG4/Factories.h"
DECLARE_GEANT4ACTION(TrajectoryWriterEventAction)