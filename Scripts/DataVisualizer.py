################################################################################
# DataVisualizer.py
#
# Description: This python script is used to parse en provide CSV and visuals of
# the extracted benchmark data. The script will recursively analyse the input
# folder. The input folder must follow the following rules:
#
# inputFolder\
#    MITIG-NAME_mitig\
#        Baseline\
#            PART_output.csv
#        MC_05_INT-TYPE\
#            PART_output.csv
#
# MITIG-NAME is the name of the mitigation used during the benchmarks and must
# not contain underscores ('_').
# MC_05_INT-TYPE correspond to the multi-core env (MC for multi-core, SC for
# single-core), 05 for the interrupt frequency (0.5ms) and INT-TYPE the
# interrupt type (INT, EXT, SC, IPI, ALL).
#
# The user is adviced to ensure that the configuration of this module (i.e
# CONSTANTS section) is in line with the RTOS benchmarks configuration.
#
# Author: Alexy Torres Aurora Dugo
#
# Date: 02/02/2022
################################################################################


################################################################################
# IMPORTS
################################################################################
import os
import csv
import json
import argparse
import collections
import pandas as pd
import numpy as np

import matplotlib.pyplot as plt
from matplotlib.pyplot import cm
from matplotlib import rcParams

################################################################################
# CONSTANTS
################################################################################
PARTITION_COUNT = 10
METRICS = ["ExecTime", "l2Miss"]
#ENVTEST = ["Baseline", "ALL", "IPI", "SC", "EXT", "INT"]
ENVTEST = ["ALL"]

################################################################################
# GLOBAL VARIABLES
################################################################################

detectedMitigation = []

################################################################################
# CLASSES
################################################################################

class DataObject:
    """
    Each dictionary of the dataObject has the following hierarchy:
    dataObj.min[PART_ID][MITIG_TYPE][INT_TYPE][INT_INTENSITY] -> DataFrame.

    - PART_ID is the partition ID.
    - MITIG_TYPE is the mitigation type (for instance CP for cache partitioning)
    - INT_TYPE is the interrupt type generated (EXT, INT, etc.)
    - INT_INTENSITY is the rate at which interrupts were generated.
    - DataFrame is the data frame stored.

    """
    def __init__(self):
        self.dataFrame = None
        self.min       = {}
        self.max       = {}
        self.mean      = {}
        self.median    = {}
        self.stdev     = {}
        self.outliers  = {}
        self.type      = "Unknown"
        self.mitig     = "Unknown"
        self.intensity = -1

################################################################################
# FUNCTIONS
################################################################################
def set_box_color(bp, color):
    """
        Sets the color for a given box plot.

    Parameters
    ----------
        bp :
            The box plot to color.
        color : int (in)
            The color to use.

    Return
    ----------
        None.

    Raises
    ----------
        None.
    """
    plt.setp(bp['boxes'], color=color)
    plt.setp(bp['whiskers'], color=color)
    plt.setp(bp['caps'], color=color)
    plt.setp(bp['medians'], color=color)

