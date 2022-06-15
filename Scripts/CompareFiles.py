import os
import csv
import json
import argparse
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.pyplot import cm

PARTITION_COUNT = 10
METRICS = ["ExecTime", "l2Miss"]
#ENVTEST = ["Baseline", "ALL", "IPI", "SC", "EXT", "INT"]
ENVTEST = ["Baseline"]

detectedMitigation = []

class DataObject:
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

class NpEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, np.integer):
            return int(obj)
        if isinstance(obj, np.floating):
            return float(obj)
        if isinstance(obj, np.ndarray):
            return obj.tolist()
        return super(NpEncoder, self).default(obj)

def printDataObject(obj):
    for i in range(PARTITION_COUNT):
        print("==== Partition {}".format(i))
        for metric in METRICS:
            print("\t {}:".format(metric))
            print("\t\tMin: {}\n\t\tMax: {}\n\t\tAvg: {}\n\t\tMed: {}\n\t\tStd: {}".format(obj.min[i][metric], obj.max[i][metric], obj.mean[i][metric], obj.median[i][metric], obj.stdev[i][metric]))

    for metric in METRICS:
        obj.dataFrame.hist(column = metric, by = "Id", bins = 50)
        plt.show()

def printParitionsData(partitions):
    for partId in range(len(partitions)):
        print("=== Partition {}".format(partId))
        for mitig, dataInf in partitions[partId].items():
            print("\tMitig: " + mitig)
            for typeInf, dataTypeInf in dataInf.items():
                print("\t\tType: " + typeInf)
                for intInf, dataFrame in dataTypeInf.items():
                    print("\t\t\tInt: " + str(intInf))
                    #print(dataFrame)

def genHistTypes(partitionsData, partId, metric):
    for mitig, typeData in partitionsData[partId].items():

        fig = plt.figure(figsize =(10, 7))
        ax = fig.add_subplot(111)
        for typeInf, dataTypeInf in typeData.items():
            for intInf, dataFrame in dataTypeInf.items():
                ax.hist(dataFrame[metric], 200, density=True, histtype='bar', label=typeInf)

        plt.legend(prop={'size': 10})
        plt.title("Partition " + str(partId) + " | " + metric + " | Mitigation: " + mitig)
        plt.show(block=False)

def genBoxplotTypes(partitionsData, partId, metric):
    for mitig, typeData in partitionsData[partId].items():
        filteredData = []
        labels = []

        for typeInf, dataTypeInf in typeData.items():
                for intInf, dataFrame in dataTypeInf.items():
                    #print(dataFrame
                    #plt.hist(dataFrame[metric], n_bins, density=True, histtype='bar', label=typeInf)
                    if(typeInf == "Baseline"):
                        filteredData.insert(0, dataFrame[metric])
                        labels.insert(0, typeInf)
                    else:
                        filteredData.append(dataFrame[metric])
                        labels.append(typeInf)

        fig = plt.figure(figsize =(10, 7))
        ax = fig.add_subplot(111)
        bp = ax.boxplot(filteredData, notch ='True', vert = 1)
        ax.set_xticklabels(labels)

        plt.legend(prop={'size': 10})
        plt.title("Partition " + str(partId) + " | " + metric + " | Mitigation: " + mitig)
        plt.show(block=False)

def set_box_color(bp, color):
    plt.setp(bp['boxes'], color=color)
    plt.setp(bp['whiskers'], color=color)
    plt.setp(bp['caps'], color=color)
    plt.setp(bp['medians'], color=color)

