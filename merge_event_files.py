import json
import argparse
import os


def merge_json_files(input_files, output_file):
    combined_data = {}

    # Loop through each file provided
    for file_path in input_files:
        # Open and load the JSON file
        with open(file_path, 'r') as file:
            data = json.load(file)
            # Update the combined dictionary with the renamed data
            combined_data.update(data)

    # Write the combined data to the specified output file
    with open(output_file, 'w') as file:
        json.dump(combined_data, file, indent=4)


def main():
    # Set up command-line argument parsing
    parser = argparse.ArgumentParser(description="Merge multiple JSON files into a single JSON file.")
    parser.add_argument('input_files', nargs='+', help='Input JSON file paths')
    parser.add_argument('-o', '--output', default='combined_data.json', help='Output JSON file name')

    args = parser.parse_args()

    # Call the merge function with the list of input files and the output file name
    merge_json_files(args.input_files, args.output)


if __name__ == '__main__':
    main()