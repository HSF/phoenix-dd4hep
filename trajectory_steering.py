from DDSim.DD4hepSimulation import DD4hepSimulation
from g4units import mm, GeV, MeV, m
import DDG4
import argparse
import sys


# This printout is to check this steering file is using
print("== STARTED STEERING FILE ==")
print(f"g4units: mm={mm}, GeV={GeV}, MeV={MeV}")


# Determine output file name
# Intercept  --outputFile flag
parser = argparse.ArgumentParser()
parser.add_argument('--outputFile', type=str, default="output", help='Output file name')
args, _ = parser.parse_known_args()

# Remove '.edm4hep.root' and append '.txtevt' or use default name (if no --outputFile is given)
outputFile = args.outputFile if args.outputFile else "output"
if outputFile.endswith('.edm4hep.root'):
    outputFile = outputFile[:-len('.edm4hep.root')] + '.evt.txt'
else:
    outputFile += '.evt.txt'

print(f"Steering event display 'outputFile' = {outputFile}")

# This is needed for stepping file
SIM = DD4hepSimulation()

# Enable UI in the simulation
#SIM.enableUI()

# Set UI commands to enable trajectory storage
# These commands need to be executed before the first event
SIM.ui.commandsConfigure = [
    f'/tracking/storeTrajectory 2',
    '/tracking/verbose 1',
    # Optional: command to draw trajectories in visualization (if used)
    #'/vis/scene/add/trajectories rich'
]

# Need DDG4 kernel to add stepping
kernel = DDG4.Kernel()

# # Set up the UI manager and commands
# ui = DDG4.Action(kernel, "Geant4UIManager/UI")
# ui.Commands = [
#     # Make sure trajectories are stored
#     f'/tracking/storeTrajectory 2',
#     # Verbose level for tracking
#     '/tracking/verbose 1',
#     # Optional: command to draw trajectories in visualization (if used)
#     '/vis/scene/add/trajectories rich'
# ]
# kernel.registerGlobalAction(ui)

# Instantiate the stepping action
event_action = DDG4.EventAction(kernel, 'TrajectoryWriterEventAction/TrajectoryWriter')
# stepping = DDG4.SteppingAction(kernel, 'TextDumpingSteppingAction/MyStepper')
# stepping.OutputFileName = outputFile
# stepping.OnlyPrimary = False
# stepping.MomentumMin = 150
# stepping.VertexCut = True              # Cut tracks which vertex is outside z_min-z_max range
# stepping.VertexZMin = -5000            # [mm] Min Z for Vertex Cut. Will work only if VertexCut = True
# stepping.VertexZMax = 5000             # [mm] Max Z for Vertex Cut. Will work only if VertexCut = True

#kernel.steppingAction().add(stepping)
kernel.eventAction().add(event_action)
print("dir(kernel)")
print(dir(kernel))

print("== ENDED STEERING FILE ==")