def genBoxplotTypesComp(partitionsData, partId, metric):
    """
        Displays box plots for a given partition of the specified metric.

    Parameters
    ----------
        partitionsData : List[] (in)
            The array of processed dataObject to output.
        partId : int (in)
            The partition to output the data of.
        metric : str (in)
            The specific metric to output.

    Return
    ----------
        None.

    Raises
    ----------
        None.
    """
    if partId != 8 and partId != 2:
        return
    typeDict = {}
    typeDictInv = {}
    color = iter(cm.rainbow(np.linspace(0, 1, len(ENVTEST))))

    filteredData = [[] for i in range(len(ENVTEST))]
    i = 0

    for mitig, typeData in partitionsData[partId].items():
        for envName in ENVTEST:
            for intInf, dataFrame in typeData[envName].items():
                if(envName not in typeDict):
                    typeDict[envName] = i
                    typeDictInv[i] = envName
                    i += 1
                filteredData[typeDict[envName]].append(dataFrame[metric])

    rcParams.update({'figure.autolayout': True})
    rcParams.update({'font.size': 20})
    rcParams.update({'font.family': "serif"})

    fig = plt.figure(figsize = (16, 7))
    ax = fig.add_subplot(111)
    i = 0
    offset = 1 * (len(ENVTEST) / -5) + 0.3

    for data in filteredData:
        bp = ax.boxplot(data, notch ='True', vert = 1, widths=0.4,
                        positions=np.array(range(len(data)))*2+offset,
                        sym='.', showfliers=False)
        offset += 0.5
        c = next(color)
        plt.plot([], c=c, label=typeDictInv[i])
        set_box_color(bp, c)
        print("Adding plot: " + typeDictInv[i])
        i += 1

    #plt.legend()

    plt.xticks(range(0, len(detectedMitigation) * 2, 2), detectedMitigation, rotation = 45)
    plt.xlim(-1, len(detectedMitigation) * 2 - 1)
    #plt.tight_layout()
    plt.ylabel('Execution time (ns)')
    #plt.title("Partition " + str(partId) + " | " + metric)
    plt.show(block=False)


    plt.savefig("Partition" + str(partId) + ".png")
    plt.savefig("Partition" + str(partId) + ".pdf")

def genTypeCompCSV(dataArray, metric, partId = -1):
    """
        Prints out the objects containted in a DataArray for a given partition
        and a specific studied metric. The output is formated as a CSV output.

    Parameters
    ----------
        dataArray : DataObject[] (in)
            The array of dataObject to output.
        metric : str (in)
            The specific metric to output.
        partId : int (in)
            The partition to output the data of. Use -1 to output all the
            partitions contained in the dataArray.

    Return
    ----------
        None.

    Raises
    ----------
        None.
    """
    print("partId, metric, type, mitig, min, max, mean, median, stdev, outliers (val), outliers (%)")
    if partId == -1:
        for partId in range(PARTITION_COUNT):
            for dataObj in dataArray:
                if dataObj.type in ENVTEST:
                    print("{},{},{},{},{},{},{},{},{},{},{}".format(partId, metric, dataObj.type, dataObj.mitig,
                                                                    dataObj.min[partId][metric],
                                                                    dataObj.max[partId][metric],
                                                                    dataObj.mean[partId][metric],
                                                                    dataObj.median[partId][metric],
                                                                    dataObj.stdev[partId][metric],
                                                                    dataObj.outliers[partId][metric],
                                                                    dataObj.outliers[partId][metric] / 10000 * 100))
    else:
        for dataObj in dataArray:
                if dataObj.type in ENVTEST:
                    print("{},{},{},{},{},{},{},{},{},{},{}".format(partId, metric, dataObj.type, dataObj.mitig,
                                                                    dataObj.min[partId][metric],
                                                                    dataObj.max[partId][metric],
                                                                    dataObj.mean[partId][metric],
                                                                    dataObj.median[partId][metric],
                                                                    dataObj.stdev[partId][metric],
                                                                    dataObj.outliers[partId][metric],
                                                                    dataObj.outliers[partId][metric] / 10000 * 100))

def parseFilename(filename, dataObj):
    """
        Parses a file path to extract infromations such as the interrupt type,
        interrupt intensity, etc.

    Parameters
    ----------
        filename : str (in)
            The path to parse.

        dataObj : DataObject (out)
            The DataObject to populate with the information exteracted from the
            path.

    Return
    ----------
        None.

    Raises
    ----------
        None.
    """
    splited = filename.split('/')

    if(len(splited) < 3):
        dataObj.type  = "Unknown"
        dataObj.mitig = "Unknown"
        dataObj.intensity = -1
    if(len(splited) > 3):
        splited = splited[len(splited) - 3:]

    dataObj.mitig     = splited[0].split('_')[0]

    if(dataObj.mitig not in detectedMitigation):
        detectedMitigation.append(dataObj.mitig)

    if(splited[1] == "Baseline"):
        dataObj.type      = splited[1]
        dataObj.intensity = 0
    else:
        dataObj.type = splited[1].split('_')[1]

        intBase = ""
        intVal  = splited[1].split('_')[2]
        if(intVal[0] == '0'):
            intBase = "0."
            intVal = intVal[1:]

        dataObj.intensity = 1.0 / float(intBase + intVal)

