################################################################################
# ExtractionConv.py
#
# Description: This python script is used to extract and parse the measurement
# data gathered by the RTOS interrupt benchmarks. The user is adviced to ensure
# that the configuration of this module (i.e CONSTANTS section) is in line with
# the RTOS benchmarks configuration.
#
# Author: Alexy Torres Aurora Dugo
#
# Date: 02/02/2022
################################################################################


################################################################################
# IMPORTS
################################################################################
import argparse
import sys
import struct

################################################################################
# CONSTANTS
################################################################################
DUMP_REGION_SIZE = 0x200000
HEADER_SIZE      = 0x100

MAGIC_VALUE        = "INTBDUMP"
PART_MAGIC_VALUE   = "PART"
SC_MAGIC_VALUE     = "SC  "
INTINT_MAGIC_VALUE = "IINT"
EXTINT_MAGIC_VALUE = "EINT"
IPI_MAGIC_VALUE    = "IPI "

MAGIC_SIZE               = 8
DUMP_SIZE_FIELD_SIZE     = 4
DUMP_REG_MAGIC_SIZE      = 4
DUMP_PARTID_FIELD_SIZE   = 4
DUMP_EXECTIME_FIELD_SIZE = 8
DUMP_L2MISS_FIELD_SIZE   = 4
DUMP_TLBMISS_FIELD_SIZE  = 4

################################################################################
# GLOBAL VARIABLES
################################################################################

# None

################################################################################
# CLASSES
################################################################################

# None

################################################################################
# FUNCTIONS
################################################################################
def parseCommand():
    """
    Parses the commend line to extract the following arguments:
        - Input file name.
        - Output file name.

    Parameters
    ----------
        None.

    Return
    ----------
        The argparse object that contains the parsed command line (parse_args
        is already called in this function)

    Raises
    ----------
        None.
    """
    parser = argparse.ArgumentParser(description = "Extractor Args Parser")

    # Add command argument: name of the file to convert
    parser.add_argument("-i", type = str, nargs = "?", dest = "inputFilename",
                        required = True,
                        help = "Input file name. This is the name of the file"
                               " to convert.")

    # Add command argument: name of the output file
    parser.add_argument("-o", type = str, nargs = "?", dest = "outputFilename",
                        required = True,
                        help = "Output file name. This is the name of the"
                               " output file once converted.")

    return parser.parse_args()

def convertFile(inputFilename, outputFilename):
    """
    Parses the binary input file and converts it to a CSV file containing human
    readable data.
    This function checks the integrity of the binary file prior to extracting
    it.

    Parameters
    ----------
        inputFilename : str (in)
            The name of the input file to convert.
        outputFilename: str(in)
            The name of the output file to generate.

    Return
    ----------
        None.

    Raises
    ----------
        IOError, RuntimeError and others can be raised during the files
        manipulations. An error message is associated with the exception to
        give information about its cause.
    """

    try:
        # Open the input file
        with open(inputFilename, "rb") as inputFile:

            # Check the magic value

            buff = inputFile.read(MAGIC_SIZE).decode("ASCII")
            if(buff != MAGIC_VALUE):
                raise RuntimeError("Binary file has an incorrect format "
                                   "(MAGIC invalid)")

            # Read the rest of the header to moove cursor
            inputFile.read(HEADER_SIZE - MAGIC_SIZE)

            # Open the output file
            with open("PART_" + outputFilename, "w") as outputFile:
                # Write header
                outputFile.write("Type,Id,ExecTime,l2Miss,tlbMiss\n")
                # Extract the PART region
                extractRegion("PART", PART_MAGIC_VALUE, inputFile, outputFile)

            with open("SC_" + outputFilename, "w") as outputFile:
                # Write header
                outputFile.write("Type,Id,ExecTime,l2Miss,tlbMiss\n")
                # Extract the SC region
                extractRegion("SC", SC_MAGIC_VALUE, inputFile, outputFile)

            with open("IntINT_" + outputFilename, "w") as outputFile:
                # Write header
                outputFile.write("Type,Id,ExecTime,l2Miss,tlbMiss\n")
                # Extract the IntINT region
                extractRegion("IntINT", INTINT_MAGIC_VALUE, inputFile, outputFile)

            with open("ExtINT_" + outputFilename, "w") as outputFile:
                # Write header
                outputFile.write("Type,Id,ExecTime,l2Miss,tlbMiss\n")
                # Extract the ExtINT region
                extractRegion("ExtINT", EXTINT_MAGIC_VALUE, inputFile, outputFile)

            with open("IPI_" + outputFilename, "w") as outputFile:
                # Write header
                outputFile.write("Type,Id,ExecTime,l2Miss,tlbMiss\n")
                # Extract the IPI region
                extractRegion("IPI", IPI_MAGIC_VALUE, inputFile, outputFile)

    except IOError as exc:
        print("Error while manipulating files: " + str(exc))
        raise
    except RuntimeError as exc:
        print("Internal error while manipulating files: " + str(exc))
        raise
    except:
        print("Error while manipulating files: " + str(sys.exc_info()[0]))
        raise

