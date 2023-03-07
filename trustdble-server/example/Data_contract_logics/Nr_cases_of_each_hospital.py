import sys
import mysql.connector
import re
def main():
  connection = mysql.connector.connect(host="localhost",user="root", password="",database="Hospital_Digital_System")
  sql_select_Query = "SELECT hospitals.name AS hospital_name,COUNT(*) AS cases_opened FROM cases INNER JOIN hospitals on cases.hospital_id=hospitals.id GROUP BY hospitals.name "
  cursor = connection.cursor()
  cursor.execute(sql_select_Query)
  records = cursor.fetchall()
  connection.close()
  cursor.close()
  return records