def generateDataObject(filename):
    """
        Generates a DataObject based on a file. The function extracts and parse the
        data from the file and generates the dataObject.
        Each dictionary of the dataObject has the following hierarchy:

        dataObj.min[PART_ID][MITIG_TYPE][INT_TYPE][INT_INTENSITY] -> DataFrame.

        - PART_ID is the partition ID.
        - MITIG_TYPE is the mitigation type (for instance CP for cache partitioning)
        - INT_TYPE is the interrupt type generated (EXT, INT, etc.)
        - INT_INTENSITY is the rate at which interrupts were generated.
        - DataFrame is the data frame stored.

    Parameters
    ----------
        filename : str (in)
            The path to the file to extract and parse.

    Return
    ----------
        The function returns a generated DataObject populated with the data
        extracted from the file.

    Raises
    ----------
        IOError, RuntimeError and others can be raised during the files
        manipulations. An error message is associated with the exception to
        give information about its cause.
    """

    dataObj = DataObject()

    print("Extracting {}...".format(filename))

    # Get file
    dataFrame = pd.read_csv(filename)

    # Get file attributes
    parseFilename(filename, dataObj)

    # Drop the first 2 executions for each partition, they contain noisy measurements
    dataFrame.drop(index =dataFrame.index[:PARTITION_COUNT * 2] , axis = "index", inplace = True)

    # Drop the first column and sort by partition ID
    dataFrame.drop(labels = "Type", axis = "columns", inplace = True)
    dataFrame.sort_values(by = "Id", axis = "index", inplace = True)

    # Compute data
    for partId in range(PARTITION_COUNT):
        dataObj.min[partId]      = {}
        dataObj.max[partId]      = {}
        dataObj.mean[partId]     = {}
        dataObj.median[partId]   = {}
        dataObj.stdev[partId]    = {}
        dataObj.outliers[partId] = {}

    for colName in dataFrame:
        if(colName not in METRICS): continue

        minTable = dataFrame.groupby("Id")[colName].min()
        medianTable = dataFrame.groupby("Id")[colName].median()
        maxTable = dataFrame.groupby("Id")[colName].max()
        meanTable = dataFrame.groupby("Id")[colName].mean()
        stdevTable = dataFrame.groupby("Id")[colName].std()
        outliersTable = dataFrame.groupby("Id")[colName]

        q1 = dataFrame.groupby("Id")[colName].quantile(0.25)
        q3 = dataFrame.groupby("Id")[colName].quantile(0.75)

        for partId in range(PARTITION_COUNT):
            dataObj.min[partId][colName]       = minTable[partId]
            dataObj.max[partId][colName]       = maxTable[partId]
            dataObj.mean[partId][colName]      = meanTable[partId]
            dataObj.median[partId][colName]    = medianTable[partId]
            dataObj.stdev[partId][colName]     = stdevTable[partId]

            iqr = q3[partId] - q1[partId]
            curTable = outliersTable.get_group(partId)
            dataObj.outliers[partId][colName] = \
                ((outliersTable.get_group(partId) < (q1[partId] - 1.5 * iqr)) |
                (outliersTable.get_group(partId) > (q3[partId] + 1.5 * iqr))).sum()

    dataObj.dataFrame = dataFrame

    return dataObj

def processDataObjects(dataObjArray):
    """
        Cleans the dataObjects in the dataObjArray and returns the per-partition
        array that contains the dataframes of each partition indexed by the
        partition ID.

    Parameters
    ----------
        dataObjArray : DataObject[] (in/out)
            The array containing extracted DataObject from the parsed files.

    Return
    ----------
        This function returns the list of dataframes organized in dictionaries
        and indexed by the partition ID.

    Raises
    ----------
        None.
    """
    print("Detected {} mitigation flavours.".format(len(detectedMitigation)))
    print("Processing {} files...".format(len(dataObjArray)))
    partitions = [collections.OrderedDict() for i in range(PARTITION_COUNT)]

    # Extract the data
    for dataObj in dataObjArray:
        print("Processing data object {};{}".format(dataObj.mitig, dataObj.type))
        for i in range(PARTITION_COUNT):
            if(dataObj.mitig not in partitions[i]):
                partitions[i][dataObj.mitig] = {}
            if(dataObj.type not in partitions[i][dataObj.mitig]):
                partitions[i][dataObj.mitig][dataObj.type] = {}
            partitions[i][dataObj.mitig][dataObj.type][dataObj.intensity] = dataObj.dataFrame.groupby("Id").get_group(i)

    return partitions

