import argparse

parser = argparse.ArgumentParser(description='Create reroute.txt')
parser.add_argument('sources', metavar='N', type=str, nargs='+',
                    help='proxy source files')
parser.add_argument('--output', type=str, help='output file')
args = parser.parse_args()

functions = set()

for name in args.sources:
    with open(name) as file:
        for line in file:
            if "CMOCK_MOCK_FUNCTION" in line or "CMOCK_MOCK_CONST_FUNCTION" in line:
                tokens = line.split(",")
                functions.add(tokens[2].strip())


with open(args.output, "w") as file:
    for function in functions:
        file.write(f"{function} proxy_{function}\n")