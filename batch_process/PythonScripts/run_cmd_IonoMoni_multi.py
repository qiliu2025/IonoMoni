# ===============================================================================
# PPP batch solution main program
# ===============================================================================

import argparse
import logging
import os
import shutil
import sys

import os  # 确保顶部导入 os

pyDirName, pyFileName = os.path.split(os.path.abspath(__file__))
sys.path.append(f"{pyDirName}/../")

from tool.gnss_sitelist_io import readSitelist
from tool.gnss_ini_tool import t_iniCfg
from tool.gnss_xml_tool import t_xmlCfg
from tool.gnss_timestran_tool import gnssTime2datetime, datetime2doy, ReplaceTimeWildcard
from tool.gnss_run_tool import execute_multi


# =========================================================================================
class t_run_IonoMoni(object):
    """
    Processes that run IonoMoni in batches are controlled by this class
    """

    def __init__(self, ini_path, year, beg, end):
        """
        The class that stores IonoMoni ini format configuration file information
        """
        if os.path.exists(ini_path):
            self.ini_path = ini_path
            self.ini_config = t_iniCfg(ini_path)
            self.iBeg = beg
            self.iEnd = end
            self.iYear = year
            logging.info(f"Read the path : {ini_path}")
        else:
            self.ini_path = ""
            logging.error(f"Check your path {ini_path}")
            exit(0)

    def run_cmd_IonoMoni(self, overwrite_path, overwrite_xml, numberThread):
        """
        The main function and external interface, by calling this function to achieve the entire process
        """

        # Handling ini files
        self.procIniConfig()

        # Process project folder
        self.procProjectPath(overwrite_path)

        # Processing xml files
        self.procXmlConfig(overwrite_xml)

        # Run an executable program
        self.procBinRunning(numberThread)

    def procBinRunning(self, numThread):
        """
        Switch to the corresponding project folder, call the executable program, calculate the result
        """
        # Loop through the previously determined list of project folders
        ## Daily cycle
        yearBeg, begDoy = datetime2doy(self.beg_time)
        yearEnd, endDoy = datetime2doy(self.end_time)
        if yearBeg != yearEnd:
            logging.error(f"Beg Time Year ({yearBeg}) != End Time Year ({yearEnd})")
            raise Exception
        year = yearBeg
        cmd_list = list()
        for doy in range(begDoy, endDoy + 1):
            yearDoyIdx = f"{year:04d}_{doy:03d}"

            # Get all commands for the day
            cmd_list = list()
            for pathIdx in self.process_path:
                if yearDoyIdx not in pathIdx:
                    continue

                proPath = self.process_path[pathIdx]
                logging.info(f"Process at : {proPath}")
                if not os.path.exists(proPath):
                    continue
                os.chdir(proPath)

                # Create a result path
                resultPath = os.path.join(proPath, "IonoMoni_result")

                if not os.path.exists(resultPath):
                    os.makedirs(resultPath)

                # Get the corresponding xml file
                xmlPath = self.xml_path[pathIdx]
                if not os.path.exists(xmlPath):
                    continue

                # Gets run command line parameters
                # IonoMoni_cmd = f"{self.IonoMoni} -x {xmlPath} > run_cmd_IonoMoni_{pathIdx}.cmd_log"
                IonoMoni_cmd = f"{self.IonoMoni} -x \"{xmlPath}\" > run_cmd_IonoMoni_{pathIdx}.cmd_log"

                logging.info(f"Run at {IonoMoni_cmd}")

                # If it is single-threaded
                if numThread == 1:
                    # Run command
                    logging.info(f"Running cmd : {IonoMoni_cmd}")
                    ret = os.system(IonoMoni_cmd)
                else:
                    cmd_list.append(IonoMoni_cmd)

            # Multithreaded execution
            if numThread != 1:
                if len(cmd_list) == 0:
                    continue
                execute_multi(cmd_list, numThread)

    def procXmlConfig(self, overwrite):
        """
        After reading the xml template file, replace and process it according to the actual file name and path each day,
        and then store the xml file to the project path.
        The following nodes are processed
        - beg
        - end
        - rec
        - input
        """
        # Check whether the template file exists
        if not os.path.exists(self.IonoMoni_xml) or not os.path.isfile(self.IonoMoni_xml):
            logging.error(f"Check your xml file : {self.IonoMoni_xml}")
        self.xml_config = t_xmlCfg(self.IonoMoni_xml)

        # Create folders according to the pattern
        yearBeg, begDoy = datetime2doy(self.beg_time)
        yearEnd, endDoy = datetime2doy(self.end_time)
        if yearBeg != yearEnd:
            logging.error(f"Beg Time Year ({yearBeg}) != End Time Year ({yearEnd})")
            raise Exception
        year = yearBeg
        self.xml_path = dict()

        for dictIDX in self.site_dict:
            for doy in range(begDoy, endDoy + 1):
                # No project folder created, skip it
                if f"{year:04d}_{doy:03d}_{dictIDX:03d}" not in self.process_path:
                    continue

                onePath = os.path.join(self.process_path[f"{year:04d}_{doy:03d}_{dictIDX:03d}"],
                                       f"IonoMoni_{year:04d}_{doy:03d}_{dictIDX:03d}.xml")

                # Store the path of the xml file
                self.xml_path[f"{year:04d}_{doy:03d}_{dictIDX:03d}"] = onePath

                # If it needs to be overwritten, simply remove it
                if overwrite and os.path.exists(onePath):
                    os.system(f"rm -f {onePath}")
                elif False == overwrite and os.path.exists(onePath):
                    continue

                # Change the start and end times
                beg_time = gnssTime2datetime(f"{year:04d} {(doy):03d}", "YearDoy")
                end_time = gnssTime2datetime(f"{year:04d} {(doy + self.int_time - 1):03d}", "YearDoy")
                begTimeStr = f"{beg_time.year:04d}-{beg_time.month:02d}-{beg_time.day:02d} 00:00:00"
                endTimeStr = f"{end_time.year:04d}-{end_time.month:02d}-{end_time.day:02d} 23:59:59"
                self.xml_config.setNode2ValueStr("gen", "beg", begTimeStr)
                self.xml_config.setNode2ValueStr("gen", "end", endTimeStr)
                self.xml_config.setNode2ValueStr("gen", "function", self.function_mode)#新修改

                # Modify the station name
                self.xml_config.setNode2ValueListChangeLine("gen", "rec", self.site_dict[dictIDX], 10, True)

                # The output file can be modified as follows:
                ## log Indicates the log type and output path
                self.xml_config.setNode2AtrrValueStr("outputs", "log", "name", f"ppp")
                self.xml_config.setNode2AtrrValueStr("outputs", "log", "type", f"CONSOLE")
                self.xml_config.setNode2ValueStr("outputs", "ppp", os.path.join(self.project_path,
                                                                                f"{year:04d}_{(doy):03d}/IonoMoni_result/$(rec)_{year:04d}{(doy):03d}"))
                self.xml_config.setNode2ValueStr("outputs", "flt", os.path.join(self.project_path,
                                                                                f"{year:04d}_{(doy):03d}/IonoMoni_result/$(rec)_{year:04d}{(doy):03d}.flt"))

                # The input file can be modified as follows:
                ## Observations file. Only one station is considered at this time
                rinexo_list = list()
                for one_rinexo in self.site_dict[dictIDX]:
                    rinexo = os.path.join(self.rinexo_path, self.rinexo_name)
                    rinexo = ReplaceTimeWildcard(rinexo, beg_time)
                    rinexo = rinexo.replace('<SITE>', f"{one_rinexo}")
                    rinexo = ReplaceTimeWildcard(rinexo, beg_time)
                    rinexo_list.append(rinexo)
                    logging.info(f"Set rinexo add : {rinexo}")
                self.xml_config.setNode2ValueList("inputs", "rinexo", rinexo_list)

                ## Broadcast ephemeris files. It may take several days
                if False == isinstance(self.rinexn_name, list):
                    rinexn = os.path.join(self.rinexn_path, self.rinexn_name)
                    rinexn = ReplaceTimeWildcard(rinexn, beg_time)
                    self.xml_config.setNode2ValueStr("inputs", "rinexn", rinexn)
                    logging.info(f"Set rinexn as : {rinexn}")
                else:
                    rinexn_list = list()
                    for one_rinexn in self.rinexn_name:
                        rinexn = os.path.join(self.rinexn_path, one_rinexn)
                        rinexn = ReplaceTimeWildcard(rinexn, beg_time)
                        rinexn_list.append(rinexn)
                        logging.info(f"Set rinexn add : {rinexn}")
                    self.xml_config.setNode2ValueList("inputs", "rinexn", rinexn_list)

                ## Precise ephemeris file. It could be multi-day
                if False == isinstance(self.sp3_name, list):
                    sp3 = os.path.join(self.sp3_path, self.sp3_name)
                    sp3 = ReplaceTimeWildcard(sp3, beg_time)
                    self.xml_config.setNode2ValueStr("inputs", "sp3", sp3)
                    logging.info(f"Set sp3 as : {sp3}")
                else:
                    sp3_list = list()
                    for one_sp3 in self.sp3_name:
                        sp3 = os.path.join(self.sp3_path, one_sp3)
                        sp3 = ReplaceTimeWildcard(sp3, beg_time)
                        sp3_list.append(sp3)
                        logging.info(f"Set sp3 add : {sp3}")
                    self.xml_config.setNode2ValueList("inputs", "sp3", sp3_list)

                ## Precision clock difference file, may occur for many days

                # === 添加 THREE_SP3 节点 ===新修改
                three_sp3_list = []
                for offset in [-1, 0, 1]:
                    this_time = gnssTime2datetime(f"{year:04d} {(doy + offset):03d}", "YearDoy")
                    sp3 = os.path.join(self.three_sp3_path, self.three_sp3_name)
                    sp3 = ReplaceTimeWildcard(sp3, this_time)
                    three_sp3_list.append(sp3)
                    logging.info(f"Set THREE_SP3 add : {sp3}")

                self.xml_config.setNode2ValueList("inputs", "THREE_SP3", three_sp3_list)



                if False == isinstance(self.rinexc_name, list):
                    rinexc = os.path.join(self.rinexc_path, self.rinexc_name)
                    rinexc = ReplaceTimeWildcard(rinexc, beg_time)
                    self.xml_config.setNode2ValueStr("inputs", "rinexc", rinexc)
                    logging.info(f"Set rinexc as : {rinexc}")
                else:
                    rinexc_list = list()
                    for one_rinexc in self.rinexc_name:
                        rinexc = os.path.join(self.rinexc_path, one_rinexc)
                        rinexc = ReplaceTimeWildcard(rinexc, beg_time)
                        rinexc_list.append(rinexc)
                        logging.info(f"Set rinexc add : {rinexc}")
                    self.xml_config.setNode2ValueList("inputs", "rinexc", rinexc_list)

                ## Precision deviation files may occur for several days
                if False == isinstance(self.bia_name, list):
                    bia = os.path.join(self.bia_path, self.bia_name)
                    bia = ReplaceTimeWildcard(bia, beg_time)
                    self.xml_config.setNode2ValueStr("inputs", "bias", bia)
                    logging.info(f"Set bias as : {bia}")
                else:
                    bia_list = list()
                    for one_bia in self.bia_name:
                        bia = os.path.join(self.bia_path, one_bia)
                        bia = ReplaceTimeWildcard(bia, beg_time)
                        bia_list.append(bia)
                        logging.info(f"Set bia add : {bia}")
                    self.xml_config.setNode2ValueList("inputs", "bias", bia_list)

                ## ifcb file, may appear for several days
                if False == isinstance(self.ifcb_name, list):
                    ifcb = os.path.join(self.ifcb_path, self.ifcb_name)
                    ifcb = ReplaceTimeWildcard(ifcb, beg_time)
                    self.xml_config.setNode2ValueStr("inputs", "ifcb", ifcb)
                    logging.info(f"Set ifcb as : {ifcb}")
                else:
                    ifcb_list = list()
                    for one_ifcb in self.ifcb_name:
                        ifcb = os.path.join(self.ifcb_path, one_ifcb)
                        ifcb = ReplaceTimeWildcard(ifcb, beg_time)
                        ifcb_list.append(ifcb)
                        logging.info(f"Set ifcb add : {ifcb}")
                    self.xml_config.setNode2ValueList("inputs", "ifcb", ifcb_list)

                # The UPD product file may exist for several days
                if False == isinstance(self.upd_name, list):
                    upd = os.path.join(self.upd_path, self.upd_name)
                    upd = ReplaceTimeWildcard(upd, beg_time)
                    self.xml_config.setNode2ValueStr("inputs", "upd", upd)
                    logging.info(f"Set upd as : {upd}")
                else:
                    upd_list = list()
                    for one_upd in self.upd_name:
                        upd = os.path.join(self.upd_path, one_upd)
                        upd = ReplaceTimeWildcard(upd, beg_time)
                        upd_list.append(upd)
                        logging.info(f"Set upd add : {upd}")
                    self.xml_config.setNode2ValueList("inputs", "upd", upd_list)

                ## System file. Node names may vary depending on the version
                ## Antenna file
                atx = os.path.join(self.systerm_path, self.systerm_atx)
                atx = ReplaceTimeWildcard(atx, beg_time)
                self.xml_config.setNode2ValueStr("inputs", "atx", atx)
                logging.info(f"Set atx as : {atx}")

                ## Celestial ephemeris file
                de = os.path.join(self.systerm_path, self.systerm_de)
                de = ReplaceTimeWildcard(de, beg_time)
                self.xml_config.setNode2ValueStr("inputs", "de", de)
                logging.info(f"Set de as : {de}")

                ## Tidal deformation file
                blq = os.path.join(self.systerm_path, self.systerm_blq)
                blq = ReplaceTimeWildcard(blq, beg_time)
                self.xml_config.setNode2ValueStr("inputs", "blq", blq)
                logging.info(f"Set blq as : {blq}")

                ## Earth rotation file
                eop = os.path.join(self.systerm_path, self.systerm_eop)
                eop = ReplaceTimeWildcard(eop, beg_time)
                self.xml_config.setNode2ValueStr("inputs", "eop", eop)
                logging.info(f"Set eop as : {eop}")

                # Save the modified xml
                logging.info(f"Save Xml file to : {onePath}")
                self.xml_config.saveDirXML(onePath, False)

    def procIniConfig(self):
        """
        After reading the ini file, the information in the ini file is processed and stored
        """
        # IonoMoni section

        # 获取要写入的功能 function 字段
        self.function_mode = self.ini_config.getValue("IonoMoni", "function")
        logging.info(f"Function Mode is : {self.function_mode}")

        ## Start, end time
        begTime = self.ini_config.getValue("IonoMoni", "beg_time")
        self.beg_time = gnssTime2datetime(f"{self.iYear:04d} {self.iBeg:03d}", "YearDoy")
        logging.info(f"Beg time is : {begTime}")

        endTime = self.ini_config.getValue("IonoMoni", "end_time")
        self.end_time = gnssTime2datetime(f"{self.iYear:04d} {self.iEnd:03d}", "YearDoy")
        logging.info(f"End time is : {endTime}")

        ## A single example deals with arc length
        self.int_time = int(self.ini_config.getValue("IonoMoni", "int_time"))
        logging.info(f"int_time is : {self.int_time}")

        ## Site list
        self.site_path = self.ini_config.getValue("IonoMoni", "site_path")
        print("[DEBUG] 当前工作目录是：", os.getcwd())
        print("[DEBUG] 你传入的 site_path 是：", self.site_path)
        print("[DEBUG] 文件是否存在：", os.path.isfile(self.site_path))

        self.site_list = readSitelist(self.site_path)
        self.site_dict = dict()
        number = 0

        ## number of thread
        cut_number = int(1)
        for site in self.site_list:
            dictIDX = int(number / cut_number)
            if dictIDX not in self.site_dict:
                self.site_dict[dictIDX] = list()
            self.site_dict[dictIDX].append(site)
            number = number + 1
        logging.info(f"Site Path is : {self.site_path}")

        ## Project folder path
        self.project_path = self.ini_config.getValue("IonoMoni", "project_path")
        logging.info(f"project_path is : {self.project_path}")

        self.project_path = os.path.abspath(self.project_path)
        logging.info(f"[abs] project_path resolved to : {self.project_path}")

        ## Path to the template xml file
        self.IonoMoni_xml = self.ini_config.getValue("IonoMoni", "IonoMoni_xml")
        logging.info(f"IonoMoni_xml  is : {self.IonoMoni_xml}")

        ## Path of the executable file
        self.IonoMoni = self.ini_config.getValue("IonoMoni", "IonoMoni")
        self.IonoMoni = os.path.abspath(self.IonoMoni)  # ← 转换为绝对路径
        logging.info(f"[abs] IonoMoni resolved to : {self.IonoMoni}")

        # data_pool section
        ## File naming format
        ## sp3 and bias files

        self.three_sp3_name = self.ini_config.getValue("data_pool", "three_sp3_name")#新修改
        self.three_sp3_path = self.ini_config.getValue("data_pool", "three_sp3_path")
        self.three_sp3_path = os.path.abspath(self.three_sp3_path)
        logging.info(f"[abs] three_sp3_path resolved to : {self.three_sp3_path}")

        logging.info(f"three_sp3_name is : {self.three_sp3_name}")
        logging.info(f"three_sp3_path is : {self.three_sp3_path}")

        self.sp3_name = self.ini_config.getValue("data_pool", "sp3_name")
        self.bia_name = self.ini_config.getValue("data_pool", "bia_name")
        self.ifcb_name = self.ini_config.getValue("data_pool", "ifcb_name")

        logging.info(f"sp3_name is : {self.sp3_name}")
        logging.info(f"bia_name is : {self.bia_name}")
        logging.info(f"ifcb_name is : {self.ifcb_name}")

        # rinexo, rinexc, and rinexn files
        self.rinexo_name = self.ini_config.getValue("data_pool", "rinexo_name")
        self.rinexn_name = self.ini_config.getValue("data_pool", "rinexn_name")
        self.rinexc_name = self.ini_config.getValue("data_pool", "rinexc_name")

        logging.info(f"rinexo_name is : {self.rinexo_name}")
        logging.info(f"rinexn_name is : {self.rinexn_name}")
        logging.info(f"rinexc_name is : {self.rinexc_name}")

        ## File storage path
        self.rinexo_path = self.ini_config.getValue("data_pool", "rinexo_path")
        self.rinexo_path = os.path.abspath(self.rinexo_path)
        logging.info(f"[abs] rinexo_path resolved to : {self.rinexo_path}")

        self.rinexn_path = self.ini_config.getValue("data_pool", "rinexn_path")
        self.rinexn_path = os.path.abspath(self.rinexn_path)
        logging.info(f"[abs] rinexn_path resolved to : {self.rinexn_path}")

        self.rinexc_path = self.ini_config.getValue("data_pool", "rinexc_path")
        self.rinexc_path = os.path.abspath(self.rinexc_path)
        logging.info(f"[abs] rinexc_path resolved to : {self.rinexc_path}")

        self.sp3_path = self.ini_config.getValue("data_pool", "sp3_path")
        self.sp3_path = os.path.abspath(self.sp3_path)
        logging.info(f"[abs] sp3_path resolved to : {self.sp3_path}")

        self.bia_path = self.ini_config.getValue("data_pool", "bia_path")
        self.bia_path = os.path.abspath(self.bia_path)
        logging.info(f"[abs] bia_path resolved to : {self.bia_path}")

        self.ifcb_path = self.ini_config.getValue("data_pool", "ifcb_path")
        self.ifcb_path = os.path.abspath(self.ifcb_path)
        logging.info(f"[abs] ifcb_path resolved to : {self.ifcb_path}")

        logging.info(f"rinexo_path is : {self.rinexo_path}")
        logging.info(f"rinexn_path is : {self.rinexn_path}")
        logging.info(f"rinexc_path is : {self.rinexc_path}")
        logging.info(f"sp3_path is : {self.sp3_path}")
        logging.info(f"bia_path is : {self.bia_path}")
        logging.info(f"ifcb_path is : {self.ifcb_path}")

        ## UPD file path
        self.upd_name = self.ini_config.getValue("data_pool", "upd_name")
        self.upd_path = self.ini_config.getValue("data_pool", "upd_path")
        self.upd_path = os.path.abspath(self.upd_path)
        logging.info(f"[abs] upd_path resolved to : {self.upd_path}")

        logging.info(f"upd_name is : {self.upd_name}")
        logging.info(f"upd_path is : {self.upd_path}")

        # System file path and name
        self.systerm_path = self.ini_config.getValue("data_pool", "systerm_path")
        self.systerm_path = os.path.abspath(self.systerm_path)
        logging.info(f"[abs] systerm_path resolved to : {self.systerm_path}")

        self.systerm_atx = self.ini_config.getValue("data_pool", "systerm_atx")
        self.systerm_de = self.ini_config.getValue("data_pool", "systerm_de")
        self.systerm_blq = self.ini_config.getValue("data_pool", "systerm_blq")
        self.systerm_eop = self.ini_config.getValue("data_pool", "systerm_eop")

    def procProjectPath(self, overwrite):
        """
        Create a folder based on the entered path and control parameters
        overwrite:If true, empty the original folder directly
        mode:
            SITE, station priority,project/<SITE>/<YYYY_DOY>
            TIME, time is priority,project/<YYYY_DOY>/<SITE>
        """
        # Is it a folder?
        if os.path.isfile(self.project_path):
            raise Exception
            return -1

        # Does the folder exist?
        if not os.path.exists(self.project_path):
            os.makedirs(self.project_path)

        # Create folders according to the pattern
        year, begDoy = datetime2doy(self.beg_time)
        year, endDoy = datetime2doy(self.end_time)
        self.process_path = dict()
        for doy in range(begDoy, endDoy + 1):
            for dictIDX in self.site_dict:
                onePath = os.path.join(self.project_path, f"{year:04d}_{doy:03d}")

                if os.path.exists(onePath):
                    if overwrite:
                        # Need to empty it?
                        shutil.rmtree(onePath)
                        self.process_path[f"{year:04d}_{doy:03d}_{dictIDX:03d}"] = onePath
                    else:
                        continue
                else:
                    os.makedirs(onePath)
                    self.process_path[f"{year:04d}_{doy:03d}_{dictIDX:03d}"] = onePath

                # Result file
                resultPath = os.path.join(onePath, "IonoMoni_result")
                if not os.path.exists(resultPath):
                    os.makedirs(resultPath)


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)

    parser = argparse.ArgumentParser(description="Starting application")
    parser.add_argument('-ini', type=str, help="ini")  # End time
    parser.add_argument('-year', type=int, help="year")  # End time
    parser.add_argument('-beg', type=int, help="beg")  # End time
    parser.add_argument('-end', type=int, help="end")  # End time
    args = parser.parse_args()
    ini = args.ini
    year = args.year
    begDoy = args.beg
    endDoy = args.end

    if ini and len(ini) != 0:
        iniPath = ini
    else:
        year = 2024
        begDoy = 40
        endDoy = 42
        iniPath = r"D:\run_ppp\ini\run_IonoMoni_test.ini"

    run_cmd_IonoMoni = t_run_IonoMoni(iniPath, year, begDoy, endDoy)

    logging.info("Begin Running!")
    run_cmd_IonoMoni.run_cmd_IonoMoni(True, True, 5)
