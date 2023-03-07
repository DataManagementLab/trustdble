import sys
import mysql.connector
import re
def main():
  connection = mysql.connector.connect(host="localhost",user="root", password="",database="Hospital_Digital_System")
  sql_select_Query = "SELECT DATE(admission_date) AS date, COUNT(*) AS patients_with_covid FROM cases WHERE diagnosis = \"COVID19\" GROUP BY DATE(admission_date)"
  cursor = connection.cursor()
  cursor.execute(sql_select_Query)
  records = cursor.fetchall()
  connection.close()
  cursor.close()
  return records