def extractRegion(type, magic, inputFile, outputFile):
    """
    Extract a dump region from the binary file given as parameter. The function
    will read the dump region size and convert it to a CSV format that is
    written in the output file given as parameter

    Parameters
    ----------
        type : str (in)
            Type name of the dump region.
        magic: str (in)
            The magic value that is validated with the region header.
        inputFile : File (in)
            The name of the input file to convert.
        outputFile: File (out)
            The name of the output file to generate.

    Return
    ----------
        None.

    Raises
    ----------
        IOError, RuntimeError and others can be raised during the files
        manipulations. An error message is associated with the exception to
        give information about its cause.
    """
    # Read the magic
    buff = inputFile.read(DUMP_REG_MAGIC_SIZE).decode("ASCII")
    if(buff != magic):
        raise RuntimeError("Dump region file has an incorrect format "
                           "(MAGIC invalid: \"" + buff + "\", expected: \"" +
                           magic + "\")")

    print("==== Extracting region " + type)
    # Read the region size
    regSize = struct.unpack(">I", inputFile.read(DUMP_SIZE_FIELD_SIZE))[0]
    print("\tRegion size: " + str(regSize))

    # Get each data chunk and convert them
    toRead = regSize
    while toRead > 0:
        # Get the part ID
        partId = struct.unpack(">I", inputFile.read(DUMP_PARTID_FIELD_SIZE))[0]
        # Get the execution time
        execTime = struct.unpack(">Q", inputFile.read(DUMP_EXECTIME_FIELD_SIZE))[0]
        # Get the L2 miss
        l2Miss = struct.unpack(">I", inputFile.read(DUMP_L2MISS_FIELD_SIZE))[0]
        # Get the TLB miss
        tlbMiss = struct.unpack(">I", inputFile.read(DUMP_TLBMISS_FIELD_SIZE))[0]

        outputFile.write("{},{},{},{},{}\n".format(type, partId, execTime, l2Miss, tlbMiss))

        toRead -= DUMP_PARTID_FIELD_SIZE + DUMP_EXECTIME_FIELD_SIZE + DUMP_L2MISS_FIELD_SIZE + DUMP_TLBMISS_FIELD_SIZE

    # Skip the rest of the region
    inputFile.seek(DUMP_REGION_SIZE - regSize - DUMP_REG_MAGIC_SIZE - DUMP_SIZE_FIELD_SIZE, 1)

if __name__ == "__main__":
    # Get the arguments
    args = parseCommand()

    try:
        convertFile(args.inputFilename, args.outputFilename)
        print("\n---------------------------------")
        print("Extraction finished without error")
    except:
        print("Error during extraction")