def getDataSetFiles(rootPath):
    """
        Generates the list of the files to parse by walking the input directory
        recurcsively. This directory and its sub-directories must conform to the
        directory hierarchy rules.

    Parameters
    ----------
        rootpath : str (in)
            The path to the input directory.

    Return
    ----------
        This function returns the list of file paths detected to be parsed.

    Raises
    ----------
        IOError, RuntimeError and others can be raised during the files
        manipulations. An error message is associated with the exception to
        give information about its cause.
    """
    mitigList = []
    testList  = []

    # Get the list of mitigations
    for folder in os.listdir(rootPath):
        splited = folder.split("_")
        if(len(splited) == 2 and splited[1] == "mitig"):
            mitigList.append(folder)

    # Get the list of sets
    for folder in mitigList:
        for sets in os.listdir(os.path.join(rootPath, folder)):
            splited = sets.split("_")
            if(len(splited) == 3 or sets == "Baseline"):
                testList.append(os.path.join(rootPath, folder, sets, "PART_output.csv"))

    return sorted(testList)

def parseCommand():
    """
        Parses the command line to extract the following arguments:
            -d [inputdir] The input directory to parse the extracted files.

    Parameters
    ----------
        None.

    Return
    ----------
        The argparse object that contains the parsed command line (parse_args
        is already called in this function).

    Raises
    ----------
        None.
    """
    parser = argparse.ArgumentParser(description = "Extractor Args Parser")

    # Add command argument: dataset root folder
    parser.add_argument("-d", type = str, nargs = "?", dest = "datasetPath",
                        required = True,
                        help = "Dataset root folder path")


    return parser.parse_args()

"""
def testParseFilename():
    dataObj = DataObject()
    parseFilename("CPC_mitig/MC_ALL_05/PART_output.csv", dataObj)
    print(dataObj.type + "; " + dataObj.mitig + "; " + str(dataObj.intensity))
    parseFilename("CPC_mitig/Baseline/PART_output.csv", dataObj)
    print(dataObj.type + "; " + dataObj.mitig + "; " + str(dataObj.intensity))
    parseFilename("Data/Other/CPC_mitig/MC_ALL_05/PART_output.csv", dataObj)
    print(dataObj.type + "; " + dataObj.mitig + "; " + str(dataObj.intensity))
    parseFilename("uneDat/CPC_mitig/Baseline/PART_output.csv", dataObj)
    print(dataObj.type + "; " + dataObj.mitig + "; " + str(dataObj.intensity))
    exit(0)

def testGetDataSetFiles():
    print(getDataSetFiles("."))
    for data in getDataSetFiles("."):
        dataObj = DataObject()
        parseFilename(data, dataObj)
        print(dataObj.type + "; " + dataObj.mitig + "; " + str(dataObj.intensity))
    exit(0)
"""

if __name__ == "__main__":
    dataObjArray = []

    # Set environment
    os.environ["DISPLAY"] = ":0"

    # Get the arguments
    args = parseCommand()
    files = getDataSetFiles(args.datasetPath)

    # Generate data object for each file
    for filename in files:
        dataObjArray.append(generateDataObject(filename))

    partitions = processDataObjects(dataObjArray)

    for i in range(PARTITION_COUNT):
        genBoxplotTypesComp(partitions, i, "ExecTime")

    genTypeCompCSV(dataObjArray, "ExecTime", -1)


    print("\n---------------------------------")
    print("Processing finished without error")
    input("Press Enter to exit")