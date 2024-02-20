import argparse
import subprocess
import os

prefix = "proxy_"

def getFunctionsFromMockFile(file):

    output = subprocess.check_output(f"nm --defined-only {file} | tr -s ' ' | awk '{{ if ($2 == \"T\") {{ print $3 }}}}'", shell=True, text=True)
    return set([text.removeprefix(prefix) for text in output.split() if text.startswith(prefix)])


def getUndefinedSymbolsFromObj(file):
    output = subprocess.check_output(f"nm --undefined-only {file} | tr -s ' ' | cut -d ' ' -f 3", shell=True, text=True)
    return set(output.split())


def createReroutedTxt(usedMocks):
    reroutedFile = "rerouted.txt"
    with open(reroutedFile, "w") as file:
        for function in usedMocks:
            file.write(f"{function} {prefix}{function}\n")
    return reroutedFile


def redirectSymbols(file, usedMocks):
    if len(usedMocks) != 0:
        reroutedFile = createReroutedTxt(usedMocks)
        subprocess.run(f"objcopy --redefine-syms={reroutedFile} {file}", shell=True)
        os.remove(reroutedFile)


def handleRerouteCommand(args):
    mockedFunctions = set()

    for file in args.mocks:
        mockedFunctions.update(getFunctionsFromMockFile(file))

    for file in args.objects:
        undefinedSymbols = getUndefinedSymbolsFromObj(file)
        usedMocks = undefinedSymbols.intersection(mockedFunctions)
        redirectSymbols(file, usedMocks)


def handleGenerateCommand(args):
    print("Not ready yes")


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Mocking C function')
    subparsers = parser.add_subparsers(dest='command')

    reroute = subparsers.add_parser('reroute', help='reroute objcts')
    reroute.add_argument('--mocks', metavar='N', type=str, nargs='+', help='mock object files', required=True)
    reroute.add_argument('--objects', metavar='N', type=str, nargs='+', help='source object files', required=True)

    generate = subparsers.add_parser('generate', help='generate mocks sources')
    generate.add_argument('--mocks', metavar='N', type=str, nargs='+', help='mock header files', required=True)

    args = parser.parse_args()
    if args.command == 'reroute':
        handleRerouteCommand(args)
    elif args.command == 'generate':
        handleGenerateCommand(args)
