import math
import argparse
import json

colors_dict = {
    "red": 0xFF0007,
    "pink": 0xCF00FF,           #
    "violet": 0x5400FF,         #
    "blue": 0x0097FF,           #
    "deep_blue": 0x003BFF,      #
    "teal": 0x00FFD1,
    "green": 0x13FF00,          #
    "salad": 0x8CFF00,          #
    "yellow": 0xFFEE00,
    "orange": 0xFF3500,
}

color_by_pdg = {
    "gamma": "yellow",
    "e-": "blue",
    "pi+": "pink",
    "pi-": "salad",
    "proton": "violet",
    "neutron": "green"
}

def select_color(id, pdg, pdg_name):
    if pdg_name == "gamma":
        return colors_dict['']


def parse_event(events, line, prefix):
    tokens = line[1:].split()
    event_num = int(tokens[1])
    run_num = int(tokens[0])

    name = f"{prefix}_{event_num}"

    event = {
        "run number": run_num,
        "event number": event_num,
        "Tracks": {
            "mc_tracks": []
        }
    }
    events[name] = event
    return event


def parse_track(event, line):

    tokens = line[1:].split()
    id = int(tokens[0])
    pdg = int(tokens[1])
    pdg_name = tokens[2]
    charge=int(tokens[3])
    eta = float(tokens[4])
    phi = float(tokens[5])
    q_over_p = float(tokens[6])
    px = float(tokens[7])
    py = float(tokens[8])
    pz = float(tokens[9])
    vx = float(tokens[10])
    vy = float(tokens[11])
    vz = float(tokens[12])

    if charge == 0:
        q_over_p = 0
    elif math.isinf(q_over_p):
        q_over_p = 0
    elif math.isnan(q_over_p):
        q_over_p = 0

    track = {
        "id": id,
        "pdg": pdg,
        "pdg_name": pdg_name,
        "charge": charge,
        "pos": [],
        "pnv": [px, py, pz, vx, vy, vz],
        "dparams": [0.0, 0.0, phi, eta, q_over_p],   # d0, z0, phi, eta, qOverP
        "color": colors_dict["deep_blue"] if charge < 0 else (colors_dict["red"] if charge > 0 else colors_dict["teal"]),  # Placeholder value, adjust as needed
    }

    if pdg_name in color_by_pdg:
        track["color"] = colors_dict[color_by_pdg[pdg_name]]

    event["Tracks"]["mc_tracks"].append(track)
    return track


def parse_point(track, line):
    tokens = line[1:].split()
    x = float(tokens[0])
    y = float(tokens[1])
    z = float(tokens[2])
    t = float(tokens[3])

    track['pos'].append([x, y, z, t])
    return x, y, z, t


def convert_to_json(input_file, output_file, event_prefix, mark_recoil):
    print(f"event_prefix: {event_prefix}, mark_recoil: {mark_recoil}")

    events = {}
    current_event = None
    current_track = None
    with open(input_file, 'r') as in_file:
        for line in in_file:
            line = line.strip()

            if line.startswith("#"):
                continue

            # Event?
            if line.startswith('E'):
                current_event = parse_event(events, line, event_prefix)
                print(current_event)

            if line.startswith('T'):
                current_track = parse_track(current_event, line)

            if line.startswith('P'):
                parse_point(current_track, line)

    if mark_recoil:
        for evt_name, event in events.items():
            for track in event["Tracks"]["mc_tracks"]:
               # print(f"look at track {track['pdg_name']}")
                if track["pdg_name"] == "e-":
                    print("Found e-")
                    track["color"] = colors_dict["radiant_red"]
                    break

    with open(output_file, 'w') as json_file:
        json.dump(events, json_file, indent=2)


def main():
    parser = argparse.ArgumentParser(description="Convert a file from txt to json format.")
    parser.add_argument("input_file", help="The name of the input file.")
    parser.add_argument("-o", "--output-file", default=None, help="The name of the output file. If not provided, deduce from input file name.")
    parser.add_argument("-p", "--event-prefix", default="event", help="Prefix for event name in phoenix json")
    parser.add_argument("--mark-recoil", action="store_true", help="Mark with special color recoil electrons")

    args = parser.parse_args()
    input_file = args.input_file

    # Deduce the output file name if not provided
    if not args.output_file:
        if input_file.endswith('.txt'):
            output_file = input_file[:-4] + '.json'
        else:
            output_file = input_file + '.json'
    else:
        output_file = args.output_file

    convert_to_json(args.input_file, output_file, args.event_prefix, args.mark_recoil)


if __name__ == "__main__":
    main()
