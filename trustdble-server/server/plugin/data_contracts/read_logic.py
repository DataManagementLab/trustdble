#!/usr/bin/env python3
"""The Python implementation of the loading Data Contract's Logic into DataContractLogic file."""

import mysql.connector
import pathlib


def ReadLogic(name): 
  try:
    connection = mysql.connector.connect(host='localhost',
                                           user='root',
                                           password='')
    # Fetch the logic from data_contract table
    sql_select_Query = "SELECT logic FROM trustdble.data_contracts WHERE name=\"{}\"".format(str(name))
    cursor = connection.cursor()
    cursor.execute(sql_select_Query)
    # get all records
    logic = cursor.fetchall()

    # create a python file from the logic
    path_str="DataContractLogic.py"
    f=open(path_str,"w")
    logic_str = ''.join(''.join(tup) for tup in logic)
    # write logic into the file
    f.write(logic_str)
    
  except mysql.connector.Error as e:
    print("Error reading data from MySQL table", e)
  finally:
    if connection.is_connected():
      connection.close()
      cursor.close()
      print("MySQL connection is closed")