def genBoxplotTypesComp(partitionsData, partId, metric):
    mitigDict = {}
    mitigDictInv = {}
    color = iter(cm.rainbow(np.linspace(0, 1, len(partitionsData))))

    filteredData = [[] for i in range(len(partitionsData[partId]))]
    i = 0

    for envName in ENVTEST:
        for mitig, typeData in partitionsData[partId].items():
            if(mitig not in mitigDict):
                mitigDict[mitig] = i
                mitigDictInv[i] = mitig
                i += 1
            for intInf, dataFrame in typeData[envName].items():
                filteredData[mitigDict[mitig]].append(dataFrame[metric])

    fig = plt.figure(figsize =(3, 6))
    ax = fig.add_subplot(111)
    i = 0
    offset = 0.2 * (len(ENVTEST) / -3) - 0.5

    colIdx = 0
    for data in filteredData:
        bp = ax.boxplot(data, notch ='True', vert = 1, widths=0.4, positions=np.array(range(len(data)))*5.0+offset, sym='.')
        offset += 0.5
        c = next(color)
        colIdx += 1
        plt.plot([], c=c, label=mitigDictInv[i])
        set_box_color(bp, c)
        print("Adding plot: " + mitigDictInv[i])
        i += 1

    plt.legend()
    plt.xticks(range(0, len(ENVTEST) * 5, 5), ENVTEST)
    plt.xlim(-2, 2)
    plt.tight_layout()
    plt.title("Partition " + str(partId) + " | " + metric)
    plt.show(block=False)
    plt.savefig("Partition_MCSC" + str(partId) + ".png")

def parseFilename(filename, dataObj):
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
            dataObj.outliers[partId][colName] = ((outliersTable.get_group(partId) < (q1[partId] - 1.5 * iqr)) | (outliersTable.get_group(partId) > (q3[partId] + 1.5 * iqr))).sum()

    dataObj.dataFrame = dataFrame

    return dataObj


def processDataObjects(dataObjArray):
    print("Detected {} mitigation flavours.".format(len(detectedMitigation)))
    print("Processing {} files...".format(len(dataObjArray)))
    partitions = [{} for i in range(PARTITION_COUNT)]

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

    return testList

def parseCommand():
    """
    Parses the command line to extract the following arguments:


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

    # Add command argument: dataset root folder
    parser.add_argument("-d", type = str, nargs = "?", dest = "datasetPath",
                        required = True,
                        help = "Dataset root folder path")


    return parser.parse_args()


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

def genTypeCompCSV(dataArray, metric, partId = -1):
    print("partId, metric, type, mitig, min, max, mean, median, stdev, outliers (val), outliers (%)")
    if partId == -1:
        for partId in range(PARTITION_COUNT):
            for dataObj in dataArray:
                if dataObj.type in ENVTEST:
                    print("{},{},{},{},{},{},{},{},{},{},{}".format(partId, metric, dataObj.type, dataObj.mitig, dataObj.min[partId][metric], dataObj.max[partId][metric], dataObj.mean[partId][metric], dataObj.median[partId][metric], dataObj.stdev[partId][metric], dataObj.outliers[partId][metric], dataObj.outliers[partId][metric] / 10000 * 100))
    else:
        for dataObj in dataArray:
                if dataObj.type in ENVTEST:
                    print("{},{},{},{},{},{},{},{},{},{},{}".format(partId, metric, dataObj.type, dataObj.mitig, dataObj.min[partId][metric], dataObj.max[partId][metric], dataObj.mean[partId][metric], dataObj.median[partId][metric], dataObj.stdev[partId][metric], dataObj.outliers[partId][metric], dataObj.outliers[partId][metric] / 10000 * 100))

if __name__ == "__main__":
    dataObjArray = []
    # Set environment
    os.environ["DISPLAY"] = ":0"

    # tests
    #testParseFilename()
    #testGetDataSetFiles()

    # Get the arguments
    args = parseCommand()
    files = getDataSetFiles(args.datasetPath)

    # Generate data object for each file
    for filename in files:
        dataObjArray.append(generateDataObject(filename))

    partitions = processDataObjects(dataObjArray)

    #genHistTypes(partitions, 0, "ExecTime")
    #genBoxplotTypesComp(partitions, 2, "ExecTime")
    #for i in range(PARTITION_COUNT):
    #    genBoxplotTypesComp(partitions, i, "ExecTime")
    #    printDataObject(dataObjArray[i])

    genTypeCompCSV(dataObjArray, "ExecTime", -1)


    print("\n---------------------------------")
    print("Extraction finished without error")
    input("Press Enter to exit")