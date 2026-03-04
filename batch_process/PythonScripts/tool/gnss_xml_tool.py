# =================================================================
# Parsing and generation of XML configuration files
# =================================================================
import os
from pickle import NONE
import xml.etree.ElementTree as ET
import lxml.html as lhtml

# =======================================================
# Macro definition
MY_GLOBAL_TEST = False
NODE_logger = "logger"
NODE_monitor = "monitor"
NODE_email = "email"

# data node
NODE_systerm = "IonoMoni_systerm"
NODE_data_pool = "data_pool"

# download node
NODE_rinexo_list = "rinexo_list"
NODE_rinexo_hour = "rinexo_hour"
NODE_rinexo_day = "rinexo_day"

NODE_rinexn_list = "rinexn_list"
NODE_rinexn_hour = "rinexn_hour"
NODE_rinexn_day = "rinexn_day"


# =======================================================


class t_xmlCfg(object):  # meter
    """
    The class that stores IonoMoni configuration file information in XML format
    """

    def __init__(self, pathXML):
        if os.path.exists(pathXML):
            self.tree = ET.parse(pathXML)  # Parse the xml into a tree
            self.root = self.tree.getroot()  # Get root node
            self.path = pathXML
        else:
            self.tree = NONE
            self.root = NONE
            self.path = NONE

    def saveDirXML(self, xmlPath, overWrite):
        """
        Save XML as
        """
        if not os.path.exists(os.path.dirname(xmlPath)):
            os.makedirs(os.path.dirname(xmlPath))
        xmlFilePath = os.path.join(xmlPath)

        if os.path.exists(xmlFilePath) and not overWrite:
            return
        else:
            self.tree.write(xmlFilePath, encoding='utf-8')
            self._prettyXML(xmlFilePath)

    def saveXML(self, xmlPath, xmlName, overWrite):
        """
        Save XML as
        """
        if not os.path.exists(xmlPath):
            os.makedirs(xmlPath)
        xmlFilePath = os.path.join(xmlPath, xmlName)

        if os.path.exists(xmlFilePath) and not overWrite:
            return
        else:
            self.tree.write(xmlFilePath, encoding='utf-8')
            self._prettyXML(xmlFilePath)

    def _prettyXML(self, xmlPath):
        """
        Beautify XML files
        """
        if not os.path.exists(xmlPath):
            return
        else:
            xml_parse = lhtml.etree.XMLParser(remove_blank_text=True)
            tree = lhtml.etree.parse(open(xmlPath, 'r', encoding='utf8'), xml_parse)
            tree.write(xmlPath, pretty_print=True)

    """
    Gets all nodes with the same name
    """

    def getNodes(self, nodeName):
        nodeNum = 0
        nodeList = list()
        for child in self.root.iter(nodeName):
            nodeNum = nodeNum + 1
            nodeList.append(child)
            if MY_GLOBAL_TEST:
                rank = child.tag
                name = child.attrib
                value = child.text
                print(name, rank)
        return (nodeNum, nodeList)

    def getNode(self, nodeName):
        """
        Get node
        """
        nodeNum = 0
        nodeList = list()
        for child in self.root.iter(nodeName):
            nodeNum = nodeNum + 1
            nodeList.append(child)
        if nodeNum == 0:
            return (0, NONE)
        else:
            if MY_GLOBAL_TEST:
                rank = nodeList[0].tag
                name = nodeList[0].attrib
                print(name, rank)
            return (1, nodeList[0])

    def getNodeValue(self, nodeName):
        """
        Get node value
        """
        (number, node) = self.getNode(nodeName)
        value = str(node.text).strip()
        return value

    def setNodeValueStr(self, nodeName, value):
        """
        Assign a value to a node
        """
        (number, node) = self.getNode(nodeName)
        node.text = " " + str(value) + " "

    def setNode2AtrrValueStr(self, nodeName1, nodeName2, attrName, value):
        """
        Assign a value to the attr variable of the node, nodeName1.nodeName2.attrName = value
        """
        node = self.root.find(nodeName1).find(nodeName2)
        if attrName not in node.attrib:
            node.attrib[attrName] = value
        else:
            node.attrib[attrName] = value

    def setNode2ValueStr(self, nodeName1, nodeName2, value):
        """
        Assign a value to the node, nodeName1.nodeName2 = value
        """
        node = self.root.find(nodeName1).find(nodeName2)
        node.text = " " + str(value) + " "

    def setNode3ValueStr(self, nodeName1, nodeName2, nodeName3, value):
        """
        Assign a value to the node, nodeName1.nodeName2.nodeName3 = value
        """
        node = self.root.find(nodeName1).find(nodeName2).find(nodeName3)
        node.text = " " + str(value) + " "

    def setNodeValueList(self, nodeName, valueList):
        """
        Assign a value to the node, in list form
        """
        (number, node) = self.getNode(nodeName)
        textStr = str("\n")
        for value in valueList:
            textStr = textStr + " " + str(value).strip() + " " + "\n"
        node.text = textStr

    def setNode2ValueList(self, nodeName1, nodeName2, valueList):
        """
        Assign a value to the node, nodeName1.nodeName2 = value
        """
        node = self.root.find(nodeName1).find(nodeName2)
        textStr = str("\n")
        for value in valueList:
            textStr = textStr + " " + str(value).strip() + " " + "\n"
        node.text = textStr

    def setNode2ValueListChangeLine(self, nodeName1, nodeName2, valueList, changeLineNum, isUpper):
        """
        Assign a value to the node, nodeName1.nodeName2 = value
        """
        node = self.root.find(nodeName1).find(nodeName2)
        textStr = str("\n")
        number = 0
        for value in valueList:
            if True == isUpper:
                textStr = textStr + " " + str(value).strip().upper() + " "
            else:
                textStr = textStr + " " + str(value).strip().lower() + " "
            number = number + 1
            if number % changeLineNum == 0:
                textStr = textStr + "\n"
        node.text = textStr

    def getNodeValueList(self, nodeName):
        """
        Gets the node values and splits them into lists
        """
        (number, node) = self.getNode(nodeName)
        value = str(node.text)
        data = value.split()
        return data