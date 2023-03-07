import sys
import mysql.connector
import re
def main(treatment):
  connection = mysql.connector.connect(host="localhost",user="root", password="", database="Hospital_Digital_System")
  sql_select_Query= "SELECT (COUNT(*) / (SELECT COUNT(*) FROM cases)) * 100 AS percentage_of_cases FROM cases WHERE treatment like \"%" + treatment + "%\""
  cursor = connection.cursor()
  cursor.execute(sql_select_Query)
  records = cursor.fetchall()
